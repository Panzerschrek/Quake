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

#include "vid_common.h"

#include "quakedef.h"
#include "winquake.h"

#define FPS_CALC_INTERVAL 1.0

static struct
{
	int fps_to_draw;
	int frames_since_last_swap;
	double last_swap_time;
} g_fps;

static cvar_t vid_drawfps = { "vid_drawfps", "0", true };

int VID_SelectVideoMode( int display, int width, int height, SDL_DisplayMode* selected )
{
	int		i;
	int		display_count;
	int		mode_count;

	display_count = SDL_GetNumVideoDisplays();

	if (display >= display_count )
		display = 0;

	mode_count = SDL_GetNumDisplayModes( display );

	for (i = 0; i < mode_count; i++)
	{
		SDL_GetDisplayMode( display, i, selected );

		if (selected->w == width && selected->h == height &&
			SDL_BYTESPERPIXEL(selected->format) == 4)
			return 1;
	}

	return 0;
}

int VID_SwitchToMode( SDL_Window* window, SDL_DisplayMode* mode )
{
	int result = SDL_SetWindowDisplayMode( window, mode );

	Sys_Printf( "SwVID_SwitchToMode %s %dx%dx%d %dHz\n",
		result ? "warning, could not set display mode" : "set display mode",
		mode->w, mode->h,
		SDL_BITSPERPIXEL(mode->format), mode->refresh_rate);

	return result == 0;

}

void VID_FPSInit(void)
{
	Cvar_RegisterVariable( &vid_drawfps );

	g_fps.fps_to_draw = 0;
	g_fps.frames_since_last_swap = 0;
	g_fps.last_swap_time = Sys_FloatTime();
}

void VID_FPSUpdate(void)
{
	double		current_time;
	double		time_delta;
	char		str[64];

	if (!vid_drawfps.value)
		return;

	g_fps.frames_since_last_swap ++;

	current_time = Sys_FloatTime();
	time_delta = current_time - g_fps.last_swap_time;

	if (time_delta > FPS_CALC_INTERVAL)
	{
		g_fps.fps_to_draw = (int)( ((double)g_fps.frames_since_last_swap + 0.5) / time_delta );
		if (g_fps.fps_to_draw > 999 )
			g_fps.fps_to_draw = 999;

		g_fps.last_swap_time = current_time;
		g_fps.frames_since_last_swap = 0;
	}

	sprintf( str, "fps: %03d", g_fps.fps_to_draw );
	Draw_String( vid.width - 8 * 10, 8, str );
}
