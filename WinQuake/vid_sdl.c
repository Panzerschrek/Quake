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

#include "vid_common.h"

#define MAX_SCALER 32

typedef union
{
	byte components[4];
	int pix;
} screen_pixel_t;

enum
{
	COMPONENT_R,
	COMPONENT_G,
	COMPONENT_B,
	COMPONENT_A,
};


// Some subsystem needs it
modestate_t	modestate = MS_UNINIT;
cvar_t		_windowed_mouse = {"_windowed_mouse","0", true};

static byte		*g_vid_surfcache;
static int		g_vid_surfcachesize;

unsigned short	d_8to16table[256];
unsigned		d_8to24table[256];

static screen_pixel_t g_palette[256];

struct
{
	SDL_Window*		window;
	SDL_Surface*	window_surface;

	int				scaler;
	qboolean		fullscreen;

	struct
	{
		int component_index[4];
	} pixel_format;

} g_sdl;

static qboolean g_initialized = false;

cvar_t	vid_width  = { "vid_width" , "640", true };
cvar_t	vid_height = { "vid_height", "480", true };
cvar_t	vid_scaler = { "vid_scaler", "1", true };
cvar_t	vid_display = { "vid_display", "0", true };
cvar_t	vid_fullscreen = { "vid_fullscreen", "0", true };

static void MenuDrawFn(void)
{
}

static void MenuKeyFn(int key)
{
	if (key == K_ESCAPE)
	{
		S_LocalSound ("misc/menu1.wav");
		M_Menu_Options_f ();
	}
}

static void GetPixelComponentsOrder( const SDL_PixelFormat*	pixel_format )
{
	if (pixel_format->BytesPerPixel != 4)
		Sys_Error("Invalid pixel format. Requred 4 bytes per pixel, actual - %d\n", pixel_format->BytesPerPixel);

		 if (pixel_format->Rmask ==       0xFF) g_sdl.pixel_format.component_index[ COMPONENT_R ] = 0;
	else if (pixel_format->Rmask ==     0xFF00) g_sdl.pixel_format.component_index[ COMPONENT_R ] = 1;
	else if (pixel_format->Rmask ==   0xFF0000) g_sdl.pixel_format.component_index[ COMPONENT_R ] = 2;
	else if (pixel_format->Rmask == 0xFF000000) g_sdl.pixel_format.component_index[ COMPONENT_R ] = 3;
	else g_sdl.pixel_format.component_index[ COMPONENT_R ] = -1;
		 if (pixel_format->Gmask ==       0xFF) g_sdl.pixel_format.component_index[ COMPONENT_G ] = 0;
	else if (pixel_format->Gmask ==     0xFF00) g_sdl.pixel_format.component_index[ COMPONENT_G ] = 1;
	else if (pixel_format->Gmask ==   0xFF0000) g_sdl.pixel_format.component_index[ COMPONENT_G ] = 2;
	else if (pixel_format->Gmask == 0xFF000000) g_sdl.pixel_format.component_index[ COMPONENT_G ] = 3;
	else g_sdl.pixel_format.component_index[ COMPONENT_G ] = -1;
		 if (pixel_format->Bmask ==       0xFF) g_sdl.pixel_format.component_index[ COMPONENT_B ] = 0;
	else if (pixel_format->Bmask ==     0xFF00) g_sdl.pixel_format.component_index[ COMPONENT_B ] = 1;
	else if (pixel_format->Bmask ==   0xFF0000) g_sdl.pixel_format.component_index[ COMPONENT_B ] = 2;
	else if (pixel_format->Bmask == 0xFF000000) g_sdl.pixel_format.component_index[ COMPONENT_B ] = 3;
	else g_sdl.pixel_format.component_index[ COMPONENT_B ] = -1;
		 if (pixel_format->Amask ==       0xFF) g_sdl.pixel_format.component_index[ COMPONENT_A ] = 0;
	else if (pixel_format->Amask ==     0xFF00) g_sdl.pixel_format.component_index[ COMPONENT_A ] = 1;
	else if (pixel_format->Amask ==   0xFF0000) g_sdl.pixel_format.component_index[ COMPONENT_A ] = 2;
	else if (pixel_format->Amask == 0xFF000000) g_sdl.pixel_format.component_index[ COMPONENT_A ] = 3;
	else g_sdl.pixel_format.component_index[ COMPONENT_A ] = 3;

	if (g_sdl.pixel_format.component_index[ COMPONENT_R ] == -1 ||
		g_sdl.pixel_format.component_index[ COMPONENT_G ] == -1 ||
		g_sdl.pixel_format.component_index[ COMPONENT_B ] == -1 )
		Sys_Error("Invalid pixel format. Unknown color component order");
}

static void UpdateMode (unsigned char *palette)
{
	SDL_PixelFormat*	pixel_format;
	SDL_DisplayMode		display_mode;
	int					width, height;
	int					system_width, system_height;

	g_sdl.fullscreen = false;
	{
		system_width  = (int)vid_width .value;
		system_height = (int)vid_height.value;

		g_sdl.scaler = vid_scaler.value;
		if (g_sdl.scaler > MAX_SCALER)
			g_sdl.scaler = MAX_SCALER;
		if (g_sdl.scaler < 1 )
			g_sdl.scaler = 1;

		if (system_width  < MIN_WIDTH )
			system_width = MIN_WIDTH;
		if (system_height < MIN_HEIGHT)
			system_height = MIN_HEIGHT;

		width  = system_width  / g_sdl.scaler;
		height = system_height / g_sdl.scaler;

		// Decrease scaler, if it is too big
		while (width < MIN_WIDTH || height < MIN_HEIGHT)
		{
			g_sdl.scaler--;
			width  = system_width  / g_sdl.scaler;
			height = system_height / g_sdl.scaler;
		}

		// Increase scaler, if it is too small (for very wide displays)
		while (width > MAXWIDTH || height > MAXHEIGHT)
		{
			g_sdl.scaler++;
			width  = system_width  / g_sdl.scaler;
			height = system_height / g_sdl.scaler;
		}
	}

	if ( SDL_InitSubSystem(SDL_INIT_VIDEO) < 0 )
		Sys_Error("Could not initialize SDL video.");

	if (vid_fullscreen.value)
	{
		int ok =
			VID_SelectVideoMode(
				vid_display.value,
				system_width, system_height,
				&display_mode );

		if ( ok )
			g_sdl.fullscreen = true;
		else
			Cvar_Set( vid_fullscreen.name, "0" );
	}

	g_sdl.window =
		SDL_CreateWindow(
			"Quake",
			SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			system_width, system_height,
			SDL_WINDOW_SHOWN | (g_sdl.fullscreen ? SDL_WINDOW_FULLSCREEN : 0) );

	if (!g_sdl.window)
		Sys_Error("Can not create window.");

	if (g_sdl.fullscreen)
	{
		int ok = VID_SwitchToMode( g_sdl.window, &display_mode );

		// reset fullscreen settings, if we have problems
		g_sdl.fullscreen = ok;
		Cvar_Set( vid_fullscreen.name, ok ? "1" : "0" );
	}

	g_sdl.window_surface = SDL_GetWindowSurface( g_sdl.window );

	pixel_format = g_sdl.window_surface->format;
	GetPixelComponentsOrder( pixel_format );

	vid.width  = width ;
	vid.height = height;
	vid.rowbytes = vid.width;
	vid.numpages = 1;
	vid.maxwarpwidth  = WARP_WIDTH ;
	vid.maxwarpheight = WARP_HEIGHT;
	vid.aspect = 1.0f;

	vid.buffer = malloc( vid.width * vid.height );
	vid.recalc_refdef = true;

	vid.conwidth  = vid.width ;
	vid.conheight = vid.height;
	vid.conrowbytes = vid.rowbytes;
	vid.conbuffer = vid.buffer;

	vid.colormap = host_colormap;
	vid.fullbright = 256 - LittleLong (*((int *)vid.colormap + 2048));

	d_pzbuffer = malloc( vid.width * vid.height * sizeof(short) );

	vid_menudrawfn = &MenuDrawFn;
	vid_menukeyfn = &MenuKeyFn;

	VID_SetPalette(palette);

	g_vid_surfcachesize = D_SurfaceCacheForRes (vid.width, vid.height);
	g_vid_surfcache = malloc( g_vid_surfcachesize );
	D_InitCaches (g_vid_surfcache, g_vid_surfcachesize);

	g_initialized = true;
}

static void RestartCommand(void)
{
	VID_Shutdown();
	UpdateMode(host_basepal);
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
		g_palette[i].components[ g_sdl.pixel_format.component_index[COMPONENT_R] ] = palette[i*3  ];
		g_palette[i].components[ g_sdl.pixel_format.component_index[COMPONENT_G] ] = palette[i*3+1];
		g_palette[i].components[ g_sdl.pixel_format.component_index[COMPONENT_B] ] = palette[i*3+2];
		g_palette[i].components[3] = 255;
	}
}

void	VID_ShiftPalette (unsigned char *palette)
{
	// panzer - stub, do something with it later
}

void	VID_Init (unsigned char *palette)
{
	Cvar_RegisterVariable( &vid_width  );
	Cvar_RegisterVariable( &vid_height );
	Cvar_RegisterVariable( &vid_scaler );
	Cvar_RegisterVariable( &vid_display );
	Cvar_RegisterVariable( &vid_fullscreen );
	Cmd_AddCommand( "vid_restart", RestartCommand );

	g_sdl.fullscreen = false;

	UpdateMode(palette);

	VID_FPSInit();
}

void	VID_Shutdown (void)
{
	if (!g_initialized)
		return;

	D_FlushCaches();

	free( vid.buffer );
	free( g_vid_surfcache );
	free( d_pzbuffer );

	SDL_DestroyWindow( g_sdl.window );

	g_initialized = true;
}

void	VID_Update (vrect_t *rects)
{
	screen_pixel_t*	dst;
	int				dst_rowbytes;
	byte*			src;
	screen_pixel_t	pix;
	int				src_x, src_y, p_x, p_y;
	int				p_left_x, p_left_y;
	int				must_lock;

	VID_FPSUpdate();

	must_lock = SDL_MUSTLOCK( g_sdl.window_surface );

	if (must_lock)
		SDL_LockSurface( g_sdl.window_surface );

	dst_rowbytes = g_sdl.window_surface->pitch;

	if (g_sdl.scaler == 1)
	{
		for (p_y = 0; p_y < vid.height; p_y++)
		{
			dst = (screen_pixel_t*)
				( ((byte*)g_sdl.window_surface->pixels) + p_y * dst_rowbytes );

			src = vid.buffer + vid.width * p_y;

			for (p_x = 0; p_x < vid.width; p_x++)
				dst[ p_x ].pix = g_palette[ src[p_x] ].pix;
		}
	}
	else
	{
		p_left_x = g_sdl.window_surface->w - vid.width  * g_sdl.scaler;
		p_left_y = g_sdl.window_surface->h - vid.height * g_sdl.scaler;

		src = vid.buffer;

		// for source lines
		for (src_y = 0; src_y < vid.height; src_y++)
		{
			src = vid.buffer + vid.width * src_y;

			for (p_y = 0; p_y < g_sdl.scaler; p_y++)
			{
				dst = (screen_pixel_t*)
					( ((byte*)g_sdl.window_surface->pixels) + (src_y * g_sdl.scaler + p_y ) * dst_rowbytes );

				// Unwind loop for some scales
				if (g_sdl.scaler == 2)
					for (src_x = 0; src_x < vid.width; src_x++, dst+= 2)
						dst[0] = dst[1] = g_palette[ src[src_x] ];

				else if (g_sdl.scaler == 3)
					for (src_x = 0; src_x < vid.width; src_x++, dst+= 3)
						dst[0] = dst[1] = dst[2] = g_palette[ src[src_x] ];

				else if (g_sdl.scaler == 4)
					for (src_x = 0; src_x < vid.width; src_x++, dst+= 4)
						dst[0] = dst[1] = dst[2] = dst[3] = g_palette[ src[src_x] ];

				else if (g_sdl.scaler == 5)
					for (src_x = 0; src_x < vid.width; src_x++, dst+= 5)
						dst[0] = dst[1] = dst[2] = dst[3] = dst[4] = g_palette[ src[src_x] ];

				else
					for (src_x = 0; src_x < vid.width; src_x++)
					{
						pix = g_palette[ src[src_x] ];
						for (p_x = 0; p_x < g_sdl.scaler; p_x++, dst++)
							*dst = pix;
					}

				// fill left pixels near screen edge
				pix = g_palette[ src[ vid.width - 1 ] ];
				for (p_x = 0; p_x < p_left_x; p_x++, dst++ )
					*dst = pix;
			}
		}

		// Copy last effective line from framebuffer to left framebuffer lines
		for (p_y = 0; p_y < p_left_y; p_y++)
		{
			memcpy(
				((byte*)g_sdl.window_surface->pixels) + (vid.height * g_sdl.scaler + p_y ) * dst_rowbytes,
				((byte*)g_sdl.window_surface->pixels) + (vid.height * g_sdl.scaler - 1   ) * dst_rowbytes,
				dst_rowbytes );
		}
	}

	if (must_lock)
		SDL_UnlockSurface( g_sdl.window_surface );

	SDL_UpdateWindowSurface( g_sdl.window );
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