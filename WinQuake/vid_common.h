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

#ifndef __VID_COMMON__
#define __VID_COMMON__

#include <SDL.h>
#include <SDL_video.h>

// Methods returns 1 if all ok
int VID_SelectVideoMode( int display, int width, int height, SDL_DisplayMode* selected );
int VID_SwitchToMode( SDL_Window* window, SDL_DisplayMode* mode );

void VID_UpdateGammaImpl( SDL_Window* window );
void VID_SaveSystemGamma( SDL_Window* window );
void VID_RestoreSystemGamma( SDL_Window* window );

void VID_FPSInit(void);
void VID_FPSUpdate(void);

#endif//__VID_COMMON__