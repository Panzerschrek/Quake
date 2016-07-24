/*
Copyright (C) 1996-1997 Id Software, Inc.
20016 Atröm "Panzerschrek" Kunç.

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

#include "quakedef.h"
#include "winquake.h"

#include <SDL.h>
#include <SDL_opengl.h>

// Some subsystem needs it
modestate_t	modestate = MS_UNINIT;
cvar_t		_windowed_mouse = {"_windowed_mouse","0", true};
cvar_t	gl_ztrick = {"gl_ztrick","0"};

unsigned short	d_8to16table[256];
unsigned		d_8to24table[256];
unsigned char	d_15to8table[65536];

qboolean gl_mtexable = false;
qboolean isPermedia = true;
int		texture_mode = GL_LINEAR_MIPMAP_LINEAR;
int		texture_extension_number = 1;

BINDTEXFUNCPTR bindTexFunc;

float gldepthmin = -1.0f, gldepthmax = 1.0f;

struct
{
	SDL_Window* window;
	SDL_GLContext* context;

} g_sdl_gl;

void	VID_Init (unsigned char *palette)
{
	int		param_width ;
	int		param_height;

	param_width  = COM_CheckParm( "-width" );
	param_height = COM_CheckParm( "-height" );
	if( param_width  != 0 && param_height != 0 )
	{
		vid.width  = Q_atoi( com_argv[ param_width  + 1 ] );
		vid.height = Q_atoi( com_argv[ param_height + 1 ] );
	}
	else
	{
		vid.width  = 640;
		vid.height = 480;
	}

	vid.rowbytes = 0;
	vid.numpages = 2;
	vid.maxwarpwidth  = 0;
	vid.maxwarpheight = 0;
	vid.aspect = 1.0f;

	vid.buffer = NULL;
	vid.recalc_refdef = true;

	vid.conwidth  = vid.width ;
	vid.conheight = vid.height;
	vid.conrowbytes = vid.rowbytes;
	vid.conbuffer = vid.buffer;

	vid.colormap = host_colormap;
	vid.fullbright = 256 - LittleLong (*((int *)vid.colormap + 2048));

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		Sys_Error("Could not initialize SDL video.");

	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 1 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );

	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24);

	g_sdl_gl.window =
		SDL_CreateWindow(
			"Quake",
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			vid.width, vid.height,
			SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN );

	if (!g_sdl_gl.window)
		Sys_Error("Can not create window.");

	g_sdl_gl.context = SDL_GL_CreateContext( g_sdl_gl.window );

	if (!g_sdl_gl.context)
		Sys_Error("Can not get window context.");

	SDL_GL_MakeCurrent( g_sdl_gl.window, g_sdl_gl.context );
	SDL_GL_SetSwapInterval(1);

	VID_SetPalette(palette);

	bindTexFunc = glBindTexture;
	
	glClearColor (1,0,0,0);
	glCullFace(GL_FRONT);
	glEnable(GL_TEXTURE_2D);

	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.666);

	glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
	glShadeModel (GL_FLAT);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}

void	VID_Shutdown (void)
{
	SDL_GL_DeleteContext( g_sdl_gl.context );
	SDL_DestroyWindow( g_sdl_gl.window );
}

void VID_LockBuffer (void)
{
}

void VID_UnlockBuffer (void)
{
}

void	VID_SetPalette (unsigned char *palette)
{
	byte	*pal;
	unsigned r,g,b;
	unsigned v;
	int     r1,g1,b1;
	int		j,k,l,m;
	unsigned short i;
	unsigned	*table;
	FILE *f;
	char s[255];
	HWND hDlg, hProgress;
	float gamma;

//
// 8 8 8 encoding
//
	pal = palette;
	table = d_8to24table;
	for (i=0 ; i<256 ; i++)
	{
		r = pal[0];
		g = pal[1];
		b = pal[2];
		pal += 3;
		
//		v = (255<<24) + (r<<16) + (g<<8) + (b<<0);
//		v = (255<<0) + (r<<8) + (g<<16) + (b<<24);
		v = (255<<24) + (r<<0) + (g<<8) + (b<<16);
		*table++ = v;
	}
	d_8to24table[255] &= 0xffffff;	// 255 is transparent

	// JACK: 3D distance calcs - k is last closest, l is the distance.
	// FIXME: Precalculate this and cache to disk.
	for (i=0; i < (1<<15); i++) {
		/* Maps
			000000000000000
			000000000011111 = Red  = 0x1F
			000001111100000 = Blue = 0x03E0
			111110000000000 = Grn  = 0x7C00
		*/
		r = ((i & 0x1F) << 3)+4;
		g = ((i & 0x03E0) >> 2)+4;
		b = ((i & 0x7C00) >> 7)+4;
		pal = (unsigned char *)d_8to24table;
		for (v=0,k=0,l=10000*10000; v<256; v++,pal+=4) {
			r1 = r-pal[0];
			g1 = g-pal[1];
			b1 = b-pal[2];
			j = (r1*r1)+(g1*g1)+(b1*b1);
			if (j<l) {
				k=v;
				l=j;
			}
		}
		d_15to8table[i]=k;
	}
}

void	VID_ShiftPalette (unsigned char *palette)
{
	// panzer - stub, do something with it later
}

void VID_HandlePause (qboolean pause)
{
	// panzer - stub, do something with it later
}

qboolean VID_Is8bit(void)
{
	return false;
}

void GL_BeginRendering (int *x, int *y, int *width, int *height)
{
	*x = 0;
	*y = 0;
	*width  = vid.width ;
	*height = vid.height;
}

void GL_EndRendering (void)
{
	SDL_GL_SwapWindow( g_sdl_gl.window );
}
