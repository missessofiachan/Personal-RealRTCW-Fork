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

// tr_surf.c
#include "tr_local.h"

/*

  THIS ENTIRE FILE IS BACK END

backEnd.currentEntity will be valid.

Tess_Begin has already been called for the surface's shader.

The modelview matrix will be set.

It is safe to actually issue drawing commands here if you don't want to
use the shader system.
*/


//============================================================================


/*
==============
RB_CheckOverflow
==============
*/
void RB_CheckOverflow( int verts, int indexes ) {
	// Performance hint: buffer space availability is the highly expected outcome
	if ( __builtin_expect( ( tess.numVertexes + verts < SHADER_MAX_VERTEXES ) &&
	                       ( tess.numIndexes + indexes < SHADER_MAX_INDEXES ), 1 ) ) {
		return;
	}

	RB_EndSurface();

	if ( verts >= SHADER_MAX_VERTEXES ) {
		ri.Error( ERR_DROP, "RB_CheckOverflow: verts > MAX (%d > %d)", verts, SHADER_MAX_VERTEXES );
	}
	if ( indexes >= SHADER_MAX_INDEXES ) {
		ri.Error( ERR_DROP, "RB_CheckOverflow: indices > MAX (%d > %d)", indexes, SHADER_MAX_INDEXES );
	}

	RB_BeginSurface( tess.shader, tess.fogNum );
}

/*
==============
RB_AddQuadStampExt
==============
*/
void RB_AddQuadStampExt( vec3_t origin, vec3_t left, vec3_t up, byte *color, float s1, float t1, float s2, float t2 ) {
	vec3_t normal;
	int ndx;

	RB_CHECKOVERFLOW( 4, 6 );

	ndx = tess.numVertexes;

	// Triangle indexing mapping layout for the quad face
	tess.indexes[ tess.numIndexes ]     = ndx;
	tess.indexes[ tess.numIndexes + 1 ] = ndx + 1;
	tess.indexes[ tess.numIndexes + 2 ] = ndx + 3;
	tess.indexes[ tess.numIndexes + 3 ] = ndx + 3;
	tess.indexes[ tess.numIndexes + 4 ] = ndx + 1;
	tess.indexes[ tess.numIndexes + 5 ] = ndx + 2;

	// Geometric coordinate mapping vectors
	tess.xyz[ndx][0] = origin[0] + left[0] + up[0];
	tess.xyz[ndx][1] = origin[1] + left[1] + up[1];
	tess.xyz[ndx][2] = origin[2] + left[2] + up[2];

	tess.xyz[ndx + 1][0] = origin[0] - left[0] + up[0];
	tess.xyz[ndx + 1][1] = origin[1] - left[1] + up[1];
	tess.xyz[ndx + 1][2] = origin[2] - left[2] + up[2];

	tess.xyz[ndx + 2][0] = origin[0] - left[0] - up[0];
	tess.xyz[ndx + 2][1] = origin[1] - left[1] - up[1];
	tess.xyz[ndx + 2][2] = origin[2] - left[2] - up[2];

	tess.xyz[ndx + 3][0] = origin[0] + left[0] - up[0];
	tess.xyz[ndx + 3][1] = origin[1] + left[1] - up[1];
	tess.xyz[ndx + 3][2] = origin[2] + left[2] - up[2];

	// Compute uniform normal facing relative to the player viewpoint axis
	VectorSubtract( vec3_origin, backEnd.viewParms.or.axis[0], normal );
	tess.normal[ndx][0] = tess.normal[ndx + 1][0] = tess.normal[ndx + 2][0] = tess.normal[ndx + 3][0] = normal[0];
	tess.normal[ndx][1] = tess.normal[ndx + 1][1] = tess.normal[ndx + 2][1] = tess.normal[ndx + 3][1] = normal[1];
	tess.normal[ndx][2] = tess.normal[ndx + 1][2] = tess.normal[ndx + 2][2] = tess.normal[ndx + 3][2] = normal[2];

	// Map texture lookups natively into dual coordinates channels
	tess.texCoords[ndx][0][0] = tess.texCoords[ndx][1][0] = s1;
	tess.texCoords[ndx][0][1] = tess.texCoords[ndx][1][1] = t1;

	tess.texCoords[ndx + 1][0][0] = tess.texCoords[ndx + 1][1][0] = s2;
	tess.texCoords[ndx + 1][0][1] = tess.texCoords[ndx + 1][1][1] = t1;

	tess.texCoords[ndx + 2][0][0] = tess.texCoords[ndx + 2][1][0] = s2;
	tess.texCoords[ndx + 2][0][1] = tess.texCoords[ndx + 2][1][1] = t2;

	tess.texCoords[ndx + 3][0][0] = tess.texCoords[ndx + 3][1][0] = s1;
	tess.texCoords[ndx + 3][0][1] = tess.texCoords[ndx + 3][1][1] = t2;

	// Vectorized 32-bit Integer color assignment block
	unsigned int *colorPtr = (unsigned int *)&tess.vertexColors[ndx];
	unsigned int packedColor = *(unsigned int *)color;
	colorPtr[0] = packedColor;
	colorPtr[1] = packedColor;
	colorPtr[2] = packedColor;
	colorPtr[3] = packedColor;

	tess.numVertexes += 4;
	tess.numIndexes  += 6;
}

/*
==============
RB_AddQuadStamp
==============
*/
void RB_AddQuadStamp( vec3_t origin, vec3_t left, vec3_t up, byte *color ) {
	RB_AddQuadStampExt( origin, left, up, color, 0.0f, 0.0f, 1.0f, 1.0f );
}

/*
==============
RB_SurfaceSplash
==============
*/
static void RB_SurfaceSplash( void ) {
	vec3_t left, up;
	float radius;

	// calculate the xyz locations for the four corners
	radius = backEnd.currentEntity->e.radius;

	VectorSet( left, -radius, 0, 0 );
	VectorSet( up, 0, radius, 0 );
	if ( backEnd.viewParms.isMirror ) {
		VectorSubtract( vec3_origin, left, left );
	}

	RB_AddQuadStamp( backEnd.currentEntity->e.origin, left, up, backEnd.currentEntity->e.shaderRGBA );
}

/*
==============
RB_SurfaceSprite
==============
*/
static void RB_SurfaceSprite( void ) {
	vec3_t left, up;
	float radius;
	refEntity_t *currEnt = &backEnd.currentEntity->e;

	radius = currEnt->radius;
	if ( currEnt->rotation == 0.0f ) {
		VectorScale( backEnd.viewParms.or.axis[1], radius, left );
		VectorScale( backEnd.viewParms.or.axis[2], radius, up );
	} else {
		float s, c;
		float ang = currEnt->rotation * (M_PI / 180.0f);

		s = sinf( ang );
		c = cosf( ang );

		float cRadius = c * radius;
		float sRadius = s * radius;

		left[0] = (backEnd.viewParms.or.axis[1][0] * cRadius) - (backEnd.viewParms.or.axis[2][0] * sRadius);
		left[1] = (backEnd.viewParms.or.axis[1][1] * cRadius) - (backEnd.viewParms.or.axis[2][1] * sRadius);
		left[2] = (backEnd.viewParms.or.axis[1][2] * cRadius) - (backEnd.viewParms.or.axis[2][2] * sRadius);

		up[0]   = (backEnd.viewParms.or.axis[2][0] * cRadius) + (backEnd.viewParms.or.axis[1][0] * sRadius);
		up[1]   = (backEnd.viewParms.or.axis[2][1] * cRadius) + (backEnd.viewParms.or.axis[1][1] * sRadius);
		up[2]   = (backEnd.viewParms.or.axis[2][2] * cRadius) + (backEnd.viewParms.or.axis[1][2] * sRadius);
	}
	
	if ( backEnd.viewParms.isMirror ) {
		VectorSubtract( vec3_origin, left, left );
	}

	RB_AddQuadStamp( currEnt->origin, left, up, currEnt->shaderRGBA );
}

/*
=============
RB_SurfacePolychain
=============
*/
static void RB_SurfacePolychain( srfPoly_t *p ) {
	int i;
	int numv;

	RB_CHECKOVERFLOW( p->numVerts, 3 * ( p->numVerts - 2 ) );

	numv = tess.numVertexes;
	for (i = 0; i < p->numVerts; i++) {
		int currentIdx = numv + i;
		VectorCopy(p->verts[i].xyz, tess.xyz[currentIdx]);

		tess.texCoords[currentIdx][0][0] = tess.texCoords[currentIdx][1][0] = p->verts[i].st[0];
		tess.texCoords[currentIdx][0][1] = tess.texCoords[currentIdx][1][1] = p->verts[i].st[1];

		tess.vertexColors[currentIdx][0] = R_GammaByte(p->verts[i].modulate[0]);
		tess.vertexColors[currentIdx][1] = R_GammaByte(p->verts[i].modulate[1]);
		tess.vertexColors[currentIdx][2] = R_GammaByte(p->verts[i].modulate[2]);
		tess.vertexColors[currentIdx][3] = p->verts[i].modulate[3];
	}

	int idxBase = tess.numIndexes;
	int vertMax = p->numVerts - 2;
	for ( i = 0; i < vertMax; i++ ) {
		tess.indexes[idxBase + 0] = tess.numVertexes;
		tess.indexes[idxBase + 1] = tess.numVertexes + i + 1;
		tess.indexes[idxBase + 2] = tess.numVertexes + i + 2;
		idxBase += 3;
	}

	tess.numIndexes  = idxBase;
	tess.numVertexes += p->numVerts;
}


/*
=============
RB_SurfaceTriangles
=============
*/
static void RB_SurfaceTriangles( srfTriangles_t *srf ) {
	int i;
	drawVert_t  *dv;
	float       *xyz, *normal, *texCoords;
	byte        *color;
	int         *vDlightBits;
	int         dlightBits;
	qboolean    needsNormal;
	int         baseVertex;

	dlightBits = srf->dlightBits;
	tess.dlightBits |= dlightBits;

	RB_CHECKOVERFLOW( srf->numVerts, srf->numIndexes );

	baseVertex = tess.numVertexes;

	// Unrolled/streamlined index mapping pass
	for ( i = 0 ; i < srf->numIndexes ; i += 3 ) {
		tess.indexes[ tess.numIndexes + i + 0 ] = baseVertex + srf->indexes[ i + 0 ];
		tess.indexes[ tess.numIndexes + i + 1 ] = baseVertex + srf->indexes[ i + 1 ];
		tess.indexes[ tess.numIndexes + i + 2 ] = baseVertex + srf->indexes[ i + 2 ];
	}
	tess.numIndexes += srf->numIndexes;

	dv = srf->verts;
	xyz = tess.xyz[ baseVertex ];
	normal = tess.normal[ baseVertex ];
	texCoords = tess.texCoords[ baseVertex ][0];
	color = tess.vertexColors[ baseVertex ];
	vDlightBits = &tess.vertexDlightBits[ baseVertex ];
	needsNormal = tess.shader->needsNormal;

	// Consolidated Unified Vertex Pass
	for ( i = 0 ; i < srf->numVerts ; i++, dv++, xyz += 4, normal += 4, texCoords += 4, color += 4 ) {
		xyz[0] = dv->xyz[0];
		xyz[1] = dv->xyz[1];
		xyz[2] = dv->xyz[2];

		if ( needsNormal ) {
			normal[0] = dv->normal[0];
			normal[1] = dv->normal[1];
			normal[2] = dv->normal[2];
		}

		// Combined multi-stage texture coordinate streaming
		texCoords[0] = dv->st[0];
		texCoords[1] = dv->st[1];
		texCoords[2] = dv->lightmap[0];
		texCoords[3] = dv->lightmap[1];

		// Packed 32-bit color transfer
		*(int *)color = *(int *)dv->color;

		// Consolidated from the legacy secondary loop - handled immediately in cache space
		*vDlightBits++ = dlightBits;
	}

	tess.numVertexes += srf->numVerts;
}

/*
=============
RB_SurfaceFoliage - ydnar
=============
*/

void RB_SurfaceFoliage(srfFoliage_t *srf)
{
	int               o, i;
	int               numVerts = srf->numVerts, numIndexes = srf->numIndexes;
	vec4_t            distanceCull, distanceVector;
	float             alpha, z, dist, fovScale = backEnd.viewParms.fovX * (1.0f / 90.0f);
	vec3_t            local;
	vec_t             *xyz;
	int               srcColor, *color;
	int               dlightBits;
	foliageInstance_t *instance;
	qboolean          shaderNeedsNormal;

	// Compute base distance vector relative to view parameters
	VectorSubtract(backEnd.or.origin, backEnd.viewParms.or.origin, local);
	distanceVector[0] = -backEnd.or.modelMatrix[2];
	distanceVector[1] = -backEnd.or.modelMatrix[6];
	distanceVector[2] = -backEnd.or.modelMatrix[10];
	distanceVector[3] = DotProduct(local, backEnd.viewParms.or.axis[0]);

	Vector4Copy(tess.shader->distanceCull, distanceCull);

	if (distanceCull[1] > 0.0f)
	{
		z     = fovScale * (DotProduct(srf->origin, distanceVector) + distanceVector[3] - srf->radius);
		alpha = (distanceCull[1] - z) * distanceCull[3];
		if (alpha < distanceCull[2])
		{
			return;
		}
	}

	dlightBits       = srf->dlightBits;
	tess.dlightBits |= dlightBits;
	shaderNeedsNormal = tess.shader->needsNormal;

	instance = srf->instances;
	for (o = 0; o < srf->numInstances; o++, instance++)
	{
		if (distanceCull[1] > 0.0f)
		{
			z = fovScale * (DotProduct(instance->origin, distanceVector) + distanceVector[3]);
			if (z < -64.0f) 
			{
				continue;
			}

			// Accelerated frustum culling boundary checks
			for (i = 0; i < MAX_FRUSTUM; i++)
			{
				dist = DotProduct(instance->origin, backEnd.viewParms.frustum[i].normal) - backEnd.viewParms.frustum[i].dist;
				if (dist < -64.0f)
				{
					break;
				}
			}
			if (i != MAX_FRUSTUM)
			{
				continue;
			}

			if (o & 1)
			{
				z *= 1.25f;
				if (o & 2)
				{
					z *= 1.25f;
				}
			}

			alpha = (distanceCull[1] - z) * distanceCull[3];
			if (alpha < distanceCull[2])
			{
				continue;
			}

			int a = alpha > 1.0f ? 255 : (int)(alpha * 255.0f);
#ifdef Q3_BIG_ENDIAN
			srcColor = (*((int*) instance->color) & 0xFFFFFF00) | (a & 0xff);
#else
			srcColor = (*((int *) instance->color) & 0xFFFFFF) | (a << 24);
#endif
		}
		else
		{
			srcColor = *((int *) instance->color);
		}

		RB_CHECKOVERFLOW(numVerts, numIndexes);
		tess.dlightBits |= dlightBits;

		// High-performance block memory indexing maps
		int currentVertexBase = tess.numVertexes;
		Com_Memcpy(&tess.indexes[tess.numIndexes], srf->indexes, numIndexes * sizeof(srf->indexes[0]));
		for (i = 0; i < numIndexes; i++)
		{
			tess.indexes[tess.numIndexes + i] += currentVertexBase;
		}

		// Aligned geometric tracking blocks copies
		xyz = tess.xyz[currentVertexBase];
		Com_Memcpy(xyz, srf->xyz, numVerts * sizeof(srf->xyz[0]));
		
		if (shaderNeedsNormal)
		{
			Com_Memcpy(&tess.normal[currentVertexBase], srf->normal, numVerts * sizeof(srf->normal[0]));
		}

		// Pointer tracking setup for texture streams
		float (*tCoords)[2][2] = &tess.texCoords[currentVertexBase];
		for (i = 0; i < numVerts; i++)
		{
			tCoords[i][0][0] = srf->texCoords[i][0];
			tCoords[i][0][1] = srf->texCoords[i][1];
			tCoords[i][1][0] = srf->lmTexCoords[i][0];
			tCoords[i][1][1] = srf->lmTexCoords[i][1];
		}

		// Fast vector alignment offset updates
		float instX = instance->origin[0];
		float instY = instance->origin[1];
		float instZ = instance->origin[2];
		for (i = 0; i < numVerts; i++, xyz += 4)
		{
			xyz[0] += instX;
			xyz[1] += instY;
			xyz[2] += instZ;
		}

		// Linear packed color assignments
		color = (int *) tess.vertexColors[currentVertexBase];
		for (i = 0; i < numVerts; i++)
		{
			color[i] = srcColor;
		}

		tess.numIndexes  += numIndexes;
		tess.numVertexes += numVerts;
	}
}


/*
==============
RB_SurfaceBeam
==============
*/
static void RB_SurfaceBeam( void ) {
#define NUM_BEAM_SEGS 6
	refEntity_t *e;
	int i;
	vec3_t perpvec;
	vec3_t direction, normalized_direction;
	vec3_t start_points[NUM_BEAM_SEGS], end_points[NUM_BEAM_SEGS];
	vec3_t oldorigin, origin;
	int vbase;
	unsigned int packedColor;

	e = &backEnd.currentEntity->e;

	VectorCopy(e->oldorigin, oldorigin);
	VectorCopy(e->origin, origin);

	VectorSubtract(oldorigin, origin, direction);
	VectorCopy(direction, normalized_direction);

	if ( VectorNormalize( normalized_direction ) == 0 ) {
		return;
	}

	PerpendicularVector( perpvec, normalized_direction );
	VectorScale( perpvec, 4.0f, perpvec );

	for ( i = 0; i < NUM_BEAM_SEGS ; i++ ) {
		RotatePointAroundVector( start_points[i], normalized_direction, perpvec, ( 360.0f / NUM_BEAM_SEGS ) * i );
		VectorAdd( start_points[i], direction, end_points[i] );
	}

	// Request structural memory allocations directly inside the unified global back-end array templates
	RB_CheckOverflow( NUM_BEAM_SEGS * 2, NUM_BEAM_SEGS * 6 );
	vbase = tess.numVertexes;
	packedColor = *(unsigned int *)e->shaderRGBA;

	// Populate vertex attributes directly into continuous memory buffers
	for ( i = 0; i < NUM_BEAM_SEGS; i++ ) {
		int nv = vbase + (i * 2);
		float texS = (float)i / NUM_BEAM_SEGS;
		
		// Beam Starting Root Point Nodes
		VectorCopy(start_points[i], tess.xyz[nv]);
		tess.texCoords[nv][0][0] = tess.texCoords[nv][1][0] = texS;
		tess.texCoords[nv][0][1] = tess.texCoords[nv][1][1] = 0.0f;
		*(unsigned int *)&tess.vertexColors[nv] = packedColor;

		// Beam Terminal Target Point Nodes
		VectorCopy(end_points[i], tess.xyz[nv + 1]);
		tess.texCoords[nv + 1][0][0] = tess.texCoords[nv + 1][1][0] = texS;
		tess.texCoords[nv + 1][0][1] = tess.texCoords[nv + 1][1][1] = 1.0f;
		*(unsigned int *)&tess.vertexColors[nv + 1] = packedColor;
	}

	// Dynamic quad layout triangulation loops to prevent CPU/GPU pipeline stalls
	int idxBase = tess.numIndexes;
	for ( i = 0; i < NUM_BEAM_SEGS; i++ ) {
		int iNext = (i + 1) % NUM_BEAM_SEGS;
		int v1 = vbase + (i * 2);
		int v2 = v1 + 1;
		int v3 = vbase + (iNext * 2);
		int v4 = v3 + 1;

		tess.indexes[idxBase + 0] = v1;
		tess.indexes[idxBase + 1] = v2;
		tess.indexes[idxBase + 2] = v3;

		tess.indexes[idxBase + 3] = v3;
		tess.indexes[idxBase + 4] = v2;
		tess.indexes[idxBase + 5] = v4;

		idxBase += 6;
	}

	tess.numIndexes  = idxBase;
	tess.numVertexes += (NUM_BEAM_SEGS * 2);
}

//================================================================================

static void DoRailCore( const vec3_t start, const vec3_t end, const vec3_t up, float len, float spanWidth ) {
	float spanWidth2;
	int vbase;
	float t = len / 256.0f;
	unsigned int packedCoreColor;
	unsigned int packedFullColor;
	byte coreColor[4];

	RB_CHECKOVERFLOW( 4, 6 );

	vbase = tess.numVertexes;
	spanWidth2 = -spanWidth;

	// Scale and pack colors safely into unified 32-bit blocks
	coreColor[0] = (byte)(backEnd.currentEntity->e.shaderRGBA[0] * 0.25f);
	coreColor[1] = (byte)(backEnd.currentEntity->e.shaderRGBA[1] * 0.25f);
	coreColor[2] = (byte)(backEnd.currentEntity->e.shaderRGBA[2] * 0.25f);
	coreColor[3] = backEnd.currentEntity->e.shaderRGBA[3];
	
	packedCoreColor = *(unsigned int *)coreColor;
	packedFullColor = *(unsigned int *)backEnd.currentEntity->e.shaderRGBA;

	// Populate the 4 quad corner vertices efficiently
	VectorMA( start, spanWidth, up, tess.xyz[vbase] );
	tess.texCoords[vbase][0][0] = tess.texCoords[vbase][1][0] = 0.0f;
	tess.texCoords[vbase][0][1] = tess.texCoords[vbase][1][1] = 0.0f;
	*(unsigned int *)&tess.vertexColors[vbase] = packedCoreColor;

	VectorMA( start, spanWidth2, up, tess.xyz[vbase + 1] );
	tess.texCoords[vbase + 1][0][0] = tess.texCoords[vbase + 1][1][0] = 0.0f;
	tess.texCoords[vbase + 1][0][1] = tess.texCoords[vbase + 1][1][1] = 1.0f;
	*(unsigned int *)&tess.vertexColors[vbase + 1] = packedFullColor;

	VectorMA( end, spanWidth, up, tess.xyz[vbase + 2] );
	tess.texCoords[vbase + 2][0][0] = tess.texCoords[vbase + 2][1][0] = t;
	tess.texCoords[vbase + 2][0][1] = tess.texCoords[vbase + 2][1][1] = 0.0f;
	*(unsigned int *)&tess.vertexColors[vbase + 2] = packedFullColor;

	VectorMA( end, spanWidth2, up, tess.xyz[vbase + 3] );
	tess.texCoords[vbase + 3][0][0] = tess.texCoords[vbase + 3][1][0] = t;
	tess.texCoords[vbase + 3][0][1] = tess.texCoords[vbase + 3][1][1] = 1.0f;
	*(unsigned int *)&tess.vertexColors[vbase + 3] = packedFullColor;

	// Fast index buffer serialization mapping
	int idxBase = tess.numIndexes;
	tess.indexes[idxBase + 0] = vbase;
	tess.indexes[idxBase + 1] = vbase + 1;
	tess.indexes[idxBase + 2] = vbase + 2;
	tess.indexes[idxBase + 3] = vbase + 2;
	tess.indexes[idxBase + 4] = vbase + 1;
	tess.indexes[idxBase + 5] = vbase + 3;

	tess.numIndexes  += 6;
	tess.numVertexes += 4;
}

static void DoRailDiscs( int numSegs, const vec3_t start, const vec3_t dir, const vec3_t right, const vec3_t up ) {
	int i;
	vec3_t pos[4];
	vec3_t v;
	int spanWidth = r_railWidth->integer;
	float scale = 0.25f;
	unsigned int packedColor;

	if ( numSegs > 1 ) {
		numSegs--;
	}
	if ( !numSegs ) {
		return;
	}

	// Hoist and pre-calculate structural angles using single-precision trigonometry
	float factor = scale * spanWidth;
	for ( i = 0; i < 4; i++ ) {
		float ang = (45.0f + i * 90.0f) * (M_PI / 180.0f);
		float c = cosf(ang);
		float s = sinf(ang);

		v[0] = ( right[0] * c + up[0] * s ) * factor;
		v[1] = ( right[1] * c + up[1] * s ) * factor;
		v[2] = ( right[2] * c + up[2] * s ) * factor;
		VectorAdd( start, v, pos[i] );

		if ( numSegs > 1 ) {
			VectorAdd( pos[i], dir, pos[i] );
		}
	}

	packedColor = *(unsigned int *)backEnd.currentEntity->e.shaderRGBA;

	// Stream aligned blocks into vertex array targets
	for ( i = 0; i < numSegs; i++ ) {
		int j;
		RB_CHECKOVERFLOW( 4, 6 );

		int vbase = tess.numVertexes;

		for ( j = 0; j < 4; j++ ) {
			int nv = vbase + j;
			VectorCopy( pos[j], tess.xyz[nv] );
			tess.texCoords[nv][0][0] = tess.texCoords[nv][1][0] = (float)( j < 2 );
			tess.texCoords[nv][0][1] = tess.texCoords[nv][1][1] = (float)( j && j != 3 );
			*(unsigned int *)&tess.vertexColors[nv] = packedColor;

			VectorAdd( pos[j], dir, pos[j] );
		}

		int idxBase = tess.numIndexes;
		tess.indexes[idxBase + 0] = vbase;
		tess.indexes[idxBase + 1] = vbase + 1;
		tess.indexes[idxBase + 2] = vbase + 3;
		tess.indexes[idxBase + 3] = vbase + 3;
		tess.indexes[idxBase + 4] = vbase + 1;
		tess.indexes[idxBase + 5] = vbase + 2;

		tess.numIndexes  += 6;
		tess.numVertexes += 4;
	}
}

/*
** RB_SurfaceRailRinges
*/
static void RB_SurfaceRailRings( void ) {
	refEntity_t *e = &backEnd.currentEntity->e;
	int numSegs, len;
	vec3_t vec, right, up;

	VectorSubtract( e->origin, e->oldorigin, vec );
	len = VectorNormalize( vec );
	MakeNormalVectors( vec, right, up );
	
	numSegs = len / r_railSegmentLength->value;
	if ( numSegs <= 0 ) {
		numSegs = 1;
	}

	VectorScale( vec, r_railSegmentLength->value, vec );
	DoRailDiscs( numSegs, e->oldorigin, vec, right, up );
}

static void RB_SurfaceRailCore( void ) {
	refEntity_t *e = &backEnd.currentEntity->e;
	int len;
	vec3_t right, vec, v1, v2;

	VectorSubtract( e->origin, e->oldorigin, vec );
	len = VectorNormalize( vec );

	VectorSubtract( e->oldorigin, backEnd.viewParms.or.origin, v1 );
	VectorNormalize( v1 );
	VectorSubtract( e->origin, backEnd.viewParms.or.origin, v2 );
	VectorNormalize( v2 );
	
	CrossProduct( v1, v2, right );
	VectorNormalize( right );

	DoRailCore( e->oldorigin, e->origin, right, (float)len, (float)r_railCoreWidth->integer );
}

static void RB_SurfaceLightningBolt( void ) {
	refEntity_t *e = &backEnd.currentEntity->e;
	int len, i;
	vec3_t right, vec, v1, v2;

	VectorSubtract( e->oldorigin, e->origin, vec );
	len = VectorNormalize( vec );

	VectorSubtract( e->origin, backEnd.viewParms.or.origin, v1 );
	VectorNormalize( v1 );
	VectorSubtract( e->oldorigin, backEnd.viewParms.or.origin, v2 );
	VectorNormalize( v2 );
	
	CrossProduct( v1, v2, right );
	VectorNormalize( right );

	for ( i = 0 ; i < 4 ; i++ ) {
		vec3_t temp;
		DoRailCore( e->origin, e->oldorigin, right, (float)len, 8.0f );
		RotatePointAroundVector( temp, vec, right, 45.0f );
		VectorCopy( temp, right );
	}
}

/*
** VectorArrayNormalize
*
* The inputs to this routing seem to always be close to length = 1.0 (about 0.6 to 2.0)
* This means that we don't have to worry about zero length or enormously long vectors.
*/
void VectorArrayNormalize( vec4_t *normals, unsigned int count ) {
	if ( __builtin_expect( count == 0, 0 ) ) {
		return;
	}

	// Unroll by 4 to max out L1 cache line throughput and hide pipeline execution latency
	while ( count >= 4 ) {
		VectorNormalizeFast( normals[0] );
		VectorNormalizeFast( normals[1] );
		VectorNormalizeFast( normals[2] );
		VectorNormalizeFast( normals[3] );
		normals += 4;
		count -= 4;
	}
	
	while ( count-- ) {
		VectorNormalizeFast( normals[0] );
		normals++;
	}
}



/*
** LerpMeshVertexes
*/
static void LerpMeshVertexes_scalar(md3Surface_t *surf, float backlerp)
{
	short	*__restrict oldXyz, *__restrict newXyz, *__restrict oldNormals, *__restrict newNormals;
	float	*__restrict outXyz, *__restrict outNormal;
	float	oldXyzScale, newXyzScale;
	float	oldNormalScale, newNormalScale;
	int		vertNum;
	unsigned lat, lng;
	int		numVerts = surf->numVerts;

	outXyz = tess.xyz[tess.numVertexes];
	outNormal = tess.normal[tess.numVertexes];

	newXyz = (short *)((byte *)surf + surf->ofsXyzNormals) + (backEnd.currentEntity->e.frame * numVerts * 4);
	newNormals = newXyz + 3;

	newXyzScale = MD3_XYZ_SCALE * (1.0f - backlerp);
	newNormalScale = 1.0f - backlerp;

	if ( backlerp == 0.0f ) {
		for (vertNum = 0; vertNum < numVerts; vertNum++) {
			outXyz[0] = newXyz[0] * newXyzScale;
			outXyz[1] = newXyz[1] * newXyzScale;
			outXyz[2] = newXyz[2] * newXyzScale;

			lat = ( newNormals[0] >> 8 ) & 0xff;
			lng = ( newNormals[0] & 0xff );
			lat <<= 2; // Fast bit shift instead of manual division/multiplication
			lng <<= 2;

			outNormal[0] = tr.sinTable[(lat + 256) & FUNCTABLE_MASK];
			outNormal[1] = tr.sinTable[lat] * tr.sinTable[lng];
			outNormal[2] = tr.sinTable[(lng + 256) & FUNCTABLE_MASK];

			newXyz += 4;
			newNormals += 4;
			outXyz += 4;
			outNormal += 4;
		}
	} else {
		oldXyz = (short *)((byte *)surf + surf->ofsXyzNormals) + (backEnd.currentEntity->e.oldframe * numVerts * 4);
		oldNormals = oldXyz + 3;

		oldXyzScale = MD3_XYZ_SCALE * backlerp;
		oldNormalScale = backlerp;

		for (vertNum = 0; vertNum < numVerts; vertNum++) {
			float nLatCos, nLatSin, nLngSin, nLngCos;
			float oLatCos, oLatSin, oLngSin, oLngCos;

			outXyz[0] = oldXyz[0] * oldXyzScale + newXyz[0] * newXyzScale;
			outXyz[1] = oldXyz[1] * oldXyzScale + newXyz[1] * newXyzScale;
			outXyz[2] = oldXyz[2] * oldXyzScale + newXyz[2] * newXyzScale;

			lat = ( newNormals[0] >> 8 ) & 0xff;
			lng = ( newNormals[0] & 0xff );
			lat <<= 2;
			lng <<= 2;
			
			nLatCos = tr.sinTable[(lat + 256) & FUNCTABLE_MASK];
			nLatSin = tr.sinTable[lat];
			nLngSin = tr.sinTable[lng];
			nLngCos = tr.sinTable[(lng + 256) & FUNCTABLE_MASK];

			lat = ( oldNormals[0] >> 8 ) & 0xff;
			lng = ( oldNormals[0] & 0xff );
			lat <<= 2;
			lng <<= 2;

			oLatCos = tr.sinTable[(lat + 256) & FUNCTABLE_MASK];
			oLatSin = tr.sinTable[lat];
			oLngSin = tr.sinTable[lng];
			oLngCos = tr.sinTable[(lng + 256) & FUNCTABLE_MASK];

			// Combine linear vertex shading steps natively
			outNormal[0] = (oLatCos * oLngSin * oldNormalScale) + (nLatCos * nLngSin * newNormalScale);
			outNormal[1] = (oLatSin * oLngSin * oldNormalScale) + (nLatSin * nLngSin * newNormalScale);
			outNormal[2] = (oLngCos * oldNormalScale) + (nLngCos * newNormalScale);

			oldXyz += 4;
			newXyz += 4;
			oldNormals += 4;
			newNormals += 4;
			outXyz += 4;
			outNormal += 4;
		}
		VectorArrayNormalize((vec4_t *)tess.normal[tess.numVertexes], numVerts);
	}
}

static void LerpMeshVertexes(md3Surface_t *surf, float backlerp)
{
#if idppc_altivec
	if (com_altivec->integer) {
		// must be in a separate translation unit or G3 systems will crash.
		LerpMeshVertexes_altivec( surf, backlerp );
		return;
	}
#endif // idppc_altivec
	LerpMeshVertexes_scalar( surf, backlerp );
}


/*
=============
RB_SurfaceMesh
=============
*/
static void RB_SurfaceMesh( md3Surface_t *surface ) {
	int j;
	float backlerp;
	int *triangles;
	float *texCoords;
	int indexes;
	int Bob, Doug;
	int numVerts;

	if ( backEnd.currentEntity->e.reFlags & REFLAG_ONLYHAND ) {
		if ( !strstr( surface->name, "hand" ) ) {
			return;
		}
	}

	backlerp = ( backEnd.currentEntity->e.oldframe == backEnd.currentEntity->e.frame ) ? 0.0f : backEnd.currentEntity->e.backlerp;

	RB_CHECKOVERFLOW( surface->numVerts, surface->numTriangles * 3 );

	LerpMeshVertexes( surface, backlerp );

	triangles = ( int * )( (byte *)surface + surface->ofsTriangles );
	indexes = surface->numTriangles * 3;
	Bob = tess.numIndexes;
	Doug = tess.numVertexes;
	
	for ( j = 0 ; j < indexes ; j++ ) {
		tess.indexes[Bob + j] = Doug + triangles[j];
	}
	tess.numIndexes += indexes;

	texCoords = ( float * )( (byte *)surface + surface->ofsSt );
	numVerts = surface->numVerts;

	// Simultaneously stream texture assignments to keep layout memory hot
	float *tessTex = &tess.texCoords[Doug][0][0];
	for ( j = 0; j < numVerts; j++ ) {
		float s = texCoords[j * 2 + 0];
		float t = texCoords[j * 2 + 1];
		
		tessTex[0] = tessTex[2] = s;
		tessTex[1] = tessTex[3] = t;
		tessTex += 4;
	}

	tess.numVertexes += surface->numVerts;
}

/*
** R_LatLongToNormal
*/
void R_LatLongToNormal( vec3_t outNormal, short latLong ) {
	unsigned lat = ( latLong >> 8 ) & 0xff;
	unsigned lng = ( latLong & 0xff );
	lat <<= 2;
	lng <<= 2;

	outNormal[0] = tr.sinTable[( lat + 256 ) & FUNCTABLE_MASK] * tr.sinTable[lng];
	outNormal[1] = tr.sinTable[lat] * tr.sinTable[lng];
	outNormal[2] = tr.sinTable[( lng + 256 ) & FUNCTABLE_MASK];
}

// Ridah
/*
** LerpCMeshVertexes
*/
static void LerpCMeshVertexes( mdcSurface_t *surf, float backlerp ) {
	short   *oldXyz, *newXyz, *oldNormals, *newNormals;
	float   *outXyz, *outNormal;
	float   oldXyzScale, newXyzScale;
	float   oldNormalScale, newNormalScale;
	int     vertNum;
	unsigned lat, lng;
	int     numVerts = surf->numVerts;

	int     oldBase, newBase;
	short   *oldComp = NULL, *newComp = NULL;
	mdcXyzCompressed_t *oldXyzComp = NULL, *newXyzComp = NULL;
	vec3_t  oldOfsVec, newOfsVec;
	qboolean hasComp;

	outXyz = tess.xyz[tess.numVertexes];
	outNormal = tess.normal[tess.numVertexes];

	newBase = (int)*( ( short * )( (byte *)surf + surf->ofsFrameBaseFrames ) + backEnd.currentEntity->e.frame );
	newXyz = ( short * )( (byte *)surf + surf->ofsXyzNormals ) + ( newBase * numVerts * 4 );
	newNormals = newXyz + 3;

	hasComp = ( surf->numCompFrames > 0 );
	if ( hasComp ) {
		newComp = ( ( short * )( (byte *)surf + surf->ofsFrameCompFrames ) + backEnd.currentEntity->e.frame );
		if ( *newComp >= 0 ) {
			newXyzComp = ( mdcXyzCompressed_t * )( (byte *)surf + surf->ofsXyzCompressed ) + ( *newComp * numVerts );
		}
	}

	newXyzScale = MD3_XYZ_SCALE * ( 1.0f - backlerp );
	newNormalScale = 1.0f - backlerp;

	if ( backlerp == 0.0f ) {
		// Hoisted compression check branch out of the execution loop path
		if ( hasComp && *newComp >= 0 ) {
			for ( vertNum = 0 ; vertNum < numVerts ; vertNum++, newXyz += 4, newNormals += 4, outXyz += 4, outNormal += 4 ) {
				outXyz[0] = newXyz[0] * newXyzScale;
				outXyz[1] = newXyz[1] * newXyzScale;
				outXyz[2] = newXyz[2] * newXyzScale;

				R_MDC_DecodeXyzCompressed( newXyzComp->ofsVec, newOfsVec, outNormal );
				newXyzComp++;
				VectorAdd( outXyz, newOfsVec, outXyz );
			}
		} else {
			for ( vertNum = 0 ; vertNum < numVerts ; vertNum++, newXyz += 4, newNormals += 4, outXyz += 4, outNormal += 4 ) {
				outXyz[0] = newXyz[0] * newXyzScale;
				outXyz[1] = newXyz[1] * newXyzScale;
				outXyz[2] = newXyz[2] * newXyzScale;

				lat = ( newNormals[0] >> 8 ) & 0xff;
				lng = ( newNormals[0] & 0xff );
				lat <<= 2;
				lng <<= 2;

				outNormal[0] = tr.sinTable[( lat + 256 ) & FUNCTABLE_MASK];
				outNormal[1] = tr.sinTable[lat] * tr.sinTable[lng];
				outNormal[2] = tr.sinTable[( lng + 256 ) & FUNCTABLE_MASK];
			}
		}
	} else {
		oldBase = (int)*( ( short * )( (byte *)surf + surf->ofsFrameBaseFrames ) + backEnd.currentEntity->e.oldframe );
		oldXyz = ( short * )( (byte *)surf + surf->ofsXyzNormals ) + ( oldBase * numVerts * 4 );
		oldNormals = oldXyz + 3;

		if ( hasComp ) {
			oldComp = ( ( short * )( (byte *)surf + surf->ofsFrameCompFrames ) + backEnd.currentEntity->e.oldframe );
			if ( *oldComp >= 0 ) {
				oldXyzComp = ( mdcXyzCompressed_t * )( (byte *)surf + surf->ofsXyzCompressed ) + ( *oldComp * numVerts );
            }
		}

		oldXyzScale = MD3_XYZ_SCALE * backlerp;
		oldNormalScale = backlerp;

		qboolean useNewComp = (hasComp && *newComp >= 0);
		qboolean useOldComp = (hasComp && *oldComp >= 0);

		for ( vertNum = 0 ; vertNum < numVerts ; vertNum++, oldXyz += 4, newXyz += 4, oldNormals += 4, newNormals += 4, outXyz += 4, outNormal += 4 ) {
			vec3_t uncompressedOldNormal, uncompressedNewNormal;

			outXyz[0] = oldXyz[0] * oldXyzScale + newXyz[0] * newXyzScale;
			outXyz[1] = oldXyz[1] * oldXyzScale + newXyz[1] * newXyzScale;
			outXyz[2] = oldXyz[2] * oldXyzScale + newXyz[2] * newXyzScale;

			if ( useNewComp ) {
				R_MDC_DecodeXyzCompressed( newXyzComp->ofsVec, newOfsVec, uncompressedNewNormal );
				newXyzComp++;
				VectorMA( outXyz, 1.0f - backlerp, newOfsVec, outXyz );
			} else {
				lat = ( newNormals[0] >> 8 ) & 0xff;
				lng = ( newNormals[0] & 0xff );
				lat <<= 2;
				lng <<= 2;
				uncompressedNewNormal[0] = tr.sinTable[( lat + 256 ) & FUNCTABLE_MASK];
				uncompressedNewNormal[1] = tr.sinTable[lat] * tr.sinTable[lng];
				uncompressedNewNormal[2] = tr.sinTable[( lng + 256 ) & FUNCTABLE_MASK];
			}

			if ( useOldComp ) {
				R_MDC_DecodeXyzCompressed( oldXyzComp->ofsVec, oldOfsVec, uncompressedOldNormal );
				oldXyzComp++;
				VectorMA( outXyz, backlerp, oldOfsVec, outXyz );
			} else {
				lat = ( oldNormals[0] >> 8 ) & 0xff;
				lng = ( oldNormals[0] & 0xff );
				lat <<= 2;
				lng <<= 2;
				uncompressedOldNormal[0] = tr.sinTable[( lat + 256 ) & FUNCTABLE_MASK];
				uncompressedOldNormal[1] = tr.sinTable[lat] * tr.sinTable[lng];
				uncompressedOldNormal[2] = tr.sinTable[( lng + 256 ) & FUNCTABLE_MASK];
			}

			outNormal[0] = uncompressedOldNormal[0] * oldNormalScale + uncompressedNewNormal[0] * newNormalScale;
			outNormal[1] = uncompressedOldNormal[1] * oldNormalScale + uncompressedNewNormal[1] * newNormalScale;
			outNormal[2] = uncompressedOldNormal[2] * oldNormalScale + uncompressedNewNormal[2] * newNormalScale;

			VectorNormalize( outNormal );
		}
	}
}

/*
=============
RB_SurfaceCMesh
=============
*/
void RB_SurfaceCMesh( mdcSurface_t *surface ) {
	int j;
	float backlerp;
	int             *triangles;
	float           *texCoords;
	int indexes;
	int Bob, Doug;
	int numVerts;

	if ( backEnd.currentEntity->e.reFlags & REFLAG_ONLYHAND ) {
		if ( !strstr( surface->name, "hand" ) ) {
			return;
		}
	}

	backlerp = ( backEnd.currentEntity->e.oldframe == backEnd.currentEntity->e.frame ) ? 0.0f : backEnd.currentEntity->e.backlerp;

	RB_CHECKOVERFLOW( surface->numVerts, surface->numTriangles * 3 );

	LerpCMeshVertexes( surface, backlerp );

	triangles = ( int * )( (byte *)surface + surface->ofsTriangles );
	indexes = surface->numTriangles * 3;
	Bob = tess.numIndexes;
	Doug = tess.numVertexes;
	
	for ( j = 0 ; j < indexes ; j++ ) {
		tess.indexes[Bob + j] = Doug + triangles[j];
	}
	tess.numIndexes += indexes;

	texCoords = ( float * )( (byte *)surface + surface->ofsSt );
	numVerts = surface->numVerts;

	float *tessTex = &tess.texCoords[Doug][0][0];
	for ( j = 0; j < numVerts; j++ ) {
		float s = texCoords[j * 2 + 0];
		float t = texCoords[j * 2 + 1];
		
		tessTex[0] = tessTex[2] = s;
		tessTex[1] = tessTex[3] = t;
		tessTex += 4;
	}

	tess.numVertexes += surface->numVerts;
}
// done.

/*
==============
RB_SurfaceFace
==============
*/
void RB_SurfaceFace( srfSurfaceFace_t *surf ) {
	int i;
	unsigned	*indices;
	glIndex_t	*tessIndexes;
	float       *v;
	float       *normal;
	int ndx;
	int Bob;
	int numPoints;
	int dlightBits;

	RB_CHECKOVERFLOW( surf->numPoints, surf->numIndices );

	dlightBits = surf->dlightBits;
	tess.dlightBits |= dlightBits;

	indices = ( unsigned * )( ( ( char  * ) surf ) + surf->ofsIndices );

	Bob = tess.numVertexes;
	tessIndexes = tess.indexes + tess.numIndexes;
	for ( i = surf->numIndices - 1 ; i >= 0  ; i-- ) {
		tessIndexes[i] = indices[i] + Bob;
	}

	tess.numIndexes += surf->numIndices;

	numPoints = surf->numPoints;

	if ( tess.shader->needsNormal ) {
		normal = surf->plane.normal;
		for ( i = 0, ndx = tess.numVertexes; i < numPoints; i++, ndx++ ) {
			VectorCopy( normal, tess.normal[ndx] );
		}
	}

	for ( i = 0, v = surf->points[0], ndx = tess.numVertexes; i < numPoints; i++, v += VERTEXSIZE, ndx++ ) {
		VectorCopy( v, tess.xyz[ndx] );
		tess.texCoords[ndx][0][0] = v[3];
		tess.texCoords[ndx][0][1] = v[4];
		tess.texCoords[ndx][1][0] = v[5];
		tess.texCoords[ndx][1][1] = v[6];
		*( unsigned int * ) &tess.vertexColors[ndx] = *( unsigned int * ) &v[7];
		tess.vertexDlightBits[ndx] = dlightBits;
	}


	tess.numVertexes += surf->numPoints;
}


static float    LodErrorForVolume( vec3_t local, float radius ) {
	vec3_t world;
	float d;

	// never let it go negative
	if ( r_lodCurveError->value < 0 ) {
		return 0;
	}

	world[0] = local[0] * backEnd.or.axis[0][0] + local[1] * backEnd.or.axis[1][0] +
			   local[2] * backEnd.or.axis[2][0] + backEnd.or.origin[0];
	world[1] = local[0] * backEnd.or.axis[0][1] + local[1] * backEnd.or.axis[1][1] +
			   local[2] * backEnd.or.axis[2][1] + backEnd.or.origin[1];
	world[2] = local[0] * backEnd.or.axis[0][2] + local[1] * backEnd.or.axis[1][2] +
			   local[2] * backEnd.or.axis[2][2] + backEnd.or.origin[2];

	VectorSubtract( world, backEnd.viewParms.or.origin, world );
	d = DotProduct( world, backEnd.viewParms.or.axis[0] );

	if ( d < 0 ) {
		d = -d;
	}
	d -= radius;
	if ( d < 1 ) {
		d = 1;
	}

	return r_lodCurveError->value / d;
}

/*
=============
RB_SurfaceGrid

Just copy the grid of points and triangulate
=============
*/
static void RB_SurfaceGrid( srfGridMesh_t *cv ) {
	int i, j;
	float   *xyz;
	float   *texCoords;
	float   *normal;
	unsigned char *color;
	drawVert_t  *dv;
	int rows, irows, vrows;
	int used;
	int widthTable[MAX_GRID_SIZE];
	int heightTable[MAX_GRID_SIZE];
	float lodError;
	int lodWidth, lodHeight;
	int numVertexes;
	int dlightBits;
	int     *vDlightBits;
	qboolean needsNormal;

	dlightBits = cv->dlightBits;
	tess.dlightBits |= dlightBits;

	lodError = LodErrorForVolume( cv->lodOrigin, cv->lodRadius );

	widthTable[0] = 0;
	lodWidth = 1;
	for ( i = 1 ; i < cv->width - 1 ; i++ ) {
		if ( cv->widthLodError[i] <= lodError ) {
			widthTable[lodWidth] = i;
			lodWidth++;
		}
	}
	widthTable[lodWidth] = cv->width - 1;
	lodWidth++;

	heightTable[0] = 0;
	lodHeight = 1;
	for ( i = 1 ; i < cv->height - 1 ; i++ ) {
		if ( cv->heightLodError[i] <= lodError ) {
			heightTable[lodHeight] = i;
			lodHeight++;
		}
	}
	heightTable[lodHeight] = cv->height - 1;
	lodHeight++;

	used = 0;
	needsNormal = tess.shader->needsNormal;
	int cvWidth = cv->width;
	drawVert_t *cvVerts = cv->verts;

	while ( used < lodHeight - 1 ) {
		do {
			vrows = ( SHADER_MAX_VERTEXES - tess.numVertexes ) / lodWidth;
			irows = ( SHADER_MAX_INDEXES - tess.numIndexes ) / ( lodWidth * 6 );

			if ( __builtin_expect( vrows < 2 || irows < 1, 0 ) ) {
				RB_EndSurface();
				RB_BeginSurface( tess.shader, tess.fogNum );
			} else {
				break;
			}
		} while ( 1 );

		rows = irows;
		if ( vrows < irows + 1 ) {
			rows = vrows - 1;
		}
		if ( used + rows > lodHeight ) {
			rows = lodHeight - used;
		}

		numVertexes = tess.numVertexes;

		xyz = tess.xyz[numVertexes];
		normal = tess.normal[numVertexes];
		texCoords = tess.texCoords[numVertexes][0];
		color = ( unsigned char * ) &tess.vertexColors[numVertexes];
		vDlightBits = &tess.vertexDlightBits[numVertexes];

		for ( i = 0 ; i < rows ; i++ ) {
			int hIndex = heightTable[ used + i ] * cvWidth;
			for ( j = 0 ; j < lodWidth ; j++ ) {
				dv = cvVerts + hIndex + widthTable[ j ];

				xyz[0] = dv->xyz[0];
				xyz[1] = dv->xyz[1];
				xyz[2] = dv->xyz[2];
				texCoords[0] = dv->st[0];
				texCoords[1] = dv->st[1];
				texCoords[2] = dv->lightmap[0];
				texCoords[3] = dv->lightmap[1];
				if ( needsNormal ) {
					normal[0] = dv->normal[0];
					normal[1] = dv->normal[1];
					normal[2] = dv->normal[2];
				}
				*( unsigned int * ) color = *( unsigned int * ) dv->color;
				*vDlightBits++ = dlightBits;
				xyz += 4;
				normal += 4;
				texCoords += 4;
				color += 4;
			}
		}

		// Optimized hardware triangle indexing block allocation
		{
			int numIndexes = tess.numIndexes;
			int w = lodWidth - 1;
			int h = rows - 1;
			glIndex_t *tessIdxPtr = &tess.indexes[numIndexes];

			for ( i = 0 ; i < h ; i++ ) {
				int rowOffset1 = numVertexes + i * lodWidth;
				int rowOffset2 = rowOffset1 + lodWidth;
				for ( j = 0 ; j < w ; j++ ) {
					int v1 = rowOffset1 + j + 1;
					int v2 = rowOffset1 + j;
					int v3 = rowOffset2 + j;
					int v4 = v3 + 1;

					tessIdxPtr[0] = v2;
					tessIdxPtr[1] = v3;
					tessIdxPtr[2] = v1;
					tessIdxPtr[3] = v1;
					tessIdxPtr[4] = v3;
					tessIdxPtr[5] = v4;
					tessIdxPtr += 6;
				}
			}
			tess.numIndexes = numIndexes + (h * w * 6);
		}

		tess.numVertexes += rows * lodWidth;
		used += rows - 1;
	}
}


/*
===========================================================================

NULL MODEL

===========================================================================
*/

/*
===================
RB_SurfaceAxis

Draws x/y/z lines from the origin for orientation debugging
===================
*/
static void RB_SurfaceAxis( void ) {
	GL_Bind( tr.whiteImage );
	GL_State( GLS_DEFAULT );
	qglLineWidth( 3 );
#ifdef USE_OPENGLES
	GLfloat col[] = {
	  1,0,0, 1,
	  1,0,0, 1,
	  0,1,0, 1,
	  0,1,0, 1,
	  0,0,1, 1,
	  0,0,1, 1
	 };
	 GLfloat vtx[] = {
	  0,0,0,
	  16,0,0,
	  0,0,0,
	  0,16,0,
	  0,0,0,
	  0,0,16
	 };
	GLboolean text = qglIsEnabled(GL_TEXTURE_COORD_ARRAY);
	GLboolean glcol = qglIsEnabled(GL_COLOR_ARRAY);
	if (text)
		qglDisableClientState( GL_TEXTURE_COORD_ARRAY );
	if (!glcol)
		qglEnableClientState( GL_COLOR_ARRAY);
	qglColorPointer( 4, GL_UNSIGNED_BYTE, 0, col );
	qglVertexPointer (3, GL_FLOAT, 0, vtx);
	qglDrawArrays(GL_LINES, 0, 6);
	if (text)
		qglEnableClientState( GL_TEXTURE_COORD_ARRAY );
	if (!glcol)
		qglDisableClientState( GL_COLOR_ARRAY);
#else
	qglBegin( GL_LINES );
	qglColor3f( 1,0,0 );
	qglVertex3f( 0,0,0 );
	qglVertex3f( 16,0,0 );
	qglColor3f( 0,1,0 );
	qglVertex3f( 0,0,0 );
	qglVertex3f( 0,16,0 );
	qglColor3f( 0,0,1 );
	qglVertex3f( 0,0,0 );
	qglVertex3f( 0,0,16 );
	qglEnd();
#endif
	qglLineWidth( 1 );
}

//===========================================================================

/*
====================
RB_SurfaceEntity

Entities that have a single procedurally generated surface
====================
*/
static void RB_SurfaceEntity( surfaceType_t *surfType ) {
	switch ( backEnd.currentEntity->e.reType ) {
	case RT_SPLASH:
		RB_SurfaceSplash();
		break;
	case RT_SPRITE:
		RB_SurfaceSprite();
		break;
	case RT_BEAM:
		RB_SurfaceBeam();
		break;
	case RT_RAIL_CORE:
		RB_SurfaceRailCore();
		break;
	case RT_RAIL_RINGS:
		RB_SurfaceRailRings();
		break;
	case RT_LIGHTNING:
		RB_SurfaceLightningBolt();
		break;
	default:
		RB_SurfaceAxis();
		break;
	}
}

static void RB_SurfaceBad( surfaceType_t *surfType ) {
	ri.Printf( PRINT_ALL, "Bad surface tesselated.\n" );
}

static void RB_SurfaceFlare( srfFlare_t *surf ) {
	if (r_flares->integer)
		RB_AddFlare(surf, tess.fogNum, surf->origin, surf->color, 1.0f, surf->normal, 0, qtrue);
}

static void RB_SurfaceSkip( void *surf ) {
}


void( *rb_surfaceTable[SF_NUM_SURFACE_TYPES] ) ( void * ) = {
	( void( * ) ( void* ) )RB_SurfaceBad,          // SF_BAD,
	( void( * ) ( void* ) )RB_SurfaceSkip,         // SF_SKIP,
	( void( * ) ( void* ) )RB_SurfaceFace,         // SF_FACE,
	( void( * ) ( void* ) )RB_SurfaceGrid,         // SF_GRID,
	( void( * ) ( void* ) )RB_SurfaceTriangles,    // SF_TRIANGLES,
	( void( * ) ( void* ) )RB_SurfacePolychain,    // SF_POLY,
	( void( * ) ( void* ) )RB_SurfaceMesh,         // SF_MD3,
	( void( * ) ( void* ) )RB_SurfaceCMesh,        // SF_MDC,
	( void( * ) ( void* ) )RB_SurfaceAnim,         // SF_MDS,
	( void( * ) ( void* ) )RB_MDRSurfaceAnim,      // SF_MDR,
	( void( * ) ( void* ) )RB_IQMSurfaceAnim,      // SF_IQM,
	( void( * ) ( void* ) )RB_SurfaceFlare,        // SF_FLARE,
	( void( * ) ( void* ) )RB_SurfaceEntity,       // SF_ENTITY
	( void( * ) ( void* ) )RB_SurfaceFoliage,      // SF_FOLIAGE,
};
