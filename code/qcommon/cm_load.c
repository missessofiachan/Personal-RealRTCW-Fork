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

// cmodel.c -- model loading

#include "cm_local.h"
#include "gp_jobsystem.h"
#include <stdlib.h>
#ifdef BSPC

#include "../bspc/l_qfiles.h"

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
#endif //BSPC

// to allow boxes to be treated as brush models, we allocate
// some extra indexes along with those needed by the map
#define BOX_BRUSHES     1
#define BOX_SIDES       6
#define BOX_LEAFS       2
#define BOX_PLANES      12

#define LL( x ) x = LittleLong( x )


clipMap_t cm;
int c_pointcontents;
int c_traces, c_brush_traces, c_patch_traces;


byte        *cmod_base;

#ifndef BSPC
cvar_t      *cm_noAreas;
cvar_t      *cm_noCurves;
cvar_t      *cm_playerCurveClip;
#endif

cmodel_t box_model;
cplane_t    *box_planes;
cbrush_t    *box_brush;



void    CM_InitBoxHull( void );
void    CM_FloodAreaConnections( void );


/*
===============================================================================

					MAP LOADING

===============================================================================
*/

/*
=================
CMod_LoadShaders
=================
*/
void CMod_LoadShaders( lump_t *l ) {
	dshader_t   *in, *out;
	int i, count;

	in = ( void * )( cmod_base + l->fileofs );
	if ( l->filelen % sizeof( *in ) ) {
		Com_Error( ERR_DROP, "CMod_LoadShaders: funny lump size" );
	}
	count = l->filelen / sizeof( *in );

	if ( count < 1 ) {
		Com_Error( ERR_DROP, "Map with no shaders" );
	}
	cm.shaders = Hunk_Alloc( count * sizeof( *cm.shaders ), h_high );
	cm.numShaders = count;

	Com_Memcpy( cm.shaders, in, count * sizeof( *cm.shaders ) );

	if ( LittleLong( 1 ) != 1 ) {
		out = cm.shaders;
		for ( i = 0 ; i < count ; i++, in++, out++ ) {
			out->contentFlags = LittleLong( out->contentFlags );
			out->surfaceFlags = LittleLong( out->surfaceFlags );
		}
	}
}


/*
=================
CMod_LoadSubmodels
=================
*/
void CMod_LoadSubmodels( lump_t *l ) {
	dmodel_t    *in;
	cmodel_t    *out;
	int i, j, count;
	int         *indexes;

	in = ( void * )( cmod_base + l->fileofs );
	if ( l->filelen % sizeof( *in ) ) {
		Com_Error( ERR_DROP, "CMod_LoadSubmodels: funny lump size" );
	}
	count = l->filelen / sizeof( *in );

	if ( count < 1 ) {
		Com_Error( ERR_DROP, "Map with no models" );
	}
	cm.cmodels = Hunk_Alloc( count * sizeof( *cm.cmodels ), h_high );
	cm.numSubModels = count;

	if ( count > MAX_SUBMODELS ) {
		Com_Error( ERR_DROP, "MAX_SUBMODELS exceeded" );
	}

	for ( i=0 ; i<count ; i++, in++)
	{
		out = &cm.cmodels[i];

		for ( j = 0 ; j < 3 ; j++ )
		{   // spread the mins / maxs by a pixel
			out->mins[j] = LittleFloat( in->mins[j] ) - 1;
			out->maxs[j] = LittleFloat( in->maxs[j] ) + 1;
		}

		if ( i == 0 ) {
			continue;   // world model doesn't need other info
		}

		// make a "leaf" just to hold the model's brushes and surfaces
		out->leaf.numLeafBrushes = LittleLong( in->numBrushes );
		indexes = Hunk_Alloc( out->leaf.numLeafBrushes * 4, h_high );
		out->leaf.firstLeafBrush = indexes - cm.leafbrushes;
		for ( j = 0 ; j < out->leaf.numLeafBrushes ; j++ ) {
			indexes[j] = LittleLong( in->firstBrush ) + j;
		}

		out->leaf.numLeafSurfaces = LittleLong( in->numSurfaces );
		indexes = Hunk_Alloc( out->leaf.numLeafSurfaces * 4, h_high );
		out->leaf.firstLeafSurface = indexes - cm.leafsurfaces;
		for ( j = 0 ; j < out->leaf.numLeafSurfaces ; j++ ) {
			indexes[j] = LittleLong( in->firstSurface ) + j;
		}
	}
}


/*
=================
CMod_LoadNodes

=================
*/
void CMod_LoadNodes( lump_t *l ) {
	dnode_t     *in;
	int child;
	cNode_t     *out;
	int i, j, count;

	in = ( void * )( cmod_base + l->fileofs );
	if ( l->filelen % sizeof( *in ) ) {
		Com_Error( ERR_DROP, "MOD_LoadBmodel: funny lump size" );
	}
	count = l->filelen / sizeof( *in );

	if ( count < 1 ) {
		Com_Error( ERR_DROP, "Map has no nodes" );
	}
	cm.nodes = Hunk_Alloc( count * sizeof( *cm.nodes ), h_high );
	cm.numNodes = count;

	out = cm.nodes;

	for ( i = 0 ; i < count ; i++, out++, in++ )
	{
		out->plane = cm.planes + LittleLong( in->planeNum );
		for ( j = 0 ; j < 2 ; j++ )
		{
			child = LittleLong( in->children[j] );
			out->children[j] = child;
		}
	}

}

/*
=================
CM_BoundBrush

=================
*/
void CM_BoundBrush( cbrush_t *b ) {
	b->bounds[0][0] = -b->sides[0].plane->dist;
	b->bounds[1][0] = b->sides[1].plane->dist;

	b->bounds[0][1] = -b->sides[2].plane->dist;
	b->bounds[1][1] = b->sides[3].plane->dist;

	b->bounds[0][2] = -b->sides[4].plane->dist;
	b->bounds[1][2] = b->sides[5].plane->dist;
}


/*
=================
CMod_LoadBrushes

=================
*/
void CMod_LoadBrushes( lump_t *l ) {
	dbrush_t    *in;
	cbrush_t    *out;
	int i, count;

	in = ( void * )( cmod_base + l->fileofs );
	if ( l->filelen % sizeof( *in ) ) {
		Com_Error( ERR_DROP, "MOD_LoadBmodel: funny lump size" );
	}
	count = l->filelen / sizeof( *in );

	cm.brushes = Hunk_Alloc( ( BOX_BRUSHES + count ) * sizeof( *cm.brushes ), h_high );
	cm.numBrushes = count;

	out = cm.brushes;

	for ( i = 0 ; i < count ; i++, out++, in++ ) {
		out->sides = cm.brushsides + LittleLong( in->firstSide );
		out->numsides = LittleLong( in->numSides );

		out->shaderNum = LittleLong( in->shaderNum );
		if ( out->shaderNum < 0 || out->shaderNum >= cm.numShaders ) {
			Com_Error( ERR_DROP, "CMod_LoadBrushes: bad shaderNum: %i", out->shaderNum );
		}
		out->contents = cm.shaders[out->shaderNum].contentFlags;

		CM_BoundBrush( out );
	}

}

/*
=================
CMod_LoadLeafs
=================
*/
void CMod_LoadLeafs( lump_t *l ) {
	int i;
	cLeaf_t     *out;
	dleaf_t     *in;
	int count;

	in = ( void * )( cmod_base + l->fileofs );
	if ( l->filelen % sizeof( *in ) ) {
		Com_Error( ERR_DROP, "MOD_LoadBmodel: funny lump size" );
	}
	count = l->filelen / sizeof( *in );

	if ( count < 1 ) {
		Com_Error( ERR_DROP, "Map with no leafs" );
	}

	cm.leafs = Hunk_Alloc( ( BOX_LEAFS + count ) * sizeof( *cm.leafs ), h_high );
	cm.numLeafs = count;

	out = cm.leafs;
	for ( i = 0 ; i < count ; i++, in++, out++ )
	{
		out->cluster = LittleLong( in->cluster );
		out->area = LittleLong( in->area );
		out->firstLeafBrush = LittleLong( in->firstLeafBrush );
		out->numLeafBrushes = LittleLong( in->numLeafBrushes );
		out->firstLeafSurface = LittleLong( in->firstLeafSurface );
		out->numLeafSurfaces = LittleLong( in->numLeafSurfaces );

		if ( out->cluster >= cm.numClusters ) {
			cm.numClusters = out->cluster + 1;
		}
		if ( out->area >= cm.numAreas ) {
			cm.numAreas = out->area + 1;
		}
	}

	cm.areas = Hunk_Alloc( cm.numAreas * sizeof( *cm.areas ), h_high );
	cm.areaPortals = Hunk_Alloc( cm.numAreas * cm.numAreas * sizeof( *cm.areaPortals ), h_high );
}

/*
=================
CMod_LoadPlanes
=================
*/
void CMod_LoadPlanes( lump_t *l ) {
	int i, j;
	cplane_t    *out;
	dplane_t    *in;
	int count;
	int bits;

	in = ( void * )( cmod_base + l->fileofs );
	if ( l->filelen % sizeof( *in ) ) {
		Com_Error( ERR_DROP, "MOD_LoadBmodel: funny lump size" );
	}
	count = l->filelen / sizeof( *in );

	if ( count < 1 ) {
		Com_Error( ERR_DROP, "Map with no planes" );
	}
	cm.planes = Hunk_Alloc( ( BOX_PLANES + count ) * sizeof( *cm.planes ), h_high );
	cm.numPlanes = count;

	out = cm.planes;

	for ( i = 0 ; i < count ; i++, in++, out++ )
	{
		bits = 0;
		for ( j = 0 ; j < 3 ; j++ )
		{
			out->normal[j] = LittleFloat( in->normal[j] );
			if ( out->normal[j] < 0 ) {
				bits |= 1 << j;
			}
		}

		out->dist = LittleFloat( in->dist );
		out->type = PlaneTypeForNormal( out->normal );
		out->signbits = bits;
	}
}

/*
=================
CMod_LoadLeafBrushes
=================
*/
void CMod_LoadLeafBrushes( lump_t *l ) {
	int i;
	int         *out;
	int         *in;
	int count;

	in = ( void * )( cmod_base + l->fileofs );
	if ( l->filelen % sizeof( *in ) ) {
		Com_Error( ERR_DROP, "MOD_LoadBmodel: funny lump size" );
	}
	count = l->filelen / sizeof( *in );

	cm.leafbrushes = Hunk_Alloc( count * sizeof( *cm.leafbrushes ), h_high );
	cm.numLeafBrushes = count;

	out = cm.leafbrushes;

	for ( i = 0 ; i < count ; i++, in++, out++ ) {
		*out = LittleLong( *in );
	}
}

/*
=================
CMod_LoadLeafSurfaces
=================
*/
void CMod_LoadLeafSurfaces( lump_t *l ) {
	int i;
	int         *out;
	int         *in;
	int count;

	in = ( void * )( cmod_base + l->fileofs );
	if ( l->filelen % sizeof( *in ) ) {
		Com_Error( ERR_DROP, "MOD_LoadBmodel: funny lump size" );
	}
	count = l->filelen / sizeof( *in );

	cm.leafsurfaces = Hunk_Alloc( count * sizeof( *cm.leafsurfaces ), h_high );
	cm.numLeafSurfaces = count;

	out = cm.leafsurfaces;

	for ( i = 0 ; i < count ; i++, in++, out++ ) {
		*out = LittleLong( *in );
	}
}

/*
=================
CMod_LoadBrushSides
=================
*/
void CMod_LoadBrushSides( lump_t *l ) {
	int i;
	cbrushside_t    *out;
	dbrushside_t    *in;
	int count;
	int num;

	in = ( void * )( cmod_base + l->fileofs );
	if ( l->filelen % sizeof( *in ) ) {
		Com_Error( ERR_DROP, "MOD_LoadBmodel: funny lump size" );
	}
	count = l->filelen / sizeof( *in );

	cm.brushsides = Hunk_Alloc( ( BOX_SIDES + count ) * sizeof( *cm.brushsides ), h_high );
	cm.numBrushSides = count;

	out = cm.brushsides;

	for ( i = 0 ; i < count ; i++, in++, out++ ) {
		num = LittleLong( in->planeNum );
		out->plane = &cm.planes[num];
		out->shaderNum = LittleLong( in->shaderNum );
		if ( out->shaderNum < 0 || out->shaderNum >= cm.numShaders ) {
			Com_Error( ERR_DROP, "CMod_LoadBrushSides: bad shaderNum: %i", out->shaderNum );
		}
		out->surfaceFlags = cm.shaders[out->shaderNum].surfaceFlags;
	}
}


/*
=================
CMod_LoadEntityString
=================
*/
void CMod_LoadEntityString( lump_t *l ) {
	cm.entityString = Hunk_Alloc( l->filelen, h_high );
	cm.numEntityChars = l->filelen;
	Com_Memcpy( cm.entityString, cmod_base + l->fileofs, l->filelen );
}

/*
=================
CMod_LoadVisibility
=================
*/
#define VIS_HEADER  8
void CMod_LoadVisibility( lump_t *l ) {
	int len;
	byte    *buf;

	len = l->filelen;
	if ( !len ) {
		cm.clusterBytes = ( cm.numClusters + 31 ) & ~31;
		cm.visibility = Hunk_Alloc( cm.clusterBytes, h_high );
		Com_Memset( cm.visibility, 255, cm.clusterBytes );
		return;
	}
	buf = cmod_base + l->fileofs;

	cm.vised = qtrue;
	cm.visibility = Hunk_Alloc( len, h_high );
	cm.numClusters = LittleLong( ( (int *)buf )[0] );
	cm.clusterBytes = LittleLong( ( (int *)buf )[1] );
	Com_Memcpy( cm.visibility, buf + VIS_HEADER, len - VIS_HEADER );
}

//==================================================================


/*
=================
CMod_LoadPatches
=================
*/
#define MAX_PATCH_VERTS     1024
void CMod_LoadPatches( lump_t *surfs, lump_t *verts ) {
	drawVert_t  *dv, *dv_p;
	dsurface_t  *in;
	int count;
	int i, j;
	int c;
	cPatch_t    *patch;
	vec3_t points[MAX_PATCH_VERTS];
	int width, height;
	int shaderNum;

	in = ( void * )( cmod_base + surfs->fileofs );
	if ( surfs->filelen % sizeof( *in ) ) {
		Com_Error( ERR_DROP, "MOD_LoadBmodel: funny lump size" );
	}
	cm.numSurfaces = count = surfs->filelen / sizeof( *in );
	cm.surfaces = Hunk_Alloc( cm.numSurfaces * sizeof( cm.surfaces[0] ), h_high );

	dv = ( void * )( cmod_base + verts->fileofs );
	if ( verts->filelen % sizeof( *dv ) ) {
		Com_Error( ERR_DROP, "MOD_LoadBmodel: funny lump size" );
	}

	// scan through all the surfaces, but only load patches,
	// not planar faces
	for ( i = 0 ; i < count ; i++, in++ ) {
		if ( LittleLong( in->surfaceType ) != MST_PATCH ) {
			continue;       // ignore other surfaces
		}
		// FIXME: check for non-colliding patches

		cm.surfaces[ i ] = patch = Hunk_Alloc( sizeof( *patch ), h_high );

		// load the full drawverts onto the stack
		width = LittleLong( in->patchWidth );
		height = LittleLong( in->patchHeight );
		c = width * height;
		if ( c > MAX_PATCH_VERTS ) {
			Com_Error( ERR_DROP, "ParseMesh: MAX_PATCH_VERTS" );
		}

		dv_p = dv + LittleLong( in->firstVert );
		for ( j = 0 ; j < c ; j++, dv_p++ ) {
			points[j][0] = LittleFloat( dv_p->xyz[0] );
			points[j][1] = LittleFloat( dv_p->xyz[1] );
			points[j][2] = LittleFloat( dv_p->xyz[2] );
		}

		shaderNum = LittleLong( in->shaderNum );
		patch->contents = cm.shaders[shaderNum].contentFlags;
		patch->surfaceFlags = cm.shaders[shaderNum].surfaceFlags;

		// create the internal facet structure
		patch->pc = CM_GeneratePatchCollide( width, height, points );
	}
}

//==================================================================


#if 0 //BSPC
/*
==================
CM_FreeMap

Free any loaded map and all submodels
==================
*/
void CM_FreeMap( void ) {
	Com_Memset( &cm, 0, sizeof( cm ) );
	Hunk_ClearHigh();
	CM_ClearLevelPatches();
}
#endif //BSPC

unsigned CM_LumpChecksum( lump_t *lump ) {
	return LittleLong( Com_BlockChecksum( cmod_base + lump->fileofs, lump->filelen ) );
}

unsigned CM_Checksum( dheader_t *header ) {
	unsigned checksums[16];
	checksums[0] = CM_LumpChecksum( &header->lumps[LUMP_SHADERS] );
	checksums[1] = CM_LumpChecksum( &header->lumps[LUMP_LEAFS] );
	checksums[2] = CM_LumpChecksum( &header->lumps[LUMP_LEAFBRUSHES] );
	checksums[3] = CM_LumpChecksum( &header->lumps[LUMP_LEAFSURFACES] );
	checksums[4] = CM_LumpChecksum( &header->lumps[LUMP_PLANES] );
	checksums[5] = CM_LumpChecksum( &header->lumps[LUMP_BRUSHSIDES] );
	checksums[6] = CM_LumpChecksum( &header->lumps[LUMP_BRUSHES] );
	checksums[7] = CM_LumpChecksum( &header->lumps[LUMP_MODELS] );
	checksums[8] = CM_LumpChecksum( &header->lumps[LUMP_NODES] );
	checksums[9] = CM_LumpChecksum( &header->lumps[LUMP_SURFACES] );
	checksums[10] = CM_LumpChecksum( &header->lumps[LUMP_DRAWVERTS] );

	return LittleLong( Com_BlockChecksum( checksums, 11 * 4 ) );
}





/*
==================
CM_LoadMap

Loads in the map and all submodels
==================
*/
void CM_LoadMap( const char *name, qboolean clientload, int *checksum ) {
	union {
		int				*i;
		void			*v;
	} buf;
	int i;
	dheader_t header;
	int length;
	static unsigned last_checksum;

	if ( !name || !name[0] ) {
		Com_Error( ERR_DROP, "CM_LoadMap: NULL name" );
	}

#ifndef BSPC
	cm_noAreas = Cvar_Get( "cm_noAreas", "0", CVAR_CHEAT );
	cm_noCurves = Cvar_Get( "cm_noCurves", "0", CVAR_CHEAT );
	cm_playerCurveClip = Cvar_Get( "cm_playerCurveClip", "1", CVAR_ARCHIVE | CVAR_CHEAT );
#endif
	Com_DPrintf( "CM_LoadMap( %s, %i )\n", name, clientload );

	if ( !strcmp( cm.name, name ) && clientload ) {
		*checksum = last_checksum;
		return;
	}

	// free old stuff
	Com_Memset( &cm, 0, sizeof( cm ) );
	CM_ClearLevelPatches();

	if ( !name[0] ) {
		cm.numLeafs = 1;
		cm.numClusters = 1;
		cm.numAreas = 1;
		cm.cmodels = Hunk_Alloc( sizeof( *cm.cmodels ), h_high );
		*checksum = 0;
		return;
	}

	//
	// load the file
	//
#ifndef BSPC
	length = FS_ReadFile( name, &buf.v );
#else
	length = LoadQuakeFile((quakefile_t *) name, &buf.v);
#endif

	if ( !buf.i ) {
		Com_Error( ERR_DROP, "Couldn't load %s", name );
	}

	last_checksum = LittleLong (Com_BlockChecksum (buf.i, length));
	*checksum = last_checksum;

// --- QUICKLOAD CACHE: Capture a backup clone in RAM ---
	{
		extern void CM_StoreSaveCache( const char *mapName, void *buffer, int len );
		CM_StoreSaveCache( name, buf.v, length );
	}
	// ------------------------------------------------------

	header = *(dheader_t *)buf.i;
	for ( i = 0 ; i < sizeof( dheader_t ) / 4 ; i++ ) {
		( (int *)&header )[i] = LittleLong( ( (int *)&header )[i] );
	}

#ifndef _SKIP_BSP_CHECK
	if ( header.version != BSP_VERSION ) {
		Com_Error( ERR_DROP, "CM_LoadMap: %s has wrong version number (%i should be %i)"
				   , name, header.version, BSP_VERSION );
	}
#endif

	cmod_base = (byte *)buf.i;

	// load into heap
	CMod_LoadShaders( &header.lumps[LUMP_SHADERS] );
	CMod_LoadLeafs( &header.lumps[LUMP_LEAFS] );
	CMod_LoadLeafBrushes( &header.lumps[LUMP_LEAFBRUSHES] );
	CMod_LoadLeafSurfaces( &header.lumps[LUMP_LEAFSURFACES] );
	CMod_LoadPlanes( &header.lumps[LUMP_PLANES] );
	CMod_LoadBrushSides( &header.lumps[LUMP_BRUSHSIDES] );
	CMod_LoadBrushes( &header.lumps[LUMP_BRUSHES] );
	CMod_LoadSubmodels( &header.lumps[LUMP_MODELS] );
	CMod_LoadNodes( &header.lumps[LUMP_NODES] );
	CMod_LoadEntityString( &header.lumps[LUMP_ENTITIES] );
	CMod_LoadVisibility( &header.lumps[LUMP_VISIBILITY] );
	CMod_LoadPatches( &header.lumps[LUMP_SURFACES], &header.lumps[LUMP_DRAWVERTS] );

	// we are NOT freeing the file, because it is cached for the ref
	FS_FreeFile( buf.v );

	CM_InitBoxHull();

	CM_FloodAreaConnections();

	// allow this to be cached if it is loaded by the server
	if ( !clientload ) {
		Q_strncpyz( cm.name, name, sizeof( cm.name ) );
	}
}

/*
==================
CM_ClearMap
==================
*/
void CM_ClearMap( void ) {
	Com_Memset( &cm, 0, sizeof( cm ) );
	CM_ClearLevelPatches();
}

/*
==================
CM_ClipHandleToModel
==================
*/
cmodel_t    *CM_ClipHandleToModel( clipHandle_t handle ) {
	if ( handle < 0 ) {
		Com_Error( ERR_DROP, "CM_ClipHandleToModel: bad handle %i", handle );
	}
	if ( handle < cm.numSubModels ) {
		return &cm.cmodels[handle];
	}
	if ( handle == BOX_MODEL_HANDLE || handle == CAPSULE_MODEL_HANDLE ) {
		return &box_model;
	}
	if ( handle < MAX_SUBMODELS ) {
		Com_Error( ERR_DROP, "CM_ClipHandleToModel: bad handle %i < %i < %i",
				   cm.numSubModels, handle, MAX_SUBMODELS );
	}
	Com_Error( ERR_DROP, "CM_ClipHandleToModel: bad handle %i", handle + MAX_SUBMODELS );

	return NULL;

}

/*
==================
CM_InlineModel
==================
*/
clipHandle_t    CM_InlineModel( int index ) {
	if ( index < 0 || index >= cm.numSubModels ) {
		Com_Error( ERR_DROP, "CM_InlineModel: bad number" );
	}
	return index;
}

int     CM_NumClusters( void ) {
	return cm.numClusters;
}

int     CM_NumInlineModels( void ) {
	return cm.numSubModels;
}

char    *CM_EntityString( void ) {
	return cm.entityString;
}

int     CM_LeafCluster( int leafnum ) {
	if ( leafnum < 0 || leafnum >= cm.numLeafs ) {
		Com_Error( ERR_DROP, "CM_LeafCluster: bad number" );
	}
	return cm.leafs[leafnum].cluster;
}

int     CM_LeafArea( int leafnum ) {
	if ( leafnum < 0 || leafnum >= cm.numLeafs ) {
		Com_Error( ERR_DROP, "CM_LeafArea: bad number" );
	}
	return cm.leafs[leafnum].area;
}

//=======================================================================


/*
===================
CM_InitBoxHull

Set up the planes and nodes so that the six floats of a bounding box
can just be stored out and get a proper clipping hull structure.
===================
*/
void CM_InitBoxHull( void ) {
	int i;
	int side;
	cplane_t    *p;
	cbrushside_t    *s;

	box_planes = &cm.planes[cm.numPlanes];

	box_brush = &cm.brushes[cm.numBrushes];
	box_brush->numsides = 6;
	box_brush->sides = cm.brushsides + cm.numBrushSides;
	box_brush->contents = CONTENTS_BODY;

	box_model.leaf.numLeafBrushes = 1;
//	box_model.leaf.firstLeafBrush = cm.numBrushes;
	box_model.leaf.firstLeafBrush = cm.numLeafBrushes;
	cm.leafbrushes[cm.numLeafBrushes] = cm.numBrushes;

	for ( i = 0 ; i < 6 ; i++ )
	{
		side = i & 1;

		// brush sides
		s = &cm.brushsides[cm.numBrushSides + i];
		s->plane =  cm.planes + ( cm.numPlanes + i * 2 + side );
		s->surfaceFlags = 0;

		// planes
		p = &box_planes[i * 2];
		p->type = i >> 1;
		p->signbits = 0;
		VectorClear( p->normal );
		p->normal[i >> 1] = 1;

		p = &box_planes[i * 2 + 1];
		p->type = 3 + ( i >> 1 );
		p->signbits = 0;
		VectorClear( p->normal );
		p->normal[i >> 1] = -1;

		SetPlaneSignbits( p );
	}
}

/*
===================
CM_TempBoxModel

To keep everything totally uniform, bounding boxes are turned into small
BSP trees instead of being compared directly.
Capsules are handled differently though.
===================
*/
clipHandle_t CM_TempBoxModel( const vec3_t mins, const vec3_t maxs, int capsule ) {

	VectorCopy( mins, box_model.mins );
	VectorCopy( maxs, box_model.maxs );

	box_planes[0].dist = maxs[0];
	box_planes[1].dist = -maxs[0];
	box_planes[2].dist = mins[0];
	box_planes[3].dist = -mins[0];
	box_planes[4].dist = maxs[1];
	box_planes[5].dist = -maxs[1];
	box_planes[6].dist = mins[1];
	box_planes[7].dist = -mins[1];
	box_planes[8].dist = maxs[2];
	box_planes[9].dist = -maxs[2];
	box_planes[10].dist = mins[2];
	box_planes[11].dist = -mins[2];

	VectorCopy( mins, box_brush->bounds[0] );
	VectorCopy( maxs, box_brush->bounds[1] );

	if ( capsule ) {
		return CAPSULE_MODEL_HANDLE;
	}

	return BOX_MODEL_HANDLE;
}

/*
===================
CM_ModelBounds
===================
*/
void CM_ModelBounds( clipHandle_t model, vec3_t mins, vec3_t maxs ) {
	if ( !cm.numNodes ) {
		VectorClear( mins );
		VectorClear( maxs );
		return;
	}

	cmodel_t    *cmod;

	cmod = CM_ClipHandleToModel( model );
	VectorCopy( cmod->mins, mins );
	VectorCopy( cmod->maxs, maxs );
}
// ==================================================================================
// SEAMLESS LEVEL STREAMING & QUICKLOAD DOUBLE-BUFFER CACHE
// ==================================================================================


typedef enum {
	STREAM_IDLE,
	STREAM_LOADING,
	STREAM_READY
} streamState_t;

typedef struct {
	char          mapName[MAX_QPATH];
	streamState_t state;
	void          *buffer;
	int           bufferLen;
} mapStreamer_t;

typedef struct {
	char          mapName[MAX_QPATH];
	void          *buffer;
	int           bufferLen;
} saveCache_t;

// Double-Buffered Storage allocations
static mapStreamer_t g_mapStreamer = { "", STREAM_IDLE, NULL, 0 };
static saveCache_t   g_saveCache   = { "", NULL, 0 };

/*
==================
CM_StoreSaveCache

Copies the current live map data to heap RAM to bypass disk reads during quickloads.
==================
*/
void CM_StoreSaveCache( const char *mapName, void *buffer, int len ) {
	if ( !mapName || !mapName[0] || !buffer || len <= 0 ) return;

	if ( g_saveCache.buffer ) {
		free( g_saveCache.buffer );
		g_saveCache.buffer = NULL;
	}

	Q_strncpyz( g_saveCache.mapName, mapName, sizeof( g_saveCache.mapName ) );
	g_saveCache.bufferLen = len;
	g_saveCache.buffer = malloc( len );
	
	if ( g_saveCache.buffer ) {
		Com_Memcpy( g_saveCache.buffer, buffer, len );
	}
}

/*
==================
CM_LoadMapFromBuffer

Directly parses structural collision arrays out of a high-speed memory block.
==================
*/
void CM_LoadMapFromBuffer( const char *name, void *bufferData, int bufferLen, qboolean clientload, int *checksum ) {
	int i;
	dheader_t header;
	static unsigned last_checksum;

	if ( !name || !name[0] ) {
		Com_Error( ERR_DROP, "CM_LoadMapFromBuffer: NULL name" );
	}

	if ( !bufferData || bufferLen <= 0 ) {
		Com_Error( ERR_DROP, "CM_LoadMapFromBuffer: Invalid buffer pointer for %s", name );
	}

#ifndef BSPC
	cm_noAreas = Cvar_Get( "cm_noAreas", "0", CVAR_CHEAT );
	cm_noCurves = Cvar_Get( "cm_noCurves", "0", CVAR_CHEAT );
	cm_playerCurveClip = Cvar_Get( "cm_playerCurveClip", "1", CVAR_ARCHIVE | CVAR_CHEAT );
#endif

	if ( !strcmp( cm.name, name ) && clientload ) {
		last_checksum = LittleLong (Com_BlockChecksum (bufferData, bufferLen));
		*checksum = last_checksum;
		return;
	}

	Com_Memset( &cm, 0, sizeof( cm ) );
	CM_ClearLevelPatches();

	last_checksum = LittleLong (Com_BlockChecksum (bufferData, bufferLen));
	*checksum = last_checksum;

	header = *(dheader_t *)bufferData;
	for ( i = 0 ; i < sizeof( dheader_t ) / 4 ; i++ ) {
		( (int *)&header )[i] = LittleLong( ( (int *)&header )[i] );
	}

#ifndef _SKIP_BSP_CHECK
	if ( header.version != BSP_VERSION ) {
		Com_Error( ERR_DROP, "CM_LoadMapFromBuffer: %s has wrong version number (%i should be %i)", 
			name, header.version, BSP_VERSION );
	}
#endif

	cmod_base = (byte *)bufferData;

	CMod_LoadShaders( &header.lumps[LUMP_SHADERS] );
	CMod_LoadLeafs( &header.lumps[LUMP_LEAFS] );
	CMod_LoadLeafBrushes( &header.lumps[LUMP_LEAFBRUSHES] );
	CMod_LoadLeafSurfaces( &header.lumps[LUMP_LEAFSURFACES] );
	CMod_LoadPlanes( &header.lumps[LUMP_PLANES] );
	CMod_LoadBrushSides( &header.lumps[LUMP_BRUSHSIDES] );
	CMod_LoadBrushes( &header.lumps[LUMP_BRUSHES] );
	CMod_LoadSubmodels( &header.lumps[LUMP_MODELS] );
	CMod_LoadNodes( &header.lumps[LUMP_NODES] );
	CMod_LoadEntityString( &header.lumps[LUMP_ENTITIES] );
	CMod_LoadVisibility( &header.lumps[LUMP_VISIBILITY] );
	CMod_LoadPatches( &header.lumps[LUMP_SURFACES], &header.lumps[LUMP_DRAWVERTS] );

	CM_InitBoxHull();
	CM_FloodAreaConnections();

	if ( !clientload ) {
		Q_strncpyz( cm.name, name, sizeof( cm.name ) );
	}
	
	// Re-cache this data pointer as the active quickload target
	CM_StoreSaveCache( name, bufferData, bufferLen );

	Com_Printf( "Stream-System: Collision layers parsed directly from memory buffer!\n" );
}

/*
==================
AsyncMapStreamWorker
==================
*/
static void AsyncMapStreamWorker(void *arg) {
	mapStreamer_t *streamer = (mapStreamer_t *)arg;
	if (!streamer) return;

	extern void *FS_LoadFileAsync(const char *qpath, int *outLen);
	streamer->buffer = FS_LoadFileAsync(streamer->mapName, &streamer->bufferLen);

	if (!streamer->buffer || streamer->bufferLen <= 0) {
		streamer->state = STREAM_IDLE;
		return;
	}

	streamer->state = STREAM_READY;
	Com_Printf("Stream-System: Next level '%s' staged in RAM background buffer.\n", streamer->mapName);
}

/*
==================
CM_TriggerMapStream
==================
*/
void CM_TriggerMapStream(const char *mapName) {
	if (!mapName || !mapName[0]) return;
	if (g_mapStreamer.state != STREAM_IDLE) return;

	Q_strncpyz(g_mapStreamer.mapName, mapName, sizeof(g_mapStreamer.mapName));
	g_mapStreamer.state = STREAM_LOADING;
	g_mapStreamer.buffer = NULL;
	g_mapStreamer.bufferLen = 0;

	Sys_QueueJob(AsyncMapStreamWorker, &g_mapStreamer);
}

/*
==================
CM_GetStreamedBuffer

INTERCEPTOR: Checks both the background stream cache (for map progression) 
and the local save cache (for fast hot-reloading quickloads).
==================
*/
void *CM_GetStreamedBuffer( const char *mapName, int *outLen ) {
	if ( !mapName || !mapName[0] || !outLen ) return NULL;

	// Check 1: Background Streaming Lane (Moving forward to next map)
	if ( g_mapStreamer.state == STREAM_READY && Q_stricmp( g_mapStreamer.mapName, mapName ) == 0 ) {
		Sys_WaitJobs();
		void *buf = g_mapStreamer.buffer;
		*outLen = g_mapStreamer.bufferLen;
		
		g_mapStreamer.buffer = NULL;
		g_mapStreamer.bufferLen = 0;
		g_mapStreamer.state = STREAM_IDLE;
		
		Com_Printf( "Stream-System: Progression buffer successfully claimed from streamer.\n" );
		return buf;
	}

	// Check 2: Save Game / Quickload Lane (Hot-reloading current map)
	if ( g_saveCache.buffer && Q_stricmp( g_saveCache.mapName, mapName ) == 0 ) {
		void *dupBuf = malloc( g_saveCache.bufferLen );
		if ( dupBuf ) {
			Com_Memcpy( dupBuf, g_saveCache.buffer, g_saveCache.bufferLen );
			*outLen = g_saveCache.bufferLen;
			Com_Printf( "Stream-System: Current map buffer successfully claimed from Quickload Cache!\n" );
			return dupBuf;
		}
	}

	return NULL;
}

/*
==================
CM_PollStreamerHandshake
==================
*/
void CM_PollStreamerHandshake(void) {
	if (g_mapStreamer.state != STREAM_READY) return;
}

/*
==================
CM_AutoTriggerNextCampaignMap

Advanced script parsing level transition identifier.
==================
*/
void CM_AutoTriggerNextCampaignMap( const char *currentMapName ) {
	char *p;
	char nextMap[MAX_QPATH];
	int i;

	if ( !currentMapName || !currentMapName[0] ) return;

	Com_Printf( "Stream-System: Running dynamic mod entity tracking pass...\n" );
	nextMap[0] = '\0';

	if ( cm.entityString && cm.numEntityChars > 0 ) {
		p = strstr( cm.entityString, "\"target_changelevel\"" );
		if ( !p ) p = strstr( cm.entityString, "\"nextmap\"" );

		if ( p ) {
			char *mapKey = strstr( p - 300 > cm.entityString ? p - 300 : cm.entityString, "\"map\"" );
			if ( !mapKey || mapKey > p + 300 ) mapKey = strstr( p, "\"map\"" );
			if ( mapKey ) {
				char *value = strstr( mapKey + 5, "\"" );
				if ( value ) {
					value++;
					for ( i = 0 ; i < MAX_QPATH - 1 ; i++ ) {
						if ( value[i] == '"' || value[i] == '\0' || value[i] == '\n' || value[i] == '\r' ) break;
						nextMap[i] = value[i];
					}
					nextMap[i] = '\0';
				}
			}
		}
	}

	if ( !nextMap[0] ) {
		char scriptPath[MAX_QPATH];
		void *scriptBuffer;
		int scriptLen = 0;

		Q_strncpyz( scriptPath, currentMapName, sizeof( scriptPath ) );
		char *ext = strrchr( scriptPath, '.' );
		if ( ext ) *ext = '\0';
		Q_strcat( scriptPath, sizeof( scriptPath ), ".script" );

		extern void *FS_LoadFileAsync( const char *qpath, int *outLen );
		scriptBuffer = FS_LoadFileAsync( scriptPath, &scriptLen );

		if ( scriptBuffer && scriptLen > 0 ) {
			p = strstr( (char *)scriptBuffer, "changelevel" );
			if ( p ) {
				p += 11;
				while ( *p == ' ' || *p == '\t' ) p++;
				for ( i = 0 ; i < MAX_QPATH - 1 ; i++ ) {
					if ( p[i] == ' ' || p[i] == '\t' || p[i] == '\n' || p[i] == '\r' || p[i] == '\0' ) break;
					nextMap[i] = p[i];
				}
				nextMap[i] = '\0';
			}
			free( scriptBuffer );
		}
	}

	if ( nextMap[0] ) {
		char bspPath[MAX_QPATH];
		if ( !strstr( nextMap, "maps/" ) ) {
			Com_sprintf( bspPath, sizeof( bspPath ), "maps/%s.bsp", nextMap );
		} else {
			Com_sprintf( bspPath, sizeof( bspPath ), "%s", nextMap );
		}
		if ( !strstr( bspPath, ".bsp" ) ) {
			Q_strcat( bspPath, sizeof( bspPath ), ".bsp" );
		}

		Com_Printf( "Stream-System: Dynamic script scanner discovered next target level -> '%s'\n", bspPath );
		CM_TriggerMapStream( bspPath );
	} else {
		Com_Printf( "Stream-System: No transition markers located. Auto-streaming resting.\n" );
	}
}

/*
==================
CM_StreamMap_f
==================
*/
void CM_StreamMap_f(void) {
	if (Cmd_Argc() < 2) return;
	CM_TriggerMapStream(Cmd_Argv(1));
}