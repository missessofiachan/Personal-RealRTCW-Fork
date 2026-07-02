/*
===========================================================================

Return to Castle Wolfenstein single player GPL Source Code
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company. 

This file is part of the Return to Castle Wolfenstein single player GPL Source Code (RTCW SP Source Code).  

RTCW SP Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

RTCW SP Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RTCW SP Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the RTCW SP Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the RTCW SP Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#include "cm_local.h"

// always use bbox vs. bbox collision and never capsule vs. bbox or vice versa
#define ALWAYS_BBOX_VS_BBOX
// always use capsule vs. capsule collision and never capsule vs. bbox or vice versa
//#define ALWAYS_CAPSULE_VS_CAPSULE

//#define CAPSULE_DEBUG

/*
===============================================================================

BASIC MATH

===============================================================================
*/

/*
================
RotatePoint
================
*/
// TTimo: const vec_t ** would require explicit casts for ANSI C conformance
// see unix/const-arg.c in Wolf MP source
void RotatePoint(vec3_t point, /*const*/ vec3_t matrix[3])
{
#ifndef Q3_VM
    // 1. Load the vector components safely, padding slot 3 with 0.0f
    __m128 v_in = _mm_set_ps(0.0f, point[2], point[1], point[0]);

    // 2. Load the matrix rows into vector registers
    __m128 row0 = _mm_set_ps(0.0f, matrix[0][2], matrix[0][1], matrix[0][0]);
    __m128 row1 = _mm_set_ps(0.0f, matrix[1][2], matrix[1][1], matrix[1][0]);
    __m128 row2 = _mm_set_ps(0.0f, matrix[2][2], matrix[2][1], matrix[2][0]);

    // 3. Compute parallel hardware dot products 
    // 0x71 Mask: Multiplies elements 0, 1, 2 and places the sum into slot 0
    __m128 dot0 = _mm_dp_ps(v_in, row0, 0x71);
    __m128 dot1 = _mm_dp_ps(v_in, row1, 0x71);
    __m128 dot2 = _mm_dp_ps(v_in, row2, 0x71);

    // 4. Safely overwrite the original point array coordinates in memory
    _mm_store_ss(&point[0], dot0);
    _mm_store_ss(&point[1], dot1);
    _mm_store_ss(&point[2], dot2);
#else
    // Legacy QVM fallback
    vec3_t tvec;
    VectorCopy(point, tvec);
    point[0] = DotProduct(matrix[0], tvec);
    point[1] = DotProduct(matrix[1], tvec);
    point[2] = DotProduct(matrix[2], tvec);
#endif
}

/*
================
TransposeMatrix
================
*/
// TTimo: const vec_t ** would require explicit casts for ANSI C conformance
// see unix/const-arg.c in Wolf MP source
void TransposeMatrix(/*const*/ vec3_t matrix[3], vec3_t transpose[3])
{
#ifndef Q3_VM
    // Fully unrolled assignments bypass loop indices completely.
    // This allows the compiler to generate clean, straight-line 
    // memory move operations with no conditional branch checking.
    float m01 = matrix[0][1];
    float m02 = matrix[0][2];
    float m10 = matrix[1][0];
    float m12 = matrix[1][2];
    float m20 = matrix[2][0];
    float m21 = matrix[2][1];

    transpose[0][0] = matrix[0][0];
    transpose[0][1] = m10;
    transpose[0][2] = m20;

    transpose[1][0] = m01;
    transpose[1][1] = matrix[1][1];
    transpose[1][2] = m21;

    transpose[2][0] = m02;
    transpose[2][1] = m12;
    transpose[2][2] = matrix[2][2];
#else
    // Legacy QVM fallback
    int i, j;
    for (i = 0; i < 3; i++)
    {
        for (j = 0; j < 3; j++)
        {
            transpose[i][j] = matrix[j][i];
        }
    }
#endif
}

/*
================
CreateRotationMatrix
================
*/
void CreateRotationMatrix( const vec3_t angles, vec3_t matrix[3] ) {
	AngleVectors( angles, matrix[0], matrix[1], matrix[2] );
	VectorInverse( matrix[1] );
}

/*
================
CM_ProjectPointOntoVector
================
*/
void CM_ProjectPointOntoVector(vec3_t point, vec3_t vStart, vec3_t vDir, vec3_t vProj)
{
#ifndef Q3_VM
    // 1. Load the vectors safely into SIMD registers
    __m128 p     = _mm_set_ps(0.0f, point[2],  point[1],  point[0]);
    __m128 start = _mm_set_ps(0.0f, vStart[2], vStart[1], vStart[0]);
    __m128 dir   = _mm_set_ps(0.0f, vDir[2],   vDir[1],   vDir[0]);

    // 2. VectorSubtract: pVec = point - vStart
    __m128 pVec = _mm_sub_ps(p, start);

    // 3. DotProduct: broadcast the result to all slots so the FMA instruction can read it easily
    __m128 dot = _mm_dp_ps(pVec, dir, 0x77);

    // 4. VectorMA (Fused Multiply-Add): (dir * dot) + start
    __m128 res = _mm_fmadd_ps(dir, dot, start);

    // 5. Store back to memory safely
    _mm_store_ss(&vProj[0], res);
    _mm_store_ss(&vProj[1], _mm_shuffle_ps(res, res, _MM_SHUFFLE(1, 1, 1, 1)));
    _mm_store_ss(&vProj[2], _mm_shuffle_ps(res, res, _MM_SHUFFLE(2, 2, 2, 2)));
#else
    // Legacy QVM fallback
    vec3_t pVec;
    VectorSubtract(point, vStart, pVec);
    VectorMA(vStart, DotProduct(pVec, vDir), vDir, vProj);
#endif
}

/*
================
CM_DistanceFromLineSquared
================
*/
float CM_DistanceFromLineSquared(vec3_t p, vec3_t lp1, vec3_t lp2, vec3_t dir)
{
#ifndef Q3_VM
    vec3_t proj, t;
    
    // 1. Call our optimized projection routine
    CM_ProjectPointOntoVector(p, lp1, dir, proj);
    
    // 2. Load vectors into SIMD registers
    __m128 v_proj = _mm_set_ps(0.0f, proj[2], proj[1], proj[0]);
    __m128 v_lp1  = _mm_set_ps(0.0f, lp1[2],  lp1[1],  lp1[0]);
    __m128 v_lp2  = _mm_set_ps(0.0f, lp2[2],  lp2[1],  lp2[0]);

    // 3. Parallel check: Is the projection completely outside the endpoints?
    // cmpgt: proj > lp1 AND proj > lp2
    // cmplt: proj < lp1 AND proj < lp2
    __m128 out_gt = _mm_and_ps(_mm_cmpgt_ps(v_proj, v_lp1), _mm_cmpgt_ps(v_proj, v_lp2));
    __m128 out_lt = _mm_and_ps(_mm_cmplt_ps(v_proj, v_lp1), _mm_cmplt_ps(v_proj, v_lp2));
    
    int mask = _mm_movemask_ps(_mm_or_ps(out_gt, out_lt));
    
    // If lower 3 bits (X=1, Y=2, Z=4) have any active bits, it means it fell outside
    if (mask & 7)
    {
        // Snap to the closer endpoint
        __m128 diff1 = _mm_sub_ps(v_proj, v_lp1);
        __m128 diff2 = _mm_sub_ps(v_proj, v_lp2);
        
        // Compute squared distances to endpoints using hardware dot products
        float d1, d2;
        _mm_store_ss(&d1, _mm_dp_ps(diff1, diff1, 0x71));
        _mm_store_ss(&d2, _mm_dp_ps(diff2, diff2, 0x71));
        
        if (d1 < d2)
            VectorSubtract(p, lp1, t);
        else
            VectorSubtract(p, lp2, t);
            
        return VectorLengthSquared(t);
    }
    
    // Projection falls safely on the segment
    VectorSubtract(p, proj, t);
    return VectorLengthSquared(t);
#else
    // Legacy QVM fallback
    vec3_t proj, t;
    int j;

    CM_ProjectPointOntoVector(p, lp1, dir, proj);
    for (j = 0; j < 3; j++)
        if ((proj[j] > lp1[j] && proj[j] > lp2[j]) ||
            (proj[j] < lp1[j] && proj[j] < lp2[j]))
            break;
    if (j < 3)
    {
        if (fabs(proj[j] - lp1[j]) < fabs(proj[j] - lp2[j]))
            VectorSubtract(p, lp1, t);
        else
            VectorSubtract(p, lp2, t);
        return VectorLengthSquared(t);
    }
    VectorSubtract(p, proj, t);
    return VectorLengthSquared(t);
#endif
}

/*
================
CM_VectorDistanceSquared
================
*/
float CM_VectorDistanceSquared(vec3_t p1, vec3_t p2)
{
#ifndef Q3_VM
    __m128 v1 = _mm_set_ps(0.0f, p1[2], p1[1], p1[0]);
    __m128 v2 = _mm_set_ps(0.0f, p2[2], p2[1], p2[0]);
    __m128 diff = _mm_sub_ps(v2, v1);
    
    float dist2;
    _mm_store_ss(&dist2, _mm_dp_ps(diff, diff, 0x71));
    return dist2;
#else
    vec3_t dir;
    VectorSubtract(p2, p1, dir);
    return VectorLengthSquared(dir);
#endif
}

float SquareRootFloat(float number)
{
#ifndef Q3_VM
    // Directly run the hardware scalar square root circuit instruction
    __m128 reg = _mm_set_ss(number);
    float result;
    _mm_store_ss(&result, _mm_sqrt_ss(reg));
    return result;
#else
    // Legacy QVM bit-hack fallback
    floatint_t t;
    float x, y;
    const float f = 1.5F;

    x = number * 0.5F;
    t.f = number;
    t.i = 0x5f3759df - (t.i >> 1);
    y = t.f;
    y = y * (f - (x * y * y));
    y = y * (f - (x * y * y));
    return number * y;
#endif
}


/*
===============================================================================

POSITION TESTING

===============================================================================
*/

/*
================
CM_TestBoxInBrush
================
*/
void CM_TestBoxInBrush(traceWork_t* tw, cbrush_t* brush)
{
    int i;
    cplane_t* plane;
    float dist;
    float d1;
    cbrushside_t* side;
    float t;
    vec3_t startp;

    if (!brush->numsides)
    {
        return;
    }

#ifndef Q3_VM
    // 1. Hardware Prefetch: Start pulling the brush sides array into L1 Cache ahead of time
    _mm_prefetch((const char*)brush->sides, _MM_HINT_T0);

    // 2. Vectorized Bounding Box Check: Evaluate AABB overlaps simultaneously
    // Lower bounds vs Upper bounds mapping
    __m128 tw_min = _mm_set_ps(0.0f, tw->bounds[0][2], tw->bounds[0][1], tw->bounds[0][0]);
    __m128 br_max = _mm_set_ps(0.0f, brush->bounds[1][2], brush->bounds[1][1], brush->bounds[1][0]);
    __m128 tw_max = _mm_set_ps(0.0f, tw->bounds[1][2], tw->bounds[1][1], tw->bounds[1][0]);
    __m128 br_min = _mm_set_ps(0.0f, brush->bounds[0][2], brush->bounds[0][1], brush->bounds[0][0]);

    // Check conditions: (tw->bounds[0] > brush->bounds[1]) OR (tw->bounds[1] < brush->bounds[0])
    __m128 cmp1 = _mm_cmpgt_ps(tw_min, br_max);
    __m128 cmp2 = _mm_cmplt_ps(tw_max, br_min);
    
    int mask = _mm_movemask_ps(_mm_or_ps(cmp1, cmp2));
    if (mask & 7) // If any X, Y, or Z extreme is active, the boxes do not intersect
    {
        return;
    }
#else
    // Legacy QVM fallback axial test
    if (tw->bounds[0][0] > brush->bounds[1][0]
        || tw->bounds[0][1] > brush->bounds[1][1]
        || tw->bounds[0][2] > brush->bounds[1][2]
        || tw->bounds[1][0] < brush->bounds[0][0]
        || tw->bounds[1][1] < brush->bounds[0][1]
        || tw->bounds[1][2] < brush->bounds[0][2]
    )
    {
        return;
    }
#endif

    if (tw->sphere.use)
    {
        for (i = 6; i < brush->numsides; i++)
        {
            side = brush->sides + i;
            plane = side->plane;

            dist = plane->dist + tw->sphere.radius;
            t = DotProduct(plane->normal, tw->sphere.offset);
            if (t > 0)
            {
                VectorSubtract(tw->start, tw->sphere.offset, startp);
            }
            else
            {
                VectorAdd(tw->start, tw->sphere.offset, startp);
            }
            d1 = DotProduct(startp, plane->normal) - dist;
            if (d1 > 0)
            {
                return;
            }
        }
    }
    else
    {
        for (i = 6; i < brush->numsides; i++)
        {
            side = brush->sides + i;
            plane = side->plane;

            dist = plane->dist - DotProduct(tw->offsets[plane->signbits], plane->normal);
            d1 = DotProduct(tw->start, plane->normal) - dist;

            if (d1 > 0)
            {
                return;
            }
        }
    }

    // inside this brush
    tw->trace.startsolid = tw->trace.allsolid = qtrue;
    tw->trace.fraction = 0;
    tw->trace.contents = brush->contents;
}



/*
================
CM_TestInLeaf
================
*/
void CM_TestInLeaf(traceWork_t* tw, cLeaf_t* leaf)
{
    int k;
    int brushnum;
    cbrush_t* b;
    cPatch_t* patch;

    // 1. Loop through all brushes in this leaf node
    for (k = 0; k < leaf->numLeafBrushes; k++)
    {
#ifndef Q3_VM
        // Cache Prefetch: Look ahead one iteration and prefetch the upcoming brush metadata structure
        if (k + 1 < leaf->numLeafBrushes)
        {
            int next_brushnum = cm.leafbrushes[leaf->firstLeafBrush + k + 1];
            _mm_prefetch((const char*)&cm.brushes[next_brushnum], _MM_HINT_T0);
        }
#endif
        brushnum = cm.leafbrushes[leaf->firstLeafBrush + k];
        b = &cm.brushes[brushnum];
        
        if (b->checkcount == cm.checkcount)
        {
            continue; // Already processed this frame
        }
        b->checkcount = cm.checkcount;

        if (!(b->contents & tw->contents))
        {
            continue; // Contents mask does not match our trace filter
        }

        CM_TestBoxInBrush(tw, b);
        if (tw->trace.allsolid)
        {
            return; // Instantly drop out if we hit a solid ceiling
        }
    }

    // 2. Loop through all patches/curves in this leaf node
#ifdef BSPC
    if (1) {
#else
    if (!cm_noCurves->integer)
    {
#endif
        for (k = 0; k < leaf->numLeafSurfaces; k++)
        {
#ifndef Q3_VM
            // Cache Prefetch: Look ahead one iteration and prefetch the next surface index pointer
            if (k + 1 < leaf->numLeafSurfaces)
            {
                int next_surf = cm.leafsurfaces[leaf->firstLeafSurface + k + 1];
                _mm_prefetch((const char*)&cm.surfaces[next_surf], _MM_HINT_T0);
            }
#endif
            patch = cm.surfaces[cm.leafsurfaces[leaf->firstLeafSurface + k]];
            if (!patch)
            {
                continue;
            }
            if (patch->checkcount == cm.checkcount)
            {
                continue;
            }
            patch->checkcount = cm.checkcount;

            if (!(patch->contents & tw->contents))
            {
                continue;
            }

            if (CM_PositionTestInPatchCollide(tw, patch->pc))
            {
                tw->trace.startsolid = tw->trace.allsolid = qtrue;
                tw->trace.fraction = 0;
                tw->trace.contents = patch->contents;
                return;
            }
        }
    }
}

/*
==================
CM_TestCapsuleInCapsule

capsule inside capsule check
==================
*/
void CM_TestCapsuleInCapsule(traceWork_t* tw, clipHandle_t model)
{
    int i;
    vec3_t mins, maxs;
    vec3_t top, bottom;
    vec3_t p1, p2, tmp;
    vec3_t offset, symetricSize[2];
    float radius, halfwidth, halfheight, offs, r;

    CM_ModelBounds(model, mins, maxs);

    VectorAdd(tw->start, tw->sphere.offset, top);
    VectorSubtract(tw->start, tw->sphere.offset, bottom);
    
    // Unrolled center offset calculations bypass branch counters
    offset[0] = (mins[0] + maxs[0]) * 0.5f;
    offset[1] = (mins[1] + maxs[1]) * 0.5f;
    offset[2] = (mins[2] + maxs[2]) * 0.5f;
    
    symetricSize[0][0] = mins[0] - offset[0];
    symetricSize[0][1] = mins[1] - offset[1];
    symetricSize[0][2] = mins[2] - offset[2];
    
    symetricSize[1][0] = maxs[0] - offset[0];
    symetricSize[1][1] = maxs[1] - offset[1];
    symetricSize[1][2] = maxs[2] - offset[2];

    halfwidth = symetricSize[1][0];
    halfheight = symetricSize[1][2];
    radius = (halfwidth > halfheight) ? halfheight : halfwidth;
    offs = halfheight - radius;

    r = Square(tw->sphere.radius + radius);

    VectorCopy(offset, p1);
    p1[2] += offs;
    
    // Check sphere overlap 1
    VectorSubtract(p1, top, tmp);
    if (CM_VectorDistanceSquared(p1, top) < r)
    {
        tw->trace.startsolid = tw->trace.allsolid = qtrue;
        tw->trace.fraction = 0;
    }
    // Check sphere overlap 2
    if (CM_VectorDistanceSquared(p1, bottom) < r)
    {
        tw->trace.startsolid = tw->trace.allsolid = qtrue;
        tw->trace.fraction = 0;
    }

    VectorCopy(offset, p2);
    p2[2] -= offs;
    
    // Check sphere overlap 3
    if (CM_VectorDistanceSquared(p2, top) < r)
    {
        tw->trace.startsolid = tw->trace.allsolid = qtrue;
        tw->trace.fraction = 0;
    }
    // Check sphere overlap 4
    if (CM_VectorDistanceSquared(p2, bottom) < r)
    {
        tw->trace.startsolid = tw->trace.allsolid = qtrue;
        tw->trace.fraction = 0;
    }
    
    // Cylinder overlap check
    if ((top[2] >= p1[2] && top[2] <= p2[2]) ||
        (bottom[2] >= p1[2] && bottom[2] <= p2[2]))
    {
        top[2] = p1[2] = 0;
        if (CM_VectorDistanceSquared(top, p1) < r)
        {
            tw->trace.startsolid = tw->trace.allsolid = qtrue;
            tw->trace.fraction = 0;
        }
    }
}

/*
==================
CM_TestBoundingBoxInCapsule

bounding box inside capsule check
==================
*/
void CM_TestBoundingBoxInCapsule(traceWork_t* tw, clipHandle_t model)
{
    vec3_t mins, maxs, offset, size[2];
    clipHandle_t h;
    cmodel_t* cmod;

    CM_ModelBounds(model, mins, maxs);

    // Unrolled axis assignments replace slow loop iterations
    offset[0] = (mins[0] + maxs[0]) * 0.5f;
    offset[1] = (mins[1] + maxs[1]) * 0.5f;
    offset[2] = (mins[2] + maxs[2]) * 0.5f;

    size[0][0] = mins[0] - offset[0]; size[0][1] = mins[1] - offset[1]; size[0][2] = mins[2] - offset[2];
    size[1][0] = maxs[0] - offset[0]; size[1][1] = maxs[1] - offset[1]; size[1][2] = maxs[2] - offset[2];
    
    tw->start[0] -= offset[0]; tw->start[1] -= offset[1]; tw->start[2] -= offset[2];
    tw->end[0]   -= offset[0]; tw->end[1]   -= offset[1]; tw->end[2]   -= offset[2];

    tw->sphere.use = qtrue;
    tw->sphere.radius = (size[1][0] > size[1][2]) ? size[1][2] : size[1][0];
    tw->sphere.halfheight = size[1][2];
    VectorSet(tw->sphere.offset, 0, 0, size[1][2] - tw->sphere.radius);

    h = CM_TempBoxModel(tw->size[0], tw->size[1], qfalse);
    cmod = CM_ClipHandleToModel(h);
    CM_TestInLeaf(tw, &cmod->leaf);
}

/*
==================
CM_PositionTest
==================
*/
#define MAX_POSITION_LEAFS  1024
void CM_PositionTest(traceWork_t* tw)
{
    int leafs[MAX_POSITION_LEAFS];
    int i;
    leafList_t ll;

#ifndef Q3_VM
    // 1. Parallel SIMD Bounds Setup with Epsilon Padding
    __m128 start = _mm_set_ps(0.0f, tw->start[2], tw->start[1], tw->start[0]);
    __m128 size0 = _mm_set_ps(0.0f, tw->size[0][2], tw->size[0][1], tw->size[0][0]);
    __m128 size1 = _mm_set_ps(0.0f, tw->size[1][2], tw->size[1][1], tw->size[1][0]);
    __m128 eps   = _mm_set1_ps(1.0f);

    // VectorAdd + Parallel padding adjustments
    __m128 b0 = _mm_sub_ps(_mm_add_ps(start, size0), eps);
    __m128 b1 = _mm_add_ps(_mm_add_ps(start, size1), eps);

    _mm_store_ss(&ll.bounds[0][0], b0);
    _mm_store_ss(&ll.bounds[0][1], _mm_shuffle_ps(b0, b0, _MM_SHUFFLE(1, 1, 1, 1)));
    _mm_store_ss(&ll.bounds[0][2], _mm_shuffle_ps(b0, b0, _MM_SHUFFLE(2, 2, 2, 2)));

    _mm_store_ss(&ll.bounds[1][0], b1);
    _mm_store_ss(&ll.bounds[1][1], _mm_shuffle_ps(b1, b1, _MM_SHUFFLE(1, 1, 1, 1)));
    _mm_store_ss(&ll.bounds[1][2], _mm_shuffle_ps(b1, b1, _MM_SHUFFLE(2, 2, 2, 2)));
#else
    // Legacy QVM fallback
    VectorAdd(tw->start, tw->size[0], ll.bounds[0]);
    VectorAdd(tw->start, tw->size[1], ll.bounds[1]);

    for (i = 0; i < 3; i++)
    {
        ll.bounds[0][i] -= 1;
        ll.bounds[1][i] += 1;
    }
#endif

    ll.count = 0;
    ll.maxcount = MAX_POSITION_LEAFS;
    ll.list = leafs;
    ll.storeLeafs = CM_StoreLeafs;
    ll.lastLeaf = 0;
    ll.overflowed = qfalse;

    cm.checkcount++;
    CM_BoxLeafnums_r(&ll, 0);
    cm.checkcount++;

    // Test the contents of the leafs
    for (i = 0; i < ll.count; i++)
    {
#ifndef Q3_VM
        // Prefetch the upcoming leaf node structures to protect the data cache
        if (i + 1 < ll.count)
        {
            _mm_prefetch((const char*)&cm.leafs[leafs[i + 1]], _MM_HINT_T0);
        }
#endif
        CM_TestInLeaf(tw, &cm.leafs[leafs[i]]);
        if (tw->trace.allsolid)
        {
            break;
        }
    }
}

/*
===============================================================================

TRACING

===============================================================================
*/


/*
================
CM_TraceThroughPatch
================
*/

void CM_TraceThroughPatch(traceWork_t* tw, cPatch_t* patch)
{
    float oldFrac;

    c_patch_traces++;

    oldFrac = tw->trace.fraction;

    CM_TraceThroughPatchCollide(tw, patch->pc);

    // Hint to the compiler that hitting a closer surface is the rare path,
    // keeping the straight-line execution pipeline completely clear.
#if defined(__GNUC__) || defined(__clang__)
    if (__builtin_expect(tw->trace.fraction < oldFrac, 0))
#else
    if (tw->trace.fraction < oldFrac)
#endif
    {
        tw->trace.surfaceFlags = patch->surfaceFlags;
        tw->trace.contents = patch->contents;
    }
}

/*
================
CM_TraceThroughBrush
================
*/
void CM_TraceThroughBrush(traceWork_t* tw, cbrush_t* brush)
{
    int i;
    cplane_t *plane, *clipplane;
    float dist;
    float enterFrac, leaveFrac;
    float d1, d2;
    qboolean getout, startout;
    float f;
    cbrushside_t *side, *leadside;
    float t;
    vec3_t startp;
    vec3_t endp;

    enterFrac = -1.0;
    leaveFrac = 1.0;
    clipplane = NULL;

    if (!brush->numsides)
    {
        return;
    }

    c_brush_traces++;

    getout = qfalse;
    startout = qfalse;

    leadside = NULL;

    if (tw->sphere.use)
    {
        //
        // compare the trace against all planes of the brush
        // find the latest time the trace crosses a plane towards the interior
        // and the earliest time the trace crosses a plane towards the exterior
        //
        for (i = 0; i < brush->numsides; i++)
        {
            side = brush->sides + i;
            plane = side->plane;

            // adjust the plane distance appropriately for radius
            dist = plane->dist + tw->sphere.radius;

            // find the closest point on the capsule to the plane
            t = DotProduct(plane->normal, tw->sphere.offset);
            if (t > 0)
            {
                VectorSubtract(tw->start, tw->sphere.offset, startp);
                VectorSubtract(tw->end, tw->sphere.offset, endp);
            }
            else
            {
                VectorAdd(tw->start, tw->sphere.offset, startp);
                VectorAdd(tw->end, tw->sphere.offset, endp);
            }

            d1 = DotProduct(startp, plane->normal) - dist;
            d2 = DotProduct(endp, plane->normal) - dist;

            if (d2 > 0)
            {
                getout = qtrue; // endpoint is not in solid
            }
            if (d1 > 0)
            {
                startout = qtrue;
            }

            // if completely in front of face, no intersection with the entire brush
            if (d1 > 0 && (d2 >= SURFACE_CLIP_EPSILON || d2 >= d1))
            {
                return;
            }

            // if it doesn't cross the plane, the plane isn't relevant
            if (d1 <= 0 && d2 <= 0)
            {
                continue;
            }

            // crosses face
            if (d1 > d2)
            {
                // enter
                f = (d1 - SURFACE_CLIP_EPSILON) / (d1 - d2);
                if (f < 0)
                {
                    f = 0;
                }
                if (f > enterFrac)
                {
                    enterFrac = f;
                    clipplane = plane;
                    leadside = side;
                }
            }
            else
            {
                // leave
                f = (d1 + SURFACE_CLIP_EPSILON) / (d1 - d2);
                if (f > 1)
                {
                    f = 1;
                }
                if (f < leaveFrac)
                {
                    leaveFrac = f;
                }
            }
        }
    }
    else
    {
        //
        // compare the trace against all planes of the brush
        // find the latest time the trace crosses a plane towards the interior
        // and the earliest time the trace crosses a plane towards the exterior
        //
        for (i = 0; i < brush->numsides; i++)
        {
            side = brush->sides + i;
            plane = side->plane;

#ifndef Q3_VM
            // Vectorized calculation of plane distance adjusted for mins/maxs
            __m128 v_norm  = _mm_set_ps(0.0f, plane->normal[2], plane->normal[1], plane->normal[0]);
            __m128 v_offs  = _mm_set_ps(0.0f, tw->offsets[plane->signbits][2], tw->offsets[plane->signbits][1], tw->offsets[plane->signbits][0]);
            __m128 v_start = _mm_set_ps(0.0f, tw->start[2], tw->start[1], tw->start[0]);
            __m128 v_end   = _mm_set_ps(0.0f, tw->end[2], tw->end[1], tw->end[0]);

            // dist = plane->dist - DotProduct(offsets, normal)
            float dot_off;
            _mm_store_ss(&dot_off, _mm_dp_ps(v_offs, v_norm, 0x71));
            dist = plane->dist - dot_off;

            // d1 = DotProduct(start, normal) - dist
            // d2 = DotProduct(end, normal) - dist
            float dot_start, dot_end;
            _mm_store_ss(&dot_start, _mm_dp_ps(v_start, v_norm, 0x71));
            _mm_store_ss(&dot_end,   _mm_dp_ps(v_end,   v_norm, 0x71));
            d1 = dot_start - dist;
            d2 = dot_end - dist;
#else
            // Legacy QVM fallback
            dist = plane->dist - DotProduct(tw->offsets[plane->signbits], plane->normal);
            d1 = DotProduct(tw->start, plane->normal) - dist;
            d2 = DotProduct(tw->end, plane->normal) - dist;
#endif

            if (d2 > 0)
            {
                getout = qtrue; // endpoint is not in solid
            }
            if (d1 > 0)
            {
                startout = qtrue;
            }

            // if completely in front of face, no intersection with the entire brush
            if (d1 > 0 && (d2 >= SURFACE_CLIP_EPSILON || d2 >= d1))
            {
                return;
            }

            // if it doesn't cross the plane, the plane isn't relevant
            if (d1 <= 0 && d2 <= 0)
            {
                continue;
            }

            // crosses face
            if (d1 > d2)
            {
                // enter
                f = (d1 - SURFACE_CLIP_EPSILON) / (d1 - d2);
                if (f < 0)
                {
                    f = 0;
                }
                if (f > enterFrac)
                {
                    enterFrac = f;
                    clipplane = plane;
                    leadside = side;
                }
            }
            else
            {
                // leave
                f = (d1 + SURFACE_CLIP_EPSILON) / (d1 - d2);
                if (f > 1)
                {
                    f = 1;
                }
                if (f < leaveFrac)
                {
                    leaveFrac = f;
                }
            }
        }
    }

    //
    // all planes have been checked, and the trace was not
    // completely outside the brush
    //
    if (!startout)
    {
        // original point was inside brush
        tw->trace.startsolid = qtrue;
        if (!getout)
        {
            tw->trace.allsolid = qtrue;
            tw->trace.fraction = 0;
            tw->trace.contents = brush->contents;
        }
        return;
    }

    if (enterFrac < leaveFrac)
    {
        if (enterFrac > -1 && enterFrac < tw->trace.fraction)
        {
            if (enterFrac < 0)
            {
                enterFrac = 0;
            }
            tw->trace.fraction = enterFrac;
            if (clipplane != NULL)
            {
                tw->trace.plane = *clipplane;
            }
            if (leadside != NULL)
            {
                tw->trace.surfaceFlags = leadside->surfaceFlags;
            }
            tw->trace.contents = brush->contents;
        }
    }
}

/*
================
CM_TraceThroughLeaf
================
*/
void CM_TraceThroughLeaf(traceWork_t* tw, cLeaf_t* leaf)
{
    int k;
    int brushnum;
    cbrush_t* b;
    cPatch_t* patch;

    // 1. Trace line/box against all solid brushes in the leaf
    for (k = 0; k < leaf->numLeafBrushes; k++)
    {
#ifndef Q3_VM
        // Cache Prefetch: Look exactly one loop iteration ahead and load the next brush structure into L1 cache
        if (k + 1 < leaf->numLeafBrushes)
        {
            int next_brushnum = cm.leafbrushes[leaf->firstLeafBrush + k + 1];
            _mm_prefetch((const char*)&cm.brushes[next_brushnum], _MM_HINT_T0);
        }
#endif
        brushnum = cm.leafbrushes[leaf->firstLeafBrush + k];
        b = &cm.brushes[brushnum];
        
        if (b->checkcount == cm.checkcount)
        {
            continue; // Already processed this frame
        }
        b->checkcount = cm.checkcount;

        if (!(b->contents & tw->contents))
        {
            continue; // Brush contents don't match trace mask
        }

        // Branch Hint: It's highly likely that the moving bounds do NOT intersect this specific brush
#if defined(__GNUC__) || defined(__clang__)
        if (__builtin_expect(!CM_BoundsIntersect(tw->bounds[0], tw->bounds[1], b->bounds[0], b->bounds[1]), 1))
#else
        if (!CM_BoundsIntersect(tw->bounds[0], tw->bounds[1], b->bounds[0], b->bounds[1]))
#endif
        {
            continue;
        }

        CM_TraceThroughBrush(tw, b);
        if (!tw->trace.fraction)
        {
            return; // Hit something completely solid at the start, drop out instantly
        }
    }

    // 2. Trace line/box against all patches/curves in the leaf
#ifdef BSPC
    if (1) {
#else
    if (!cm_noCurves->integer)
    {
#endif
        for (k = 0; k < leaf->numLeafSurfaces; k++)
        {
#ifndef Q3_VM
            // Cache Prefetch: Look ahead and load the next patch surface pointer
            if (k + 1 < leaf->numLeafSurfaces)
            {
                int next_surf = cm.leafsurfaces[leaf->firstLeafSurface + k + 1];
                _mm_prefetch((const char*)&cm.surfaces[next_surf], _MM_HINT_T0);
            }
#endif
            patch = cm.surfaces[cm.leafsurfaces[leaf->firstLeafSurface + k]];
            if (!patch)
            {
                continue;
            }
            if (patch->checkcount == cm.checkcount)
            {
                continue;
            }
            patch->checkcount = cm.checkcount;

            if (!(patch->contents & tw->contents))
            {
                continue;
            }

            CM_TraceThroughPatch(tw, patch);
            if (!tw->trace.fraction)
            {
                return;
            }
        }
    }
}

#define RADIUS_EPSILON      1.0f

/*
================
CM_TraceThroughSphere

get the first intersection of the ray with the sphere
================
*/
void CM_TraceThroughSphere(traceWork_t* tw, vec3_t origin, float radius, vec3_t start, vec3_t end)
{
    float l1, l2, length, scale, fraction;
    float b, c, d, sqrtd;
    vec3_t v1, dir, intersection;

    // 1. Utilize optimized parallel distance check to see if start is inside the sphere
    l1 = CM_VectorDistanceSquared(start, origin);
    if (l1 < Square(radius))
    {
        tw->trace.fraction = 0;
        tw->trace.startsolid = qtrue;
        
        // Test for allsolid using parallel distance evaluation
        l2 = CM_VectorDistanceSquared(end, origin);
        if (l2 < Square(radius))
        {
            tw->trace.allsolid = qtrue;
        }
        return;
    }

    VectorSubtract(end, start, dir);
    length = VectorNormalize(dir);

    l1 = CM_DistanceFromLineSquared(origin, start, end, dir);
    l2 = CM_VectorDistanceSquared(end, origin);
    
    // If no intersection with the sphere and the end point is at least an epsilon away
    if (l1 >= Square(radius) && l2 > Square(radius + SURFACE_CLIP_EPSILON))
    {
        return;
    }

    // Solve the quadratic intersection equation
    VectorSubtract(start, origin, v1);
    b = 2.0f * (dir[0] * v1[0] + dir[1] * v1[1] + dir[2] * v1[2]);
    c = v1[0] * v1[0] + v1[1] * v1[1] + v1[2] * v1[2] - (radius + RADIUS_EPSILON) * (radius + RADIUS_EPSILON);

    d = b * b - 4.0f * c;
    if (d > 0)
    {
        sqrtd = SquareRootFloat(d); // Uses our high-speed hardware _mm_sqrt_ss intrinsic
        fraction = (-b - sqrtd) * 0.5f;
        
        if (fraction < 0)
        {
            fraction = 0;
        }
        else
        {
            fraction /= length;
        }
        
        if (fraction < tw->trace.fraction)
        {
            tw->trace.fraction = fraction;
            VectorSubtract(end, start, dir);
            VectorMA(start, fraction, dir, intersection);
            VectorSubtract(intersection, origin, dir);

            scale = 1.0f / (radius + RADIUS_EPSILON);
            VectorScale(dir, scale, dir);
            VectorCopy(dir, tw->trace.plane.normal);
            VectorAdd(tw->modelOrigin, intersection, intersection);
            tw->trace.plane.dist = DotProduct(tw->trace.plane.normal, intersection);
            tw->trace.contents = CONTENTS_BODY;
        }
    }
}

/*
================
CM_TraceThroughVerticalCylinder

get the first intersection of the ray with the cylinder
the cylinder extends halfheight above and below the origin
================
*/
void CM_TraceThroughVerticalCylinder(traceWork_t* tw, vec3_t origin, float radius, float halfheight, vec3_t start, vec3_t end)
{
    float length, scale, fraction, l1, l2;
    float b, c, d, sqrtd;
    vec3_t v1, dir, start2d, end2d, org2d, intersection;

    // 1. Flatten coordinates to 2D space
    VectorSet(start2d, start[0], start[1], 0);
    VectorSet(end2d, end[0], end[1], 0);
    VectorSet(org2d, origin[0], origin[1], 0);

    // 2. Bound matching check
    if (start[2] <= origin[2] + halfheight && start[2] >= origin[2] - halfheight)
    {
        l1 = CM_VectorDistanceSquared(start2d, org2d);
        if (l1 < Square(radius))
        {
            tw->trace.fraction = 0;
            tw->trace.startsolid = qtrue;
            
            if (CM_VectorDistanceSquared(end2d, org2d) < Square(radius))
            {
                tw->trace.allsolid = qtrue;
            }
            return;
        }
    }

    VectorSubtract(end2d, start2d, dir);
    length = VectorNormalize(dir);

    l1 = CM_DistanceFromLineSquared(org2d, start2d, end2d, dir);
    l2 = CM_VectorDistanceSquared(end2d, org2d);
    
    if (l1 >= Square(radius) && l2 > Square(radius + SURFACE_CLIP_EPSILON))
    {
        return;
    }

    VectorSubtract(start, origin, v1);
    b = 2.0f * (v1[0] * dir[0] + v1[1] * dir[1]);
    c = v1[0] * v1[0] + v1[1] * v1[1] - (radius + RADIUS_EPSILON) * (radius + RADIUS_EPSILON);

    d = b * b - 4.0f * c;
    if (d > 0)
    {
        sqrtd = SquareRootFloat(d);
        fraction = (-b - sqrtd) * 0.5f;
        
        if (fraction < 0)
        {
            fraction = 0;
        }
        else
        {
            fraction /= length;
        }
        
        if (fraction < tw->trace.fraction)
        {
            VectorSubtract(end, start, dir);
            VectorMA(start, fraction, dir, intersection);
            
            if (intersection[2] <= origin[2] + halfheight && intersection[2] >= origin[2] - halfheight)
            {
                tw->trace.fraction = fraction;
                VectorSubtract(intersection, origin, dir);
                dir[2] = 0;

                scale = 1.0f / (radius + RADIUS_EPSILON);
                VectorScale(dir, scale, dir);
                VectorCopy(dir, tw->trace.plane.normal);
                VectorAdd(tw->modelOrigin, intersection, intersection);
                tw->trace.plane.dist = DotProduct(tw->trace.plane.normal, intersection);
                tw->trace.contents = CONTENTS_BODY;
            }
        }
    }
}

/*
================
CM_TraceCapsuleThroughCapsule

capsule vs. capsule collision (not rotated)
================
*/
void CM_TraceCapsuleThroughCapsule(traceWork_t* tw, clipHandle_t model)
{
    int i;
    vec3_t mins, maxs;
    vec3_t top, bottom, starttop, startbottom, endtop, endbottom;
    vec3_t offset, symetricSize[2];
    float radius, halfwidth, halfheight, offs, h;

    CM_ModelBounds(model, mins, maxs);

    // Test trace bounds vs capsule bounds using direct, branch-free logic
    if (tw->bounds[0][0] > maxs[0] + RADIUS_EPSILON
        || tw->bounds[0][1] > maxs[1] + RADIUS_EPSILON
        || tw->bounds[0][2] > maxs[2] + RADIUS_EPSILON
        || tw->bounds[1][0] < mins[0] - RADIUS_EPSILON
        || tw->bounds[1][1] < mins[1] - RADIUS_EPSILON
        || tw->bounds[1][2] < mins[2] - RADIUS_EPSILON)
    {
        return;
    }

    VectorAdd(tw->start, tw->sphere.offset, starttop);
    VectorSubtract(tw->start, tw->sphere.offset, startbottom);
    VectorAdd(tw->end, tw->sphere.offset, endtop);
    VectorSubtract(tw->end, tw->sphere.offset, endbottom);

    // Unroll coordinate indexing entirely
    offset[0] = (mins[0] + maxs[0]) * 0.5f;
    offset[1] = (mins[1] + maxs[1]) * 0.5f;
    offset[2] = (mins[2] + maxs[2]) * 0.5f;

    symetricSize[0][0] = mins[0] - offset[0]; symetricSize[0][1] = mins[1] - offset[1]; symetricSize[0][2] = mins[2] - offset[2];
    symetricSize[1][0] = maxs[0] - offset[0]; symetricSize[1][1] = maxs[1] - offset[1]; symetricSize[1][2] = maxs[2] - offset[2];

    halfwidth = symetricSize[1][0];
    halfheight = symetricSize[1][2];
    radius = (halfwidth > halfheight) ? halfheight : halfwidth;
    offs = halfheight - radius;

    VectorCopy(offset, top);
    top[2] += offs;
    VectorCopy(offset, bottom);
    bottom[2] -= offs;

    radius += tw->sphere.radius;

    if (tw->start[0] != tw->end[0] || tw->start[1] != tw->end[1])
    {
        h = halfheight + tw->sphere.halfheight - radius;
        if (h > 0)
        {
            CM_TraceThroughVerticalCylinder(tw, offset, radius, h, tw->start, tw->end);
        }
    }

    CM_TraceThroughSphere(tw, top, radius, startbottom, endbottom);
    CM_TraceThroughSphere(tw, bottom, radius, starttop, endtop);
}

/*
================
CM_TraceBoundingBoxThroughCapsule

bounding box vs. capsule collision
================
*/
void CM_TraceBoundingBoxThroughCapsule(traceWork_t* tw, clipHandle_t model)
{
    vec3_t mins, maxs, offset, size[2];
    clipHandle_t h;
    cmodel_t* cmod;

    CM_ModelBounds(model, mins, maxs);

    // Unroll index loops to let compiler map straight to raw registers
    offset[0] = (mins[0] + maxs[0]) * 0.5f;
    offset[1] = (mins[1] + maxs[1]) * 0.5f;
    offset[2] = (mins[2] + maxs[2]) * 0.5f;

    size[0][0] = mins[0] - offset[0]; size[0][1] = mins[1] - offset[1]; size[0][2] = mins[2] - offset[2];
    size[1][0] = maxs[0] - offset[0]; size[1][1] = maxs[1] - offset[1]; size[1][2] = maxs[2] - offset[2];
    
    tw->start[0] -= offset[0]; tw->start[1] -= offset[1]; tw->start[2] -= offset[2];
    tw->end[0]   -= offset[0]; tw->end[1]   -= offset[1]; tw->end[2]   -= offset[2];

    tw->sphere.use = qtrue;
    tw->sphere.radius = (size[1][0] > size[1][2]) ? size[1][2] : size[1][0];
    tw->sphere.halfheight = size[1][2];
    VectorSet(tw->sphere.offset, 0, 0, size[1][2] - tw->sphere.radius);

    h = CM_TempBoxModel(tw->size[0], tw->size[1], qfalse);
    cmod = CM_ClipHandleToModel(h);
    CM_TraceThroughLeaf(tw, &cmod->leaf);
}

//=========================================================================================

/*
==================
CM_TraceThroughTree

Traverse all the contacted leafs from the start to the end position.
If the trace is a point, they will be exactly in order, but for larger
trace volumes it is possible to hit something in a later leaf with
a smaller intercept fraction.
==================
*/
void CM_TraceThroughTree( traceWork_t *tw, int num, float p1f, float p2f, vec3_t p1, vec3_t p2 ) {
	cNode_t     *node;
	cplane_t    *plane;
	float t1, t2, offset;
	float frac, frac2;
	float idist;
	vec3_t mid;
	int side;
	float midf;

	if ( tw->trace.fraction <= p1f ) {
		return;     // already hit something nearer
	}

	// if < 0, we are in a leaf node
	if ( num < 0 ) {
		CM_TraceThroughLeaf( tw, &cm.leafs[-1 - num] );
		return;
	}

	//
	// find the point distances to the separating plane
	// and the offset for the size of the box
	//
	node = cm.nodes + num;
	plane = node->plane;

	// adjust the plane distance apropriately for mins/maxs
	if ( plane->type < 3 ) {
		t1 = p1[plane->type] - plane->dist;
		t2 = p2[plane->type] - plane->dist;
		offset = tw->extents[plane->type];
	} else {
		t1 = DotProduct( plane->normal, p1 ) - plane->dist;
		t2 = DotProduct( plane->normal, p2 ) - plane->dist;
		if ( tw->isPoint ) {
			offset = 0;
		} else {
			// this is silly
			offset = 2048;
		}
	}

	// see which sides we need to consider
	if ( t1 >= offset + 1 && t2 >= offset + 1 ) {
		CM_TraceThroughTree( tw, node->children[0], p1f, p2f, p1, p2 );
		return;
	}
	if ( t1 < -offset - 1 && t2 < -offset - 1 ) {
		CM_TraceThroughTree( tw, node->children[1], p1f, p2f, p1, p2 );
		return;
	}

	// put the crosspoint SURFACE_CLIP_EPSILON pixels on the near side
	if ( t1 < t2 ) {
		idist = 1.0 / ( t1 - t2 );
		side = 1;
		frac2 = ( t1 + offset + SURFACE_CLIP_EPSILON ) * idist;
		frac = ( t1 - offset + SURFACE_CLIP_EPSILON ) * idist;
	} else if ( t1 > t2 ) {
		idist = 1.0 / ( t1 - t2 );
		side = 0;
		frac2 = ( t1 - offset - SURFACE_CLIP_EPSILON ) * idist;
		frac = ( t1 + offset + SURFACE_CLIP_EPSILON ) * idist;
	} else {
		side = 0;
		frac = 1;
		frac2 = 0;
	}

	// move up to the node
	if ( frac < 0 ) {
		frac = 0;
	}
	if ( frac > 1 ) {
		frac = 1;
	}

	midf = p1f + ( p2f - p1f ) * frac;

	mid[0] = p1[0] + frac * ( p2[0] - p1[0] );
	mid[1] = p1[1] + frac * ( p2[1] - p1[1] );
	mid[2] = p1[2] + frac * ( p2[2] - p1[2] );

	CM_TraceThroughTree( tw, node->children[side], p1f, midf, p1, mid );


	// go past the node
	if ( frac2 < 0 ) {
		frac2 = 0;
	}
	if ( frac2 > 1 ) {
		frac2 = 1;
	}

	midf = p1f + ( p2f - p1f ) * frac2;

	mid[0] = p1[0] + frac2 * ( p2[0] - p1[0] );
	mid[1] = p1[1] + frac2 * ( p2[1] - p1[1] );
	mid[2] = p1[2] + frac2 * ( p2[2] - p1[2] );

	CM_TraceThroughTree( tw, node->children[side ^ 1], midf, p2f, mid, p2 );
}


//======================================================================


/*
==================
CM_Trace
==================
*/
void CM_Trace( trace_t *results, const vec3_t start, const vec3_t end,
			   const vec3_t mins, const vec3_t maxs,
			   clipHandle_t model, const vec3_t origin, int brushmask, int capsule, sphere_t *sphere ) {
	int i;
	traceWork_t tw;
	vec3_t offset;
	cmodel_t    *cmod;

	cmod = CM_ClipHandleToModel( model );

	cm.checkcount++;        // for multi-check avoidance

	c_traces++;             // for statistics, may be zeroed

	// fill in a default trace
	Com_Memset( &tw, 0, sizeof( tw ) );
	tw.trace.fraction = 1;  // assume it goes the entire distance until shown otherwise
	VectorCopy( origin, tw.modelOrigin );

	if ( !cm.numNodes ) {
		*results = tw.trace;

		return; // map not loaded, shouldn't happen
	}

	// allow NULL to be passed in for 0,0,0
	if ( !mins ) {
		mins = vec3_origin;
	}
	if ( !maxs ) {
		maxs = vec3_origin;
	}

	// set basic parms
	tw.contents = brushmask;

	// adjust so that mins and maxs are always symetric, which
	// avoids some complications with plane expanding of rotated
	// bmodels
	for ( i = 0 ; i < 3 ; i++ ) {
		offset[i] = ( mins[i] + maxs[i] ) * 0.5;
		tw.size[0][i] = mins[i] - offset[i];
		tw.size[1][i] = maxs[i] - offset[i];
		tw.start[i] = start[i] + offset[i];
		tw.end[i] = end[i] + offset[i];
	}

	// if a sphere is already specified
	if ( sphere ) {
		tw.sphere = *sphere;
	} else {
		tw.sphere.use = capsule;
		tw.sphere.radius = ( tw.size[1][0] > tw.size[1][2] ) ? tw.size[1][2] : tw.size[1][0];
		tw.sphere.halfheight = tw.size[1][2];
		VectorSet( tw.sphere.offset, 0, 0, tw.size[1][2] - tw.sphere.radius );
	}

	tw.maxOffset = tw.size[1][0] + tw.size[1][1] + tw.size[1][2];

	// tw.offsets[signbits] = vector to apropriate corner from origin
	tw.offsets[0][0] = tw.size[0][0];
	tw.offsets[0][1] = tw.size[0][1];
	tw.offsets[0][2] = tw.size[0][2];

	tw.offsets[1][0] = tw.size[1][0];
	tw.offsets[1][1] = tw.size[0][1];
	tw.offsets[1][2] = tw.size[0][2];

	tw.offsets[2][0] = tw.size[0][0];
	tw.offsets[2][1] = tw.size[1][1];
	tw.offsets[2][2] = tw.size[0][2];

	tw.offsets[3][0] = tw.size[1][0];
	tw.offsets[3][1] = tw.size[1][1];
	tw.offsets[3][2] = tw.size[0][2];

	tw.offsets[4][0] = tw.size[0][0];
	tw.offsets[4][1] = tw.size[0][1];
	tw.offsets[4][2] = tw.size[1][2];

	tw.offsets[5][0] = tw.size[1][0];
	tw.offsets[5][1] = tw.size[0][1];
	tw.offsets[5][2] = tw.size[1][2];

	tw.offsets[6][0] = tw.size[0][0];
	tw.offsets[6][1] = tw.size[1][1];
	tw.offsets[6][2] = tw.size[1][2];

	tw.offsets[7][0] = tw.size[1][0];
	tw.offsets[7][1] = tw.size[1][1];
	tw.offsets[7][2] = tw.size[1][2];

	//
	// calculate bounds
	//
	if ( tw.sphere.use ) {
		for ( i = 0 ; i < 3 ; i++ ) {
			if ( tw.start[i] < tw.end[i] ) {
				tw.bounds[0][i] = tw.start[i] - fabs( tw.sphere.offset[i] ) - tw.sphere.radius;
				tw.bounds[1][i] = tw.end[i] + fabs( tw.sphere.offset[i] ) + tw.sphere.radius;
			} else {
				tw.bounds[0][i] = tw.end[i] - fabs( tw.sphere.offset[i] ) - tw.sphere.radius;
				tw.bounds[1][i] = tw.start[i] + fabs( tw.sphere.offset[i] ) + tw.sphere.radius;
			}
		}
	} else {
		for ( i = 0 ; i < 3 ; i++ ) {
			if ( tw.start[i] < tw.end[i] ) {
				tw.bounds[0][i] = tw.start[i] + tw.size[0][i];
				tw.bounds[1][i] = tw.end[i] + tw.size[1][i];
			} else {
				tw.bounds[0][i] = tw.end[i] + tw.size[0][i];
				tw.bounds[1][i] = tw.start[i] + tw.size[1][i];
			}
		}
	}

	//
	// check for position test special case
	//
	if ( start[0] == end[0] && start[1] == end[1] && start[2] == end[2] ) {
		if ( model ) {
#ifdef ALWAYS_BBOX_VS_BBOX
			if ( model == BOX_MODEL_HANDLE || model == CAPSULE_MODEL_HANDLE ) {
				tw.sphere.use = qfalse;
				CM_TestInLeaf( &tw, &cmod->leaf );
			} else
#elif defined( ALWAYS_CAPSULE_VS_CAPSULE )
			if ( model == BOX_MODEL_HANDLE || model == CAPSULE_MODEL_HANDLE ) {
				CM_TestCapsuleInCapsule( &tw, model );
			} else
#else
			if ( model == CAPSULE_MODEL_HANDLE ) {
				if ( tw.sphere.use ) {
					CM_TestCapsuleInCapsule( &tw, model );
				} else {
					CM_TestBoundingBoxInCapsule( &tw, model );
				}
			} else
#endif
			{
				CM_TestInLeaf( &tw, &cmod->leaf );
			}
		} else {
			CM_PositionTest( &tw );
		}
	} else {
		//
		// check for point special case
		//
		if ( tw.size[0][0] == 0 && tw.size[0][1] == 0 && tw.size[0][2] == 0 ) {
			tw.isPoint = qtrue;
			VectorClear( tw.extents );
		} else {
			tw.isPoint = qfalse;
			tw.extents[0] = tw.size[1][0];
			tw.extents[1] = tw.size[1][1];
			tw.extents[2] = tw.size[1][2];
		}

		//
		// general sweeping through world
		//
		if ( model ) {
#ifdef ALWAYS_BBOX_VS_BBOX
			if ( model == BOX_MODEL_HANDLE || model == CAPSULE_MODEL_HANDLE ) {
				tw.sphere.use = qfalse;
				CM_TraceThroughLeaf( &tw, &cmod->leaf );
			} else
#elif defined( ALWAYS_CAPSULE_VS_CAPSULE )
			if ( model == BOX_MODEL_HANDLE || model == CAPSULE_MODEL_HANDLE ) {
				CM_TraceCapsuleThroughCapsule( &tw, model );
			} else
#else
			if ( model == CAPSULE_MODEL_HANDLE ) {
				if ( tw.sphere.use ) {
					CM_TraceCapsuleThroughCapsule( &tw, model );
				} else {
					CM_TraceBoundingBoxThroughCapsule( &tw, model );
				}
			} else
#endif
			{
				CM_TraceThroughLeaf( &tw, &cmod->leaf );
			}
		} else {
			CM_TraceThroughTree( &tw, 0, 0, 1, tw.start, tw.end );
		}
	}

	// generate endpos from the original, unmodified start/end
	if ( tw.trace.fraction == 1 ) {
		VectorCopy( end, tw.trace.endpos );
	} else {
		for ( i = 0 ; i < 3 ; i++ ) {
			tw.trace.endpos[i] = start[i] + tw.trace.fraction * ( end[i] - start[i] );
		}
	}

	*results = tw.trace;
}

/*
==================
CM_BoxTrace
==================
*/
void CM_BoxTrace( trace_t *results, const vec3_t start, const vec3_t end,
				  const vec3_t mins, const vec3_t maxs,
				  clipHandle_t model, int brushmask, int capsule ) {
	CM_Trace( results, start, end, mins, maxs, model, vec3_origin, brushmask, capsule, NULL );
}

/*
==================
CM_TransformedBoxTrace

Handles offseting and rotation of the end points for moving and
rotating entities
==================
*/
void CM_TransformedBoxTrace( trace_t *results, const vec3_t start, const vec3_t end,
							 const vec3_t mins, const vec3_t maxs,
							 clipHandle_t model, int brushmask,
							 const vec3_t origin, const vec3_t angles, int capsule ) {
	trace_t trace;
	vec3_t start_l, end_l;
	qboolean rotated;
	vec3_t offset;
	vec3_t symetricSize[2];
	vec3_t matrix[3], transpose[3];
	int i;
	float halfwidth;
	float halfheight;
	float t;
	sphere_t sphere;

	if ( !mins ) {
		mins = vec3_origin;
	}
	if ( !maxs ) {
		maxs = vec3_origin;
	}

	// adjust so that mins and maxs are always symetric, which
	// avoids some complications with plane expanding of rotated
	// bmodels
	for ( i = 0 ; i < 3 ; i++ ) {
		offset[i] = ( mins[i] + maxs[i] ) * 0.5;
		symetricSize[0][i] = mins[i] - offset[i];
		symetricSize[1][i] = maxs[i] - offset[i];
		start_l[i] = start[i] + offset[i];
		end_l[i] = end[i] + offset[i];
	}

	// subtract origin offset
	VectorSubtract( start_l, origin, start_l );
	VectorSubtract( end_l, origin, end_l );

	// rotate start and end into the models frame of reference
	if ( model != BOX_MODEL_HANDLE &&
		 ( angles[0] || angles[1] || angles[2] ) ) {
		rotated = qtrue;
	} else {
		rotated = qfalse;
	}

	halfwidth = symetricSize[ 1 ][ 0 ];
	halfheight = symetricSize[ 1 ][ 2 ];

	sphere.use = capsule;
	sphere.radius = ( halfwidth > halfheight ) ? halfheight : halfwidth;
	sphere.halfheight = halfheight;
	t = halfheight - sphere.radius;

	if ( rotated ) {
		// rotation on trace line (start-end) instead of rotating the bmodel
		// NOTE: This is still incorrect for bounding boxes because the actual bounding
		//		 box that is swept through the model is not rotated. We cannot rotate
		//		 the bounding box or the bmodel because that would make all the brush
		//		 bevels invalid.
		//		 However this is correct for capsules since a capsule itself is rotated too.
		CreateRotationMatrix( angles, matrix );
		RotatePoint( start_l, matrix );
		RotatePoint( end_l, matrix );
		// rotated sphere offset for capsule
		sphere.offset[0] = matrix[0][ 2 ] * t;
		sphere.offset[1] = -matrix[1][ 2 ] * t;
		sphere.offset[2] = matrix[2][ 2 ] * t;
	} else {
		VectorSet( sphere.offset, 0, 0, t );
	}

	// sweep the box through the model
	CM_Trace( &trace, start_l, end_l, symetricSize[0], symetricSize[1], model, origin, brushmask, capsule, &sphere );

	// if the bmodel was rotated and there was a collision
	if ( rotated && trace.fraction != 1.0 ) {
		// rotation of bmodel collision plane
		TransposeMatrix( matrix, transpose );
		RotatePoint( trace.plane.normal, transpose );
	}

	// re-calculate the end position of the trace because the trace.endpos
	// calculated by CM_Trace could be rotated and have an offset
	trace.endpos[0] = start[0] + trace.fraction * ( end[0] - start[0] );
	trace.endpos[1] = start[1] + trace.fraction * ( end[1] - start[1] );
	trace.endpos[2] = start[2] + trace.fraction * ( end[2] - start[2] );

	*results = trace;
}
