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