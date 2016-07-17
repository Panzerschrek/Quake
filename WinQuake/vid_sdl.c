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
#include <stdlib.h>

#include <SDL.h>
#include <SDL_video.h>

#include "quakedef.h"
#include "winquake.h"
#include "d_local.h"

#include <SDL_syswm.h>

typedef union
{
	byte components[4];
	int pix;
} screen_pixel_t;

// Some subsystem needs it
HWND		mainwindow;
modestate_t	modestate = MS_UNINIT;
qboolean			DDActive = 0;
cvar_t		_windowed_mouse = {"_windowed_mouse","0", true};

static byte		*g_vid_surfcache;
static int		g_vid_surfcachesize;

unsigned short	d_8to16table[256];
unsigned	d_8to24table[256];

static screen_pixel_t g_palette[256];

struct
{
	SDL_Window*		window;
	SDL_Surface*	window_surface;
} g_sdl;


static void MenuDrawFn(void)
{
}

static void MenuKeyFn(int x)
{
}

// lock/unlock used in some places

void VID_LockBuffer (void)
{
}

void VID_UnlockBuffer (void)
{
}


void	VID_SetPalette (unsigned char *palette)
{
	int	i;

	for (i = 0; i < 256; i++)
	{
		g_palette[i].components[0] = palette[ i*3 + 2 ];
		g_palette[i].components[1] = palette[ i*3 + 1 ];
		g_palette[i].components[2] = palette[ i*3 + 0 ];
		g_palette[i].components[3] = 0;
	}
}

void	VID_ShiftPalette (unsigned char *palette)
{
	// panzer - stub, do something with it later
}

void	VID_Init (unsigned char *palette)
{
	if ( SDL_InitSubSystem(SDL_INIT_VIDEO) < 0 )
		Sys_Error("Could not initialize SDL video.");

	VID_SetPalette(palette);

	vid.width  = 640;
	vid.height = 480;
	vid.rowbytes = vid.width;
	vid.numpages = 1;
	vid.maxwarpwidth = WARP_WIDTH;
	vid.maxwarpheight = WARP_HEIGHT;
	vid.aspect = ((float)vid.height / (float)vid.width) * (320.0 / 240.0);

	vid.buffer = malloc( vid.width * vid.height );

	vid.conwidth  = 320;
	vid.conheight = vid.height;
	vid.conrowbytes = vid.conwidth;
	vid.conbuffer = malloc( vid.conwidth * vid.conheight );

	vid.colormap = host_colormap;
	vid.fullbright = 256 - LittleLong (*((int *)vid.colormap + 2048));

	d_pzbuffer = malloc( vid.width * vid.height * sizeof(short) );

	g_sdl.window =
		SDL_CreateWindow(
			"Quake",
			SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			640, 480,
			SDL_WINDOW_SHOWN | (SDL_WINDOW_FULLSCREEN & 0) );

	if (!g_sdl.window)
		Sys_Error("Can not create window.");

	g_sdl.window_surface = SDL_GetWindowSurface( g_sdl.window );

	{
		// Panzer - get native window handle for sound system initialization
		// TODO - remove this stuff, when we switch to sdl sound
		SDL_SysWMinfo wmInfo;
		SDL_VERSION(&wmInfo.version);
		SDL_GetWindowWMInfo (g_sdl.window, &wmInfo);

		mainwindow = wmInfo.info.win.window;
	}
	S_Init ();

	vid_menudrawfn = &MenuDrawFn;
	vid_menukeyfn = &MenuKeyFn;

	g_vid_surfcachesize = D_SurfaceCacheForRes (vid.width, vid.height);
	g_vid_surfcache = malloc( g_vid_surfcachesize );
	D_InitCaches (g_vid_surfcache, g_vid_surfcachesize);
}

void	VID_Shutdown (void)
{
	SDL_DestroyWindow( g_sdl.window );

	free( vid.buffer );
	free( vid.conbuffer );
	free( g_vid_surfcache );
	free( d_pzbuffer );
}

void	VID_Update (vrect_t *rects)
{
	int i;
	screen_pixel_t* p;

	p = g_sdl.window_surface->pixels;
	for (i = 0; i < vid.width * vid.height; i++)
	{
		p[i].pix = g_palette[ vid.buffer[i] ].pix;
	}

	SDL_UpdateWindowSurface( g_sdl.window );
}

int VID_SetMode (int modenum, unsigned char *palette)
{
	// panzer - stub, do something with it later
	return true;
}

void VID_SetDefaultMode (void)
{
	// panzer - stub, do something with it later
}

void VID_HandlePause (qboolean pause)
{
	// panzer - stub, do something with it later
}

void VID_ForceLockState (int lk)
{
	// panzer - stub, do something with it later
}

int VID_ForceUnlockedAndReturnState (void)
{
	// panzer - stub, do something with it later
	return 0;
}

void D_BeginDirectRect (int x, int y, byte *pbitmap, int width, int height)
{
	// panzer - stub, do something with it later
}

void D_EndDirectRect (int x, int y, int width, int height)
{
	// panzer - stub, do something with it later
}