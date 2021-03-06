/*
Copyright (C) 1996-1997 Id Software, Inc.
2016 Atröm "Panzerschrek" Kunç.

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
// d_local.h:  private rasterization driver defs

#include "r_shared.h"

//
// TODO: fine-tune this; it's based on providing some overage even if there
// is a 2k-wide scan, with subdivision every 8, for 256 spans of 12 bytes each
//
#define SCANBUFFERPAD		0x1000

#define R_SKY_SMASK	0x007F0000
#define R_SKY_TMASK	0x007F0000

#define DS_SPAN_LIST_END	-128

#define SURFCACHE_SIZE_AT_320X200	600*1024

typedef struct {
	void			*pdest;
	short			*pz;
	int				count;
	byte			*ptex;
	int				sfrac, tfrac, light, zi;
} spanpackage_t;

typedef struct surfcache_s
{
	struct surfcache_s	*next;
	struct surfcache_s 	**owner;		// NULL is an empty chunk of memory
	int					lightadj[MAXLIGHTMAPS]; // checked for strobe flush
	int					dlight;
	int					size;		// including header
	unsigned			width;
	unsigned			height;		// DEBUG only needed for debug
	float				mipscale;
	struct texture_s	*texture;	// checked for animating textures
	byte				data[4];	// width*height elements
} surfcache_t;

// !!! if this is changed, it must be changed in asm_draw.h too !!!
typedef struct sspan_s
{
	int				u, v, count;
} sspan_t;

extern cvar_t	d_subdiv16;

extern float	scale_for_mip;

extern qboolean		d_roverwrapped;
extern surfcache_t	*sc_rover;
extern surfcache_t	*d_initial_rover;

extern float	d_sdivzstepu, d_tdivzstepu, d_zistepu;
extern float	d_sdivzstepv, d_tdivzstepv, d_zistepv;
extern float	d_sdivzorigin, d_tdivzorigin, d_ziorigin;

fixed16_t	sadjust, tadjust;
fixed16_t	bbextents, bbextentt;


void D_DrawSpans8 (espan_t *pspans);
void D_DrawSpans32 (espan_t *pspans);
void D_DrawZSpans (espan_t *pspans);
void Turbulent8 (espan_t *pspan);
void Turbulent32 (espan_t *pspan);
void D_SpriteDrawSpans8 (sspan_t *pspan);
void D_SpriteDrawSpans32 (sspan_t *pspan);

void D_DrawSkyScans8 (espan_t *pspan);
void D_DrawSkyScans32 (espan_t *pspan);

void D_DrawParticlePixels8(void);
void D_DrawParticlePixels32(void);

void D_PolysetDrawSpans8 (spanpackage_t *pspanpackage);
void D_PolysetDrawSpans32 (spanpackage_t *pspanpackage);

void R_ShowSubDiv (void);
void (*prealspandrawer)(void);
surfcache_t	*D_CacheSurface (msurface_t *surface, int miplevel);

extern int D_MipLevelForScale (float scale);

extern short *d_pzbuffer;
extern unsigned int d_zrowbytes, d_zwidth;

extern int	*d_pscantable;
extern int	d_scantable[MAXHEIGHT];

extern int	d_vrectx, d_vrecty, d_vrectright_particle, d_vrectbottom_particle;

extern int		d_y_aspect_shift, d_pix_min, d_pix_max, d_pix_shift;
extern float	d_pix_mul;

extern pixel_t	*d_viewbuffer;

extern short	*zspantable[MAXHEIGHT];

extern int		d_minmip;
extern float	d_scalemip[3];

extern void (*d_drawspans) (espan_t *pspan);
extern void (*d_drawturbulent) (espan_t *pspan);
extern void (*d_drawskyscans) (espan_t *pspan);
extern void (*d_drawparticlepixels) (void);
extern void (*d_drawpolysetspans) (spanpackage_t *pspanpackage);
extern void (*d_spritedrawspans) (sspan_t *pspan);

extern	cvar_t		r_particle_size;