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

// Copyright (C) 1999-2000 Id Software, Inc.
//
// q_math.c -- stateless support routines that are included in each code module

// Some of the vector functions are static inline in q_shared.h. q3asm
// doesn't understand static functions though, so we only want them in
// one file. That's what this is about.
// Only include SIMD headers if we are NOT compiling for the Quake VM
#ifndef Q3_VM
#define Q_HAS_SIMD 1
#include <smmintrin.h> // Intel SSE4.1 intrinsics
#include <immintrin.h> // Required for FMA and advanced intrinsics
#endif

#include "q_shared.h"

vec3_t vec3_origin = {0,0,0};
vec3_t axisDefault[3] = { { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 } };


vec4_t colorBlack  = {0, 0, 0, 1};
vec4_t colorRed    = {1, 0, 0, 1};
vec4_t colorGreen  = {0, 1, 0, 1};
vec4_t colorBlue   = {0, 0, 1, 1};
vec4_t colorYellow = {1, 1, 0, 1};
vec4_t colorMagenta = {1, 0, 1, 1};
vec4_t colorCyan   = {0, 1, 1, 1};
vec4_t colorWhite  = {1, 1, 1, 1};
vec4_t colorLtGrey = {0.75, 0.75, 0.75, 1};
vec4_t colorMdGrey = {0.5, 0.5, 0.5, 1};
vec4_t colorDkGrey = {0.25, 0.25, 0.25, 1};

vec4_t g_color_table[8] =
{
	{0.0, 0.0, 0.0, 1.0},
	{1.0, 0.0, 0.0, 1.0},
	{0.0, 1.0, 0.0, 1.0},
	{1.0, 1.0, 0.0, 1.0},
	{0.0, 0.0, 1.0, 1.0},
	{0.0, 1.0, 1.0, 1.0},
	{1.0, 0.0, 1.0, 1.0},
	{1.0, 1.0, 1.0, 1.0},
};


vec3_t bytedirs[NUMVERTEXNORMALS] =
{
	{-0.525731, 0.000000, 0.850651}, {-0.442863, 0.238856, 0.864188},
	{-0.295242, 0.000000, 0.955423}, {-0.309017, 0.500000, 0.809017},
	{-0.162460, 0.262866, 0.951056}, {0.000000, 0.000000, 1.000000},
	{0.000000, 0.850651, 0.525731}, {-0.147621, 0.716567, 0.681718},
	{0.147621, 0.716567, 0.681718}, {0.000000, 0.525731, 0.850651},
	{0.309017, 0.500000, 0.809017}, {0.525731, 0.000000, 0.850651},
	{0.295242, 0.000000, 0.955423}, {0.442863, 0.238856, 0.864188},
	{0.162460, 0.262866, 0.951056}, {-0.681718, 0.147621, 0.716567},
	{-0.809017, 0.309017, 0.500000},{-0.587785, 0.425325, 0.688191},
	{-0.850651, 0.525731, 0.000000},{-0.864188, 0.442863, 0.238856},
	{-0.716567, 0.681718, 0.147621},{-0.688191, 0.587785, 0.425325},
	{-0.500000, 0.809017, 0.309017}, {-0.238856, 0.864188, 0.442863},
	{-0.425325, 0.688191, 0.587785}, {-0.716567, 0.681718, -0.147621},
	{-0.500000, 0.809017, -0.309017}, {-0.525731, 0.850651, 0.000000},
	{0.000000, 0.850651, -0.525731}, {-0.238856, 0.864188, -0.442863},
	{0.000000, 0.955423, -0.295242}, {-0.262866, 0.951056, -0.162460},
	{0.000000, 1.000000, 0.000000}, {0.000000, 0.955423, 0.295242},
	{-0.262866, 0.951056, 0.162460}, {0.238856, 0.864188, 0.442863},
	{0.262866, 0.951056, 0.162460}, {0.500000, 0.809017, 0.309017},
	{0.238856, 0.864188, -0.442863},{0.262866, 0.951056, -0.162460},
	{0.500000, 0.809017, -0.309017},{0.850651, 0.525731, 0.000000},
	{0.716567, 0.681718, 0.147621}, {0.716567, 0.681718, -0.147621},
	{0.525731, 0.850651, 0.000000}, {0.425325, 0.688191, 0.587785},
	{0.864188, 0.442863, 0.238856}, {0.688191, 0.587785, 0.425325},
	{0.809017, 0.309017, 0.500000}, {0.681718, 0.147621, 0.716567},
	{0.587785, 0.425325, 0.688191}, {0.955423, 0.295242, 0.000000},
	{1.000000, 0.000000, 0.000000}, {0.951056, 0.162460, 0.262866},
	{0.850651, -0.525731, 0.000000},{0.955423, -0.295242, 0.000000},
	{0.864188, -0.442863, 0.238856}, {0.951056, -0.162460, 0.262866},
	{0.809017, -0.309017, 0.500000}, {0.681718, -0.147621, 0.716567},
	{0.850651, 0.000000, 0.525731}, {0.864188, 0.442863, -0.238856},
	{0.809017, 0.309017, -0.500000}, {0.951056, 0.162460, -0.262866},
	{0.525731, 0.000000, -0.850651}, {0.681718, 0.147621, -0.716567},
	{0.681718, -0.147621, -0.716567},{0.850651, 0.000000, -0.525731},
	{0.809017, -0.309017, -0.500000}, {0.864188, -0.442863, -0.238856},
	{0.951056, -0.162460, -0.262866}, {0.147621, 0.716567, -0.681718},
	{0.309017, 0.500000, -0.809017}, {0.425325, 0.688191, -0.587785},
	{0.442863, 0.238856, -0.864188}, {0.587785, 0.425325, -0.688191},
	{0.688191, 0.587785, -0.425325}, {-0.147621, 0.716567, -0.681718},
	{-0.309017, 0.500000, -0.809017}, {0.000000, 0.525731, -0.850651},
	{-0.525731, 0.000000, -0.850651}, {-0.442863, 0.238856, -0.864188},
	{-0.295242, 0.000000, -0.955423}, {-0.162460, 0.262866, -0.951056},
	{0.000000, 0.000000, -1.000000}, {0.295242, 0.000000, -0.955423},
	{0.162460, 0.262866, -0.951056}, {-0.442863, -0.238856, -0.864188},
	{-0.309017, -0.500000, -0.809017}, {-0.162460, -0.262866, -0.951056},
	{0.000000, -0.850651, -0.525731}, {-0.147621, -0.716567, -0.681718},
	{0.147621, -0.716567, -0.681718}, {0.000000, -0.525731, -0.850651},
	{0.309017, -0.500000, -0.809017}, {0.442863, -0.238856, -0.864188},
	{0.162460, -0.262866, -0.951056}, {0.238856, -0.864188, -0.442863},
	{0.500000, -0.809017, -0.309017}, {0.425325, -0.688191, -0.587785},
	{0.716567, -0.681718, -0.147621}, {0.688191, -0.587785, -0.425325},
	{0.587785, -0.425325, -0.688191}, {0.000000, -0.955423, -0.295242},
	{0.000000, -1.000000, 0.000000}, {0.262866, -0.951056, -0.162460},
	{0.000000, -0.850651, 0.525731}, {0.000000, -0.955423, 0.295242},
	{0.238856, -0.864188, 0.442863}, {0.262866, -0.951056, 0.162460},
	{0.500000, -0.809017, 0.309017}, {0.716567, -0.681718, 0.147621},
	{0.525731, -0.850651, 0.000000}, {-0.238856, -0.864188, -0.442863},
	{-0.500000, -0.809017, -0.309017}, {-0.262866, -0.951056, -0.162460},
	{-0.850651, -0.525731, 0.000000}, {-0.716567, -0.681718, -0.147621},
	{-0.716567, -0.681718, 0.147621}, {-0.525731, -0.850651, 0.000000},
	{-0.500000, -0.809017, 0.309017}, {-0.238856, -0.864188, 0.442863},
	{-0.262866, -0.951056, 0.162460}, {-0.864188, -0.442863, 0.238856},
	{-0.809017, -0.309017, 0.500000}, {-0.688191, -0.587785, 0.425325},
	{-0.681718, -0.147621, 0.716567}, {-0.442863, -0.238856, 0.864188},
	{-0.587785, -0.425325, 0.688191}, {-0.309017, -0.500000, 0.809017},
	{-0.147621, -0.716567, 0.681718}, {-0.425325, -0.688191, 0.587785},
	{-0.162460, -0.262866, 0.951056}, {0.442863, -0.238856, 0.864188},
	{0.162460, -0.262866, 0.951056}, {0.309017, -0.500000, 0.809017},
	{0.147621, -0.716567, 0.681718}, {0.000000, -0.525731, 0.850651},
	{0.425325, -0.688191, 0.587785}, {0.587785, -0.425325, 0.688191},
	{0.688191, -0.587785, 0.425325}, {-0.955423, 0.295242, 0.000000},
	{-0.951056, 0.162460, 0.262866}, {-1.000000, 0.000000, 0.000000},
	{-0.850651, 0.000000, 0.525731}, {-0.955423, -0.295242, 0.000000},
	{-0.951056, -0.162460, 0.262866}, {-0.864188, 0.442863, -0.238856},
	{-0.951056, 0.162460, -0.262866}, {-0.809017, 0.309017, -0.500000},
	{-0.864188, -0.442863, -0.238856}, {-0.951056, -0.162460, -0.262866},
	{-0.809017, -0.309017, -0.500000}, {-0.681718, 0.147621, -0.716567},
	{-0.681718, -0.147621, -0.716567}, {-0.850651, 0.000000, -0.525731},
	{-0.688191, 0.587785, -0.425325}, {-0.587785, 0.425325, -0.688191},
	{-0.425325, 0.688191, -0.587785}, {-0.425325, -0.688191, -0.587785},
	{-0.587785, -0.425325, -0.688191}, {-0.688191, -0.587785, -0.425325}
};

//==============================================================

int     Q_rand( int *seed ) {
	*seed = ( 69069U * *seed + 1U );
	return *seed;
}

float   Q_random( int *seed ) {
	return ( Q_rand( seed ) & 0xffff ) / (float)0x10000;
}

float   Q_crandom( int *seed ) {
	return 2.0 * ( Q_random( seed ) - 0.5 );
}


//=======================================================

signed char ClampChar( int i ) {
	if ( i < -128 ) {
		return -128;
	}
	if ( i > 127 ) {
		return 127;
	}
	return i;
}

signed short ClampShort( int i ) {
	if ( i < -32768 ) {
		return -32768;
	}
	if ( i > 0x7fff ) {
		return 0x7fff;
	}
	return i;
}


// this isn't a real cheap function to call!

int DirToByte(vec3_t dir)
{
    int i, best;
    float bestd;

    if (!dir)
    {
        return 0;
    }

    bestd = 0.0f;
    best = 0;

    // 1. Broadcast 'dir' components into a SIMD register: [0, dir[2], dir[1], dir[0]]
    __m128 v_dir = _mm_set_ps(0.0f, dir[2], dir[1], dir[0]);

    // Track the best distances in a SIMD register initialized to 0
    __m128 v_bestd = _mm_setzero_ps(); 

    // 2. Process 4 vectors at a time. 
    // Note: 162 isn't perfectly divisible by 4 (162 / 4 = 40.5).
    // We loop up to 160 safely using SIMD, then handle the last 2 scalars at the end.
    for (i = 0; i < 160; i += 4)
    {
        // Load 4 consecutive vec3s from the bytedirs table
        __m128 b0 = _mm_set_ps(0.0f, bytedirs[i+0][2], bytedirs[i+0][1], bytedirs[i+0][0]);
        __m128 b1 = _mm_set_ps(0.0f, bytedirs[i+1][2], bytedirs[i+1][1], bytedirs[i+1][0]);
        __m128 b2 = _mm_set_ps(0.0f, bytedirs[i+2][2], bytedirs[i+2][1], bytedirs[i+2][0]);
        __m128 b3 = _mm_set_ps(0.0f, bytedirs[i+3][2], bytedirs[i+3][1], bytedirs[i+3][0]);

        // Calculate 4 dot products instantly
        // 0x71 mask: multiply X, Y, Z components and store the result in the lowest slot (index 0)
        __m128 dot0 = _mm_dp_ps(v_dir, b0, 0x71);
        __m128 dot1 = _mm_dp_ps(v_dir, b1, 0x71);
        __m128 dot2 = _mm_dp_ps(v_dir, b2, 0x71);
        __m128 dot3 = _mm_dp_ps(v_dir, b3, 0x71);

        // Pack the 4 dot products into a single SIMD register: [dot3, dot2, dot1, dot0]
        __m128 dots = _mm_unpacklo_ps(_mm_unpacklo_ps(dot0, dot2), _mm_unpacklo_ps(dot1, dot3));

        // Scalar extraction out of the packed register to update the best match
        // (Keeping the conditional updates scalar prevents complex mask generation errors)
        float d0 = _mm_cvtss_f32(dots);
        float d1 = _mm_cvtss_f32(_mm_shuffle_ps(dots, dots, _MM_SHUFFLE(1, 1, 1, 1)));
        float d2 = _mm_cvtss_f32(_mm_shuffle_ps(dots, dots, _MM_SHUFFLE(2, 2, 2, 2)));
        float d3 = _mm_cvtss_f32(_mm_shuffle_ps(dots, dots, _MM_SHUFFLE(3, 3, 3, 3)));

        if (d0 > bestd) { bestd = d0; best = i + 0; }
        if (d1 > bestd) { bestd = d1; best = i + 1; }
        if (d2 > bestd) { bestd = d2; best = i + 2; }
        if (d3 > bestd) { bestd = d3; best = i + 3; }
    }

    // 3. Clean up loop remainder (elements 160 and 161)
    for (; i < NUMVERTEXNORMALS; i++)
    {
        float d = dir[0]*bytedirs[i][0] + dir[1]*bytedirs[i][1] + dir[2]*bytedirs[i][2];
        if (d > bestd)
        {
            bestd = d;
            best = i;
        }
    }

    return best;
}

void ByteToDir( int b, vec3_t dir ) {
	if ( b < 0 || b >= NUMVERTEXNORMALS ) {
		VectorCopy( vec3_origin, dir );
		return;
	}
	VectorCopy( bytedirs[b], dir );
}


unsigned ColorBytes3( float r, float g, float b ) {
	unsigned i;

	( (byte *)&i )[0] = r * 255;
	( (byte *)&i )[1] = g * 255;
	( (byte *)&i )[2] = b * 255;

	return i;
}

unsigned ColorBytes4( float r, float g, float b, float a ) {
	unsigned i;

	( (byte *)&i )[0] = r * 255;
	( (byte *)&i )[1] = g * 255;
	( (byte *)&i )[2] = b * 255;
	( (byte *)&i )[3] = a * 255;

	return i;
}

float NormalizeColor( const vec3_t in, vec3_t out ) {
	float max;

	max = in[0];
	if ( in[1] > max ) {
		max = in[1];
	}
	if ( in[2] > max ) {
		max = in[2];
	}

	if ( !max ) {
		VectorClear( out );
	} else {
		out[0] = in[0] / max;
		out[1] = in[1] / max;
		out[2] = in[2] / max;
	}
	return max;
}


/*
=====================
PlaneFromPoints

Returns false if the triangle is degenrate.
The normal will point out of the clock for clockwise ordered points
=====================
*/
qboolean PlaneFromPoints( vec4_t plane, const vec3_t a, const vec3_t b, const vec3_t c ) {
	vec3_t d1, d2;

	VectorSubtract( b, a, d1 );
	VectorSubtract( c, a, d2 );
	CrossProduct( d2, d1, plane );
	if ( VectorNormalize( plane ) == 0 ) {
		return qfalse;
	}

	plane[3] = DotProduct( a, plane );
	return qtrue;
}

/*
===============
RotatePointAroundVector

This is not implemented very well...
===============
*/
void RotatePointAroundVector( vec3_t dst, const vec3_t dir, const vec3_t point,
							  float degrees ) {
	float m[3][3];
	float im[3][3];
	float zrot[3][3];
	float tmpmat[3][3];
	float rot[3][3];
	int i;
	vec3_t vr, vup, vf;
	float rad;

	vf[0] = dir[0];
	vf[1] = dir[1];
	vf[2] = dir[2];

	PerpendicularVector( vr, dir );
	CrossProduct( vr, vf, vup );

	m[0][0] = vr[0];
	m[1][0] = vr[1];
	m[2][0] = vr[2];

	m[0][1] = vup[0];
	m[1][1] = vup[1];
	m[2][1] = vup[2];

	m[0][2] = vf[0];
	m[1][2] = vf[1];
	m[2][2] = vf[2];

	memcpy( im, m, sizeof( im ) );

	im[0][1] = m[1][0];
	im[0][2] = m[2][0];
	im[1][0] = m[0][1];
	im[1][2] = m[2][1];
	im[2][0] = m[0][2];
	im[2][1] = m[1][2];

	memset( zrot, 0, sizeof( zrot ) );
	zrot[0][0] = zrot[1][1] = zrot[2][2] = 1.0F;

	rad = DEG2RAD( degrees );
	zrot[0][0] = cos( rad );
	zrot[0][1] = sin( rad );
	zrot[1][0] = -sin( rad );
	zrot[1][1] = cos( rad );

	MatrixMultiply( m, zrot, tmpmat );
	MatrixMultiply( tmpmat, im, rot );

	for ( i = 0; i < 3; i++ ) {
		dst[i] = rot[i][0] * point[0] + rot[i][1] * point[1] + rot[i][2] * point[2];
	}
}

/*
===============
RotateAroundDirection
===============
*/
void RotateAroundDirection( vec3_t axis[3], float yaw ) {

	// create an arbitrary axis[1]
	PerpendicularVector( axis[1], axis[0] );

	// rotate it around axis[0] by yaw
	if ( yaw ) {
		vec3_t temp;

		VectorCopy( axis[1], temp );
		RotatePointAroundVector( axis[1], axis[0], temp, yaw );
	}

	// cross to get axis[2]
	CrossProduct( axis[0], axis[1], axis[2] );
}



void vectoangles( const vec3_t value1, vec3_t angles ) {
	float forward;
	float yaw, pitch;

	if ( value1[1] == 0 && value1[0] == 0 ) {
		yaw = 0;
		if ( value1[2] > 0 ) {
			pitch = 90;
		} else {
			pitch = 270;
		}
	} else {
		if ( value1[0] ) {
			yaw = ( atan2( value1[1], value1[0] ) * 180 / M_PI );
		} else if ( value1[1] > 0 )   {
			yaw = 90;
		} else {
			yaw = 270;
		}
		if ( yaw < 0 ) {
			yaw += 360;
		}

		forward = sqrt( value1[0] * value1[0] + value1[1] * value1[1] );
		pitch = ( atan2( value1[2], forward ) * 180 / M_PI );
		if ( pitch < 0 ) {
			pitch += 360;
		}
	}

	angles[PITCH] = -pitch;
	angles[YAW] = yaw;
	angles[ROLL] = 0;
}


/*
=================
AnglesToAxis
=================
*/
void AnglesToAxis( const vec3_t angles, vec3_t axis[3] ) {
	vec3_t right;

	// angle vectors returns "right" instead of "y axis"
	AngleVectors( angles, axis[0], right, axis[2] );
	VectorSubtract( vec3_origin, right, axis[1] );
}

void AxisClear( vec3_t axis[3] ) {
	axis[0][0] = 1;
	axis[0][1] = 0;
	axis[0][2] = 0;
	axis[1][0] = 0;
	axis[1][1] = 1;
	axis[1][2] = 0;
	axis[2][0] = 0;
	axis[2][1] = 0;
	axis[2][2] = 1;
}

void AxisCopy( vec3_t in[3], vec3_t out[3] ) {
	VectorCopy( in[0], out[0] );
	VectorCopy( in[1], out[1] );
	VectorCopy( in[2], out[2] );
}

void ProjectPointOnPlane( vec3_t dst, const vec3_t p, const vec3_t normal ) {
	float d;
	vec3_t n;
	float inv_denom;

	inv_denom = 1.0F / DotProduct( normal, normal );

	d = DotProduct( normal, p ) * inv_denom;

	n[0] = normal[0] * inv_denom;
	n[1] = normal[1] * inv_denom;
	n[2] = normal[2] * inv_denom;

	dst[0] = p[0] - d * n[0];
	dst[1] = p[1] - d * n[1];
	dst[2] = p[2] - d * n[2];
}

/*
================
MakeNormalVectors

Given a normalized forward vector, create two
other perpendicular vectors
================
*/
void MakeNormalVectors( const vec3_t forward, vec3_t right, vec3_t up ) {
	float d;

	// this rotate and negate guarantees a vector
	// not colinear with the original
	right[1] = -forward[0];
	right[2] = forward[1];
	right[0] = forward[2];

	d = DotProduct( right, forward );
	VectorMA( right, -d, forward, right );
	VectorNormalize( right );
	CrossProduct( right, forward, up );
}


void VectorRotate(vec3_t in, vec3_t matrix[3], vec3_t out)
{
#ifndef Q3_VM
 
    __m128 v_in = _mm_set_ps(0.0f, in[2], in[1], in[0]);

    __m128 row0 = _mm_set_ps(0.0f, matrix[0][2], matrix[0][1], matrix[0][0]);
    __m128 row1 = _mm_set_ps(0.0f, matrix[1][2], matrix[1][1], matrix[1][0]);
    __m128 row2 = _mm_set_ps(0.0f, matrix[2][2], matrix[2][1], matrix[2][0]);

    __m128 dot0 = _mm_dp_ps(v_in, row0, 0x71);
    __m128 dot1 = _mm_dp_ps(v_in, row1, 0x71);
    __m128 dot2 = _mm_dp_ps(v_in, row2, 0x71);

    _mm_store_ss(&out[0], dot0);
    _mm_store_ss(&out[1], dot1);
    _mm_store_ss(&out[2], dot2);
#else
    // QVM FALLBACK (Simple scalar math q3asm understands)
    out[0] = in[0] * matrix[0][0] + in[1] * matrix[0][1] + in[2] * matrix[0][2];
    out[1] = in[0] * matrix[1][0] + in[1] * matrix[1][1] + in[2] * matrix[1][2];
    out[2] = in[0] * matrix[2][0] + in[1] * matrix[2][1] + in[2] * matrix[2][2];
#endif
}

//============================================================================

#if !idppc
/*
** float q_rsqrt( float number )
*/
float Q_rsqrt(float number)
{
#ifndef Q3_VM
    // 1. Load the single scalar float into a SIMD register
    __m128 reg = _mm_set_ss(number);
    
    // 2. Compute the reciprocal square root using specialized hardware silicon.
    // This gives a highly accurate initial approximation instantly.
    reg = _mm_rsqrt_ss(reg);
    
    float y;
    _mm_store_ss(&y, reg);
    
    // 3. One iteration of Newton-Raphson refinement to match the precision 
    // of the original Quake III implementation perfectly.
    return y * (1.5f - (number * 0.5f * y * y));
#else
    // Original legendary scalar bit-hack for QVM compilation compatibility
    floatint_t t;
    float x2, y;
    const float threehalfs = 1.5F;

    x2 = number * 0.5F;
    t.f = number;
    t.i = 0x5f3759df - (t.i >> 1); // what the fuck?
    y = t.f;
    y = y * (threehalfs - (x2 * y * y));

    return y;
#endif
}

float Q_fabs( float f ) {
	floatint_t fi;
	fi.f = f;
	fi.i &= 0x7FFFFFFF;
	return fi.f;
}
#endif

//============================================================

/*
===============
LerpAngle

===============
*/
float LerpAngle( float from, float to, float frac ) {
	float a;

	if ( to - from > 180 ) {
		to -= 360;
	}
	if ( to - from < -180 ) {
		to += 360;
	}
	a = from + frac * ( to - from );

	return a;
}


/*
=================
AngleSubtract

Always returns a value from -180 to 180
=================
*/
float   AngleSubtract( float a1, float a2 ) {
	float a;

	a = a1 - a2;
	while ( a > 180 ) {
		a -= 360;
	}
	while ( a < -180 ) {
		a += 360;
	}
	return a;
}


void AnglesSubtract( vec3_t v1, vec3_t v2, vec3_t v3 ) {
	v3[0] = AngleSubtract( v1[0], v2[0] );
	v3[1] = AngleSubtract( v1[1], v2[1] );
	v3[2] = AngleSubtract( v1[2], v2[2] );
}


float   AngleMod( float a ) {
	a = ( 360.0 / 65536 ) * ( (int)( a * ( 65536 / 360.0 ) ) & 65535 );
	return a;
}


/*
=================
AngleNormalize360

returns angle normalized to the range [0 <= angle < 360]
=================
*/
float AngleNormalize360( float angle ) {
	return ( 360.0 / 65536 ) * ( (int)( angle * ( 65536 / 360.0 ) ) & 65535 );
}


/*
=================
AngleNormalize180

returns angle normalized to the range [-180 < angle <= 180]
=================
*/
float AngleNormalize180( float angle ) {
	angle = AngleNormalize360( angle );
	if ( angle > 180.0 ) {
		angle -= 360.0;
	}
	return angle;
}


/*
=================
AngleDelta

returns the normalized delta from angle1 to angle2
=================
*/
float AngleDelta( float angle1, float angle2 ) {
	return AngleNormalize180( angle1 - angle2 );
}


//============================================================


/*
=================
SetPlaneSignbits
=================
*/
void SetPlaneSignbits( cplane_t *out ) {
	int bits, j;

	// for fast box on planeside test
	bits = 0;
	for ( j = 0 ; j < 3 ; j++ ) {
		if ( out->normal[j] < 0 ) {
			bits |= 1 << j;
		}
	}
	out->signbits = bits;
}


/*
==================
BoxOnPlaneSide

Returns 1, 2, or 1 + 2
==================
*/
int BoxOnPlaneSide(vec3_t emins, vec3_t emaxs, struct cplane_s *p)
{
	// fast axial cases
	if ( p->type < 3 ) {
		if ( p->dist <= emins[p->type] ) {
			return 1;
		}
		if ( p->dist >= emaxs[p->type] ) {
			return 2;
		}
		return 3;
	}

#if Q_HAS_SIMD
	if (p->signbits < 8)
	{
		__m128 v_emins = _mm_loadu_ps(emins);
		__m128 v_emaxs = _mm_loadu_ps(emaxs);
		__m128 v_normal = _mm_loadu_ps(p->normal);
		
		unsigned int m0 = (p->signbits & 1) ? 0xFFFFFFFF : 0;
		unsigned int m1 = (p->signbits & 2) ? 0xFFFFFFFF : 0;
		unsigned int m2 = (p->signbits & 4) ? 0xFFFFFFFF : 0;
		__m128 mask = _mm_set_ps(0.0f, *(float*)&m2, *(float*)&m1, *(float*)&m0);
		
		__m128 v_dist0 = _mm_or_ps(_mm_and_ps(mask, v_emins), _mm_andnot_ps(mask, v_emaxs));
		__m128 v_dist1 = _mm_or_ps(_mm_and_ps(mask, v_emaxs), _mm_andnot_ps(mask, v_emins));
		
		__m128 mul0 = _mm_mul_ps(v_normal, v_dist0);
		__m128 mul1 = _mm_mul_ps(v_normal, v_dist1);
		
		__m128 shuf0_1 = _mm_shuffle_ps(mul0, mul0, _MM_SHUFFLE(1, 1, 1, 1));
		__m128 shuf0_2 = _mm_shuffle_ps(mul0, mul0, _MM_SHUFFLE(2, 2, 2, 2));
		__m128 sum0 = _mm_add_ss(mul0, _mm_add_ss(shuf0_1, shuf0_2));
		
		__m128 shuf1_1 = _mm_shuffle_ps(mul1, mul1, _MM_SHUFFLE(1, 1, 1, 1));
		__m128 shuf1_2 = _mm_shuffle_ps(mul1, mul1, _MM_SHUFFLE(2, 2, 2, 2));
		__m128 sum1 = _mm_add_ss(mul1, _mm_add_ss(shuf1_1, shuf1_2));
		
		float dist0, dist1;
		_mm_store_ss(&dist0, sum0);
		_mm_store_ss(&dist1, sum1);
		
		int sides = 0;
		if (dist0 >= p->dist)
			sides = 1;
		if (dist1 < p->dist)
			sides |= 2;
			
		return sides;
	}
#endif

	// general case
	float	dist[2];
	int		sides, b, i;
	dist[0] = dist[1] = 0;
	if (p->signbits < 8) // >= 8: default case is original code (dist[0]=dist[1]=0)
	{
		for (i=0 ; i<3 ; i++)
		{
			b = (p->signbits >> i) & 1;
			dist[ b] += p->normal[i]*emaxs[i];
			dist[!b] += p->normal[i]*emins[i];
		}
	}

	sides = 0;
	if (dist[0] >= p->dist)
		sides = 1;
	if (dist[1] < p->dist)
		sides |= 2;

	return sides;
}

/*
=================
RadiusFromBounds
=================
*/
float RadiusFromBounds( const vec3_t mins, const vec3_t maxs ) {
	int i;
	vec3_t corner;
	float a, b;

	for ( i = 0 ; i < 3 ; i++ ) {
		a = fabs( mins[i] );
		b = fabs( maxs[i] );
		corner[i] = a > b ? a : b;
	}

	return VectorLength( corner );
}


void ClearBounds( vec3_t mins, vec3_t maxs ) {
	mins[0] = mins[1] = mins[2] = 99999;
	maxs[0] = maxs[1] = maxs[2] = -99999;
}

void AddPointToBounds( const vec3_t v, vec3_t mins, vec3_t maxs ) {
#if Q_HAS_SIMD
	__m128 pt = _mm_loadu_ps(v);
	__m128 mn = _mm_loadu_ps(mins);
	__m128 mx = _mm_loadu_ps(maxs);
	
	__m128 new_mn = _mm_min_ps(pt, mn);
	__m128 new_mx = _mm_max_ps(pt, mx);
	
	_mm_store_ss(&mins[0], new_mn);
	_mm_store_ss(&mins[1], _mm_shuffle_ps(new_mn, new_mn, _MM_SHUFFLE(1, 1, 1, 1)));
	_mm_store_ss(&mins[2], _mm_shuffle_ps(new_mn, new_mn, _MM_SHUFFLE(2, 2, 2, 2)));
	
	_mm_store_ss(&maxs[0], new_mx);
	_mm_store_ss(&maxs[1], _mm_shuffle_ps(new_mx, new_mx, _MM_SHUFFLE(1, 1, 1, 1)));
	_mm_store_ss(&maxs[2], _mm_shuffle_ps(new_mx, new_mx, _MM_SHUFFLE(2, 2, 2, 2)));
#else
	if ( v[0] < mins[0] ) {
		mins[0] = v[0];
	}
	if ( v[0] > maxs[0] ) {
		maxs[0] = v[0];
	}

	if ( v[1] < mins[1] ) {
		mins[1] = v[1];
	}
	if ( v[1] > maxs[1] ) {
		maxs[1] = v[1];
	}

	if ( v[2] < mins[2] ) {
		mins[2] = v[2];
	}
	if ( v[2] > maxs[2] ) {
		maxs[2] = v[2];
	}
#endif
}

qboolean BoundsIntersect(const vec3_t mins, const vec3_t maxs,
                         const vec3_t mins2, const vec3_t maxs2)
{
#ifndef Q3_VM
    // 1. Load the bounds into registers (padding 4th slot with 0)
    __m128 v_mins  = _mm_set_ps(0.0f, mins[2],  mins[1],  mins[0]);
    __m128 v_maxs  = _mm_set_ps(0.0f, maxs[2],  maxs[1],  maxs[0]);
    __m128 v_mins2 = _mm_set_ps(0.0f, mins2[2], mins2[1], mins2[0]);
    __m128 v_maxs2 = _mm_set_ps(0.0f, maxs2[2], maxs2[1], maxs2[0]);

    // 2. Perform simultaneous comparisons across X, Y, and Z axes
    // cmplt: returns 0xFFFFFFFF if true, 0x0 if false per component
    __m128 cmp1 = _mm_cmplt_ps(v_maxs, v_mins2);  // Is maxs < mins2?
    __m128 cmp2 = _mm_cmpgt_ps(v_mins, v_maxs2);  // Is mins > maxs2?

    // 3. Combine the comparison results using a bitwise OR
    __m128 combined = _mm_or_ps(cmp1, cmp2);

    // 4. Movemask extracts the most significant bit of each float slot 
    // into a standard integer (slots 0, 1, 2 correspond to bits 0, 1, 2)
    int mask = _mm_movemask_ps(combined);

    // 5. If any of the lower 3 bits (value 1, 2, or 4) are set, an exclusion condition met.
    // Masking with 7 (binary 0111) checks axes X, Y, and Z simultaneously.
    if (mask & 7)
    {
        return qfalse; 
    }

    return qtrue;
#else
    // Original scalar fallback for QVM
    if (maxs[0] < mins2[0] || maxs[1] < mins2[1] || maxs[2] < mins2[2] ||
        mins[0] > maxs2[0] || mins[1] > maxs2[1] || mins[2] > maxs2[2])
    {
        return qfalse;
    }
    return qtrue;
#endif
}

qboolean BoundsIntersectSphere(const vec3_t mins, const vec3_t maxs,
                               const vec3_t origin, vec_t radius)
{
#ifndef Q3_VM
    // 1. Load bounds and origin (padding 4th slot with 0)
    __m128 v_mins   = _mm_set_ps(0.0f, mins[2],   mins[1],   mins[0]);
    __m128 v_maxs   = _mm_set_ps(0.0f, maxs[2],   maxs[1],   maxs[0]);
    __m128 v_origin = _mm_set_ps(0.0f, origin[2], origin[1], origin[0]);
    
    // 2. Broadcast the radius across all slots
    __m128 v_radius = _mm_set1_ps(radius);

    // 3. Expand the sphere origin out into a min/max bounding box
    __m128 sphere_mins = _mm_sub_ps(v_origin, v_radius);
    __m128 sphere_maxs = _mm_add_ps(v_origin, v_radius);

    // 4. Simultaneous check: Is sphere completely outside the AABB?
    __m128 cmp1 = _mm_cmpgt_ps(sphere_mins, v_maxs); // origin - radius > maxs
    __m128 cmp2 = _mm_cmplt_ps(sphere_maxs, v_mins); // origin + radius < mins

    // 5. Extract bitmask of the results
    int mask = _mm_movemask_ps(_mm_or_ps(cmp1, cmp2));

    // If any X, Y, or Z bits (lower 3 bits) are set, there is no intersection
    if (mask & 7)
    {
        return qfalse;
    }

    return qtrue;
#else
    if (origin[0] - radius > maxs[0] ||
        origin[0] + radius < mins[0] ||
        origin[1] - radius > maxs[1] ||
        origin[1] + radius < mins[1] ||
        origin[2] - radius > maxs[2] ||
        origin[2] + radius < mins[2])
    {
        return qfalse;
    }
    return qtrue;
#endif
}

qboolean BoundsIntersectPoint(const vec3_t mins, const vec3_t maxs,
                              const vec3_t origin)
{
#ifndef Q3_VM
    __m128 v_mins   = _mm_set_ps(0.0f, mins[2],   mins[1],   mins[0]);
    __m128 v_maxs   = _mm_set_ps(0.0f, maxs[2],   maxs[1],   maxs[0]);
    __m128 v_origin = _mm_set_ps(0.0f, origin[2], origin[1], origin[0]);

    // Check if the origin point escapes the bounds on any axis
    __m128 cmp1 = _mm_cmpgt_ps(v_origin, v_maxs); // origin > maxs
    __m128 cmp2 = _mm_cmplt_ps(v_origin, v_mins); // origin < mins

    int mask = _mm_movemask_ps(_mm_or_ps(cmp1, cmp2));

    if (mask & 7)
    {
        return qfalse;
    }

    return qtrue;
#else
    if (origin[0] > maxs[0] ||
        origin[0] < mins[0] ||
        origin[1] > maxs[1] ||
        origin[1] < mins[1] ||
        origin[2] > maxs[2] ||
        origin[2] < mins[2])
    {
        return qfalse;
    }
    return qtrue;
#endif
}

vec_t VectorNormalize(vec3_t v)
{
#ifndef Q3_VM
    // 1. Load the 3 floats (safely padding the 4th element with 0.0f)
    __m128 x = _mm_set_ps(0.0f, v[2], v[1], v[0]);

    // 2. Compute dot product of the vector with itself (X^2 + Y^2 + Z^2)
    // 0x77 mask: Multiply slots 0,1,2 and broadcast the sum to ALL slots of 'sum'
    __m128 sum = _mm_dp_ps(x, x, 0x77);

    float length;
    _mm_store_ss(&length, sum);

    if (length > 0.0f) 
    {
        // 3. Take the square root of the sum
        __m128 sqrt_len = _mm_sqrt_ps(sum);
        
        // Extract the actual float length to return later
        _mm_store_ss(&length, sqrt_len);

        // 4. Divide the original vector components by the calculated length
        __m128 norm = _mm_div_ps(x, sqrt_len);

        // 5. Store back to memory
        _mm_store_ss(&v[0], norm);
        _mm_store_ss(&v[1], _mm_shuffle_ps(norm, norm, _MM_SHUFFLE(1, 1, 1, 1)));
        _mm_store_ss(&v[2], _mm_shuffle_ps(norm, norm, _MM_SHUFFLE(2, 2, 2, 2)));
    }
    return length;
#else
    // Original scalar fallback for QVM compiler
    float length, ilength;
    length = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
    if (length)
    {
        ilength = 1 / (float)sqrt(length);
        length *= ilength;
        v[0] *= ilength;
        v[1] *= ilength;
        v[2] *= ilength;
    }
    return length;
#endif
}

vec_t VectorNormalize2( const vec3_t v, vec3_t out ) {
	float length, ilength;

	length = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];

	if ( length ) {
		/* writing it this way allows gcc to recognize that rsqrt can be used */
		ilength = 1/(float)sqrt (length);
		/* sqrt(length) = length * (1 / sqrt(length)) */
		length *= ilength;
		out[0] = v[0] * ilength;
		out[1] = v[1] * ilength;
		out[2] = v[2] * ilength;
	} else {
		VectorClear( out );
	}

	return length;

}

void _VectorMA(const vec3_t veca, float scale, const vec3_t vecb, vec3_t vecc)
{
#ifndef Q3_VM
    // 1. Load veca and vecb (padding 4th element with 0.0f)
    __m128 a = _mm_set_ps(0.0f, veca[2], veca[1], veca[0]);
    __m128 b = _mm_set_ps(0.0f, vecb[2], vecb[1], vecb[0]);
    
    // 2. Broadcast the single scalar scale float across all 4 slots
    __m128 s = _mm_set1_ps(scale);
    
    // 3. Fused Multiply-Add: (b * s) + a
    __m128 res = _mm_fmadd_ps(b, s, a);
    
    // 4. Stream back to the vecc destination array safely
    _mm_store_ss(&vecc[0], res);
    _mm_store_ss(&vecc[1], _mm_shuffle_ps(res, res, _MM_SHUFFLE(1, 1, 1, 1)));
    _mm_store_ss(&vecc[2], _mm_shuffle_ps(res, res, _MM_SHUFFLE(2, 2, 2, 2)));
#else
    vecc[0] = veca[0] + scale * vecb[0];
    vecc[1] = veca[1] + scale * vecb[1];
    vecc[2] = veca[2] + scale * vecb[2];
#endif
}


vec_t _DotProduct( const vec3_t v1, const vec3_t v2 ) {
	return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}

void _VectorSubtract(const vec3_t veca, const vec3_t vecb, vec3_t out)
{
#ifndef Q3_VM
    __m128 a = _mm_set_ps(0.0f, veca[2], veca[1], veca[0]);
    __m128 b = _mm_set_ps(0.0f, vecb[2], vecb[1], vecb[0]);
    
    __m128 res = _mm_sub_ps(a, b);
    
    _mm_store_ss(&out[0], res);
    _mm_store_ss(&out[1], _mm_shuffle_ps(res, res, _MM_SHUFFLE(1, 1, 1, 1)));
    _mm_store_ss(&out[2], _mm_shuffle_ps(res, res, _MM_SHUFFLE(2, 2, 2, 2)));
#else
    out[0] = veca[0] - vecb[0];
    out[1] = veca[1] - vecb[1];
    out[2] = veca[2] - vecb[2];
#endif
}

void _VectorAdd(const vec3_t veca, const vec3_t vecb, vec3_t out)
{
#ifndef Q3_VM
    __m128 a = _mm_set_ps(0.0f, veca[2], veca[1], veca[0]);
    __m128 b = _mm_set_ps(0.0f, vecb[2], vecb[1], vecb[0]);
    
    __m128 res = _mm_add_ps(a, b);
    
    _mm_store_ss(&out[0], res);
    _mm_store_ss(&out[1], _mm_shuffle_ps(res, res, _MM_SHUFFLE(1, 1, 1, 1)));
    _mm_store_ss(&out[2], _mm_shuffle_ps(res, res, _MM_SHUFFLE(2, 2, 2, 2)));
#else
    out[0] = veca[0] + vecb[0];
    out[1] = veca[1] + vecb[1];
    out[2] = veca[2] + vecb[2];
#endif
}

void _VectorCopy( const vec3_t in, vec3_t out ) {
	out[0] = in[0];
	out[1] = in[1];
	out[2] = in[2];
}

void _VectorScale( const vec3_t in, vec_t scale, vec3_t out ) {
	out[0] = in[0] * scale;
	out[1] = in[1] * scale;
	out[2] = in[2] * scale;
}

void Vector4Scale( const vec4_t in, vec_t scale, vec4_t out ) {
	out[0] = in[0] * scale;
	out[1] = in[1] * scale;
	out[2] = in[2] * scale;
	out[3] = in[3] * scale;
}


int Q_log2( int val ) {
	int answer;

	answer = 0;
	while ( ( val >>= 1 ) != 0 ) {
		answer++;
	}
	return answer;
}



/*
=================
PlaneTypeForNormal
=================
*/
/*
int	PlaneTypeForNormal (vec3_t normal) {
	if ( normal[0] == 1.0 )
		return PLANE_X;
	if ( normal[1] == 1.0 )
		return PLANE_Y;
	if ( normal[2] == 1.0 )
		return PLANE_Z;

	return PLANE_NON_AXIAL;
}
*/


/*
================
MatrixMultiply
================
*/
void MatrixMultiply( float in1[3][3], float in2[3][3], float out[3][3] ) {
	out[0][0] = in1[0][0] * in2[0][0] + in1[0][1] * in2[1][0] +
				in1[0][2] * in2[2][0];
	out[0][1] = in1[0][0] * in2[0][1] + in1[0][1] * in2[1][1] +
				in1[0][2] * in2[2][1];
	out[0][2] = in1[0][0] * in2[0][2] + in1[0][1] * in2[1][2] +
				in1[0][2] * in2[2][2];
	out[1][0] = in1[1][0] * in2[0][0] + in1[1][1] * in2[1][0] +
				in1[1][2] * in2[2][0];
	out[1][1] = in1[1][0] * in2[0][1] + in1[1][1] * in2[1][1] +
				in1[1][2] * in2[2][1];
	out[1][2] = in1[1][0] * in2[0][2] + in1[1][1] * in2[1][2] +
				in1[1][2] * in2[2][2];
	out[2][0] = in1[2][0] * in2[0][0] + in1[2][1] * in2[1][0] +
				in1[2][2] * in2[2][0];
	out[2][1] = in1[2][0] * in2[0][1] + in1[2][1] * in2[1][1] +
				in1[2][2] * in2[2][1];
	out[2][2] = in1[2][0] * in2[0][2] + in1[2][1] * in2[1][2] +
				in1[2][2] * in2[2][2];
}


void AngleVectors( const vec3_t angles, vec3_t forward, vec3_t right, vec3_t up ) {
	float angle;
	static float sr, sp, sy, cr, cp, cy;
	// static to help MS compiler fp bugs

	angle = angles[YAW] * ( M_PI * 2 / 360 );
	sy = sin( angle );
	cy = cos( angle );
	angle = angles[PITCH] * ( M_PI * 2 / 360 );
	sp = sin( angle );
	cp = cos( angle );
	angle = angles[ROLL] * ( M_PI * 2 / 360 );
	sr = sin( angle );
	cr = cos( angle );

	if ( forward ) {
		forward[0] = cp * cy;
		forward[1] = cp * sy;
		forward[2] = -sp;
	}
	if ( right ) {
		right[0] = ( -1 * sr * sp * cy + - 1 * cr * -sy );
		right[1] = ( -1 * sr * sp * sy + - 1 * cr * cy );
		right[2] = -1 * sr * cp;
	}
	if ( up ) {
		up[0] = ( cr * sp * cy + - sr * -sy );
		up[1] = ( cr * sp * sy + - sr * cy );
		up[2] = cr * cp;
	}
}

/*
** assumes "src" is normalized
*/
void PerpendicularVector( vec3_t dst, const vec3_t src ) {
	int pos;
	int i;
	float minelem = 1.0F;
	vec3_t tempvec;

	/*
	** find the smallest magnitude axially aligned vector
	*/
	for ( pos = 0, i = 0; i < 3; i++ )
	{
		if ( fabs( src[i] ) < minelem ) {
			pos = i;
			minelem = fabs( src[i] );
		}
	}
	tempvec[0] = tempvec[1] = tempvec[2] = 0.0F;
	tempvec[pos] = 1.0F;

	/*
	** project the point onto the plane defined by src
	*/
	ProjectPointOnPlane( dst, tempvec, src );

	/*
	** normalize the result
	*/
	VectorNormalize( dst );
}

/*
================
Q_isnan

Don't pass doubles to this
================
*/
int Q_isnan( float x )
{
	floatint_t fi;

	fi.f = x;
	fi.ui &= 0x7FFFFFFF;
	fi.ui = 0x7F800000 - fi.ui;

	return (int)( (unsigned int)fi.ui >> 31 );
}
//------------------------------------------------------------------------

#ifndef Q3_VM
/*
=====================
Q_acos

the msvc acos doesn't always return a value between -PI and PI:

int i;
i = 1065353246;
acos(*(float*) &i) == -1.#IND0

=====================
*/
float Q_acos(float c) {
	float angle;

	angle = acos(c);

	if (angle > M_PI) {
		return (float)M_PI;
	}
	if (angle < -M_PI) {
		return (float)M_PI;
	}
	return angle;
}
#endif

// Ridah
/*
=================
GetPerpendicularViewVector

  Used to find an "up" vector for drawing a sprite so that it always faces the view as best as possible
=================
*/
void GetPerpendicularViewVector( const vec3_t point, const vec3_t p1, const vec3_t p2, vec3_t up ) {
	vec3_t v1, v2;

	VectorSubtract( point, p1, v1 );
	VectorNormalize( v1 );

	VectorSubtract( point, p2, v2 );
	VectorNormalize( v2 );

	CrossProduct( v1, v2, up );
	VectorNormalize( up );
}

/*
================
ProjectPointOntoVector
================
*/
void ProjectPointOntoVector( vec3_t point, vec3_t vStart, vec3_t vEnd, vec3_t vProj ) {
	vec3_t pVec, vec;

	VectorSubtract( point, vStart, pVec );
	VectorSubtract( vEnd, vStart, vec );
	VectorNormalize( vec );
	// project onto the directional vector for this segment
	VectorMA( vStart, DotProduct( pVec, vec ), vec, vProj );
}

float vectoyaw( const vec3_t vec ) {
	float yaw;

	if ( vec[YAW] == 0 && vec[PITCH] == 0 ) {
		yaw = 0;
	} else {
		if ( vec[PITCH] ) {
			yaw = ( atan2( vec[YAW], vec[PITCH] ) * 180 / M_PI );
		} else if ( vec[YAW] > 0 ) {
			yaw = 90;
		} else {
			yaw = 270;
		}
		if ( yaw < 0 ) {
			yaw += 360;
		}
	}

	return yaw;
}

/*
=================
AxisToAngles

  Used to convert the MD3 tag axis to MDC tag angles, which are much smaller

  This doesn't have to be fast, since it's only used for conversion in utils, try to avoid
  using this during gameplay
=================
*/
void AxisToAngles( vec3_t axis[3], vec3_t angles ) {
	vec3_t right, roll_angles, tvec;

	// first get the pitch and yaw from the forward vector
	vectoangles( axis[0], angles );

	// now get the roll from the right vector
	VectorCopy( axis[1], right );
	// get the angle difference between the tmpAxis[2] and axis[2] after they have been reverse-rotated
	RotatePointAroundVector( tvec, axisDefault[2], right, -angles[YAW] );
	RotatePointAroundVector( right, axisDefault[1], tvec, -angles[PITCH] );
	// now find the angles, the PITCH is effectively our ROLL
	vectoangles( right, roll_angles );
	roll_angles[PITCH] = AngleNormalize180( roll_angles[PITCH] );
	// if the yaw is more than 90 degrees difference, we should adjust the pitch
	if ( DotProduct( right, axisDefault[1] ) < 0 ) {
		if ( roll_angles[PITCH] < 0 ) {
			roll_angles[PITCH] = -90 + ( -90 - roll_angles[PITCH] );
		} else {
			roll_angles[PITCH] =  90 + ( 90 - roll_angles[PITCH] );
		}
	}

	angles[ROLL] = -roll_angles[PITCH];
}

float VectorDistance( vec3_t v1, vec3_t v2 ) {
	vec3_t dir;

	VectorSubtract( v2, v1, dir );
	return VectorLength( dir );
}
// done.


/*
================
DistanceFromLineSquared
================
*/
float DistanceFromLineSquared( vec3_t p, vec3_t lp1, vec3_t lp2 ) {
	vec3_t proj, t;
	int j;

	ProjectPointOntoVector( p, lp1, lp2, proj );
	for ( j = 0; j < 3; j++ )
		if ( ( proj[j] > lp1[j] && proj[j] > lp2[j] ) ||
			 ( proj[j] < lp1[j] && proj[j] < lp2[j] ) ) {
			break;
		}
	if ( j < 3 ) {
		if ( Q_fabs( proj[j] - lp1[j] ) < Q_fabs( proj[j] - lp2[j] ) ) {
			VectorSubtract( p, lp1, t );
		} else {
			VectorSubtract( p, lp2, t );
		}
		return VectorLengthSquared( t );
	}
	VectorSubtract( p, proj, t );
	return VectorLengthSquared( t );
}