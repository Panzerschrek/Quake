/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#ifndef __GL_MODEL__
#define __GL_MODEL__

#include "modelgen.h"
#include "spritegn.h"

#include "model_common.h"

/*

d*_t structures are on-disk representations
m*_t structures are in-memory

*/

// entity effects

#define	EF_BRIGHTFIELD			1
#define	EF_MUZZLEFLASH 			2
#define	EF_BRIGHTLIGHT 			4
#define	EF_DIMLIGHT 			8


/*
==============================================================================

BRUSH MODELS

==============================================================================
*/


typedef struct gl_texture_s
{
	char				name[16];
	unsigned			width, height;
	int					gl_texturenum;
	struct msurface_s	*texturechain;	// for gl_texsort drawing
	int					anim_total;				// total tenths in sequence ( 0 = no)
	int					anim_min, anim_max;		// time for this frame min <=time< max
	struct gl_texture_s *anim_next;		// in the animation sequence
	struct gl_texture_s *alternate_anims;	// bmodels in frmae 1 use these
	unsigned			offsets[MIPLEVELS];		// four mip maps stored
} gl_texture_t;

typedef struct
{
	float			vecs[2][4];
	float			mipadjust;
	gl_texture_t	*texture;
	int				flags;
} gl_mtexinfo_t;

#define	VERTEXSIZE	7

typedef struct glpoly_s
{
	struct	glpoly_s	*next;
	struct	glpoly_s	*chain;
	int		numverts;
	int		flags;			// for SURF_UNDERWATER
	float	verts[4][VERTEXSIZE];	// variable sized (xyz s1t1 s2t2)
} glpoly_t;

typedef struct gl_msurface_s
{
	int			visframe;		// should be drawn when node is crossed

	mplane_t	*plane;
	int			flags;

	int			firstedge;	// look up in model->surfedges[], negative numbers
	int			numedges;	// are backwards edges
	
	short		texturemins[2];
	short		extents[2];

	int			light_s, light_t;	// gl lightmap coordinates

	glpoly_t	*polys;				// multiple if warped
	struct	msurface_s	*texturechain;

	gl_mtexinfo_t	*texinfo;
	
// lighting info
	int			dlightframe;
	int			dlightbits;

	int			lightmaptexturenum;
	byte		styles[MAXLIGHTMAPS];
	int			cached_light[MAXLIGHTMAPS];	// values currently used in lightmap
	qboolean	cached_dlight;				// true if dynamic light in cache
	byte		*samples;		// [numstyles*surfsize]
} gl_msurface_t;


typedef struct gl_mleaf_s
{
// common with node
	int			contents;		// wil be a negative contents number
	int			visframe;		// node needs to be traversed if current

	float		minmaxs[6];		// for bounding box culling

	struct mnode_s	*parent;

// leaf specific
	byte		*compressed_vis;
	efrag_t		*efrags;

	gl_msurface_t	**firstmarksurface;
	int			nummarksurfaces;
	int			key;			// BSP sequence number for leaf's contents
	byte		ambient_sound_level[NUM_AMBIENTS];
} gl_mleaf_t;

/*
==============================================================================

SPRITE MODELS

==============================================================================
*/


// FIXME: shorten these?
typedef struct gl_mspriteframe_s
{
	int		width;
	int		height;
	float	up, down, left, right;
	int		gl_texturenum;
} gl_mspriteframe_t;

typedef struct
{	
	int					numframes;
	float				*intervals;
	gl_mspriteframe_t	*frames[1];
} gl_mspritegroup_t;

typedef struct
{
	spriteframetype_t	type;
	gl_mspriteframe_t		*frameptr;
} gl_mspriteframedesc_t;

typedef struct
{
	int					type;
	int					maxwidth;
	int					maxheight;
	int					numframes;
	float				beamlength;		// remove?
	void				*cachespot;		// remove?
	gl_mspriteframedesc_t	frames[1];
} gl_msprite_t;


/*
==============================================================================

ALIAS MODELS

Alias models are position independent, so the cache manager can move them.
==============================================================================
*/

typedef struct
{
	int					firstpose;
	int					numposes;
	float				interval;
	trivertx_t			bboxmin;
	trivertx_t			bboxmax;
	int					frame;
	char				name[16];
} gl_maliasframedesc_t;

#define	MAX_SKINS	32
typedef struct {
	int			ident;
	int			version;
	vec3_t		scale;
	vec3_t		scale_origin;
	float		boundingradius;
	vec3_t		eyeposition;
	int			numskins;
	int			skinwidth;
	int			skinheight;
	int			numverts;
	int			numtris;
	int			numframes;
	synctype_t	synctype;
	int			flags;
	float		size;

	int					numposes;
	int					poseverts;
	int					posedata;	// numposes*poseverts trivert_t
	int					commands;	// gl command list with embedded s/t
	int					gl_texturenum[MAX_SKINS][4];
	int					texels[MAX_SKINS];	// only for player skins
	gl_maliasframedesc_t	frames[1];	// variable sized
} gl_aliashdr_t;

#define	MAXALIASVERTS	1024
#define	MAXALIASFRAMES	256
#define	MAXALIASTRIS	2048
extern	gl_aliashdr_t	*pheader;
extern	stvert_t	stverts[MAXALIASVERTS];
extern	mtriangle_t	triangles[MAXALIASTRIS];
extern	trivertx_t	*poseverts[MAXALIASFRAMES];

typedef struct gl_model_s
{
	char		name[MAX_QPATH];
	qboolean	needload;		// bmodels and sprites don't cache normally

	modtype_t	type;
	int			numframes;
	synctype_t	synctype;
	
	int			flags;

//
// volume occupied by the model graphics
//		
	vec3_t		mins, maxs;
	float		radius;

//
// solid volume for clipping 
//
	qboolean	clipbox;
	vec3_t		clipmins, clipmaxs;

//
// brush model
//
	int			firstmodelsurface, nummodelsurfaces;

	int			numsubmodels;
	dmodel_t	*submodels;

	int			numplanes;
	mplane_t	*planes;

	int				numleafs;		// number of visible leafs, not counting 0
	gl_mleaf_t		*leafs;

	int			numvertexes;
	mvertex_t	*vertexes;

	int			numedges;
	medge_t		*edges;

	int			numnodes;
	mnode_t		*nodes;

	int				numtexinfo;
	gl_mtexinfo_t	*texinfo;

	int			numsurfaces;
	gl_msurface_t	*surfaces;

	int			numsurfedges;
	int			*surfedges;

	int			numclipnodes;
	dclipnode_t	*clipnodes;

	int				nummarksurfaces;
	gl_msurface_t	**marksurfaces;

	hull_t		hulls[MAX_MAP_HULLS];

	int				numtextures;
	gl_texture_t	**textures;

	byte		*visdata;
	byte		*lightdata;
	char		*entities;

//
// additional model data
//
	cache_user_t	cache;		// only access through Mod_Extradata

} gl_model_t;

//============================================================================

void	Mod_Init (void);
void	Mod_ClearAll (void);
gl_model_t *ModGL_ForName (char *name, qboolean crash);
void	*ModGL_Extradata (gl_model_t *mod);	// handles caching
void	Mod_TouchModel (char *name);

gl_mleaf_t *ModGL_PointInLeaf (float *p, gl_model_t *model);
byte	*ModGL_LeafPVS (gl_mleaf_t *leaf, gl_model_t *model);

#endif	// __GL_MODEL__
