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

#include <SDL.h>

#include "quakedef.h"

static int TranslateSDLKey( int key )
{
	switch(key)
	{
	case SDLK_LEFT:		return K_LEFTARROW;
	case SDLK_RIGHT:	return K_RIGHTARROW;
	case SDLK_UP:		return K_UPARROW;
	case SDLK_DOWN:		return K_DOWNARROW;

	case SDLK_ESCAPE:	return K_ESCAPE;
	case SDLK_RETURN:	return K_ENTER;
	case SDLK_TAB:		return K_TAB;

	case SDLK_F1:		return K_F1;
	case SDLK_F2:		return K_F2;
	case SDLK_F3:		return K_F3;
	case SDLK_F4:		return K_F4;
	case SDLK_F5:		return K_F5;
	case SDLK_F6:		return K_F6;
	case SDLK_F7:		return K_F7;
	case SDLK_F8:		return K_F8;
	case SDLK_F9:		return K_F9;
	case SDLK_F10:		return K_F10;
	case SDLK_F11:		return K_F11;
	case SDLK_F12:		return K_F12;

	case SDLK_BACKSPACE: return K_BACKSPACE;

	case SDLK_PAUSE:	return K_PAUSE;

	case SDLK_LSHIFT:
	case SDLK_RSHIFT:	return K_SHIFT;

	case SDLK_LCTRL:
	case SDLK_RCTRL:	return K_CTRL;

	case SDLK_LALT:
	case SDLK_RALT:		return K_ALT;

	case SDLK_INSERT:	return K_INS;
	case SDLK_DELETE:	return K_DEL;

	case SDLK_PAGEUP:	return K_PGUP;
	case SDLK_PAGEDOWN:	return K_PGDN;
	
	case SDLK_HOME:		return K_HOME;
	case SDLK_END:		return K_END;

	default:
		if (key >= SDLK_a && key <= SDLK_z )
			return key - SDLK_a + 'a';
		if ( key >= SDLK_0 && key <= SDLK_9 )
			return key - SDLK_0 + '0';

	// Left unstranslated
	return key;
	}
}

static int TranslateSDLMouseButton(int button)
{
	switch(button)
	{
	case SDL_BUTTON_LEFT:	return K_MOUSE1;
	case SDL_BUTTON_RIGHT:	return K_MOUSE2;
	case SDL_BUTTON_MIDDLE:	return K_MOUSE3;
	default:				return K_MOUSE1;
	}
}

static void ProcessSDLEvent( const SDL_Event* event )
{
	int wheel_event;

	switch (event->type)
	{
	case SDL_KEYDOWN:
		Key_Event( TranslateSDLKey( event->key.keysym.sym ), true );
		break;

	case SDL_KEYUP:
		Key_Event( TranslateSDLKey( event->key.keysym.sym ), false );
		break;

	case SDL_MOUSEBUTTONDOWN:
		Key_Event( TranslateSDLMouseButton( event->button.button ), true );
		break;

	case SDL_MOUSEBUTTONUP:
		Key_Event( TranslateSDLMouseButton( event->button.button ), false );
		break;

	case SDL_MOUSEWHEEL:
		wheel_event = event->wheel.y > 0 ? K_MWHEELUP : K_MWHEELDOWN;
		Key_Event( wheel_event, true );
		Key_Event( wheel_event, false );
		break;

	case SDL_MOUSEMOTION:
		break;

	case SDL_WINDOWEVENT_ENTER:
		break;

	case SDL_WINDOWEVENT_LEAVE:
		break;

	case SDL_WINDOWEVENT_CLOSE:
	case SDL_QUIT:
		Sys_Quit();
		break;

	default:
		break;
	}
}

void IN_Init (void)
{
}

void IN_Shutdown (void)
{
}

void IN_Commands (void)
{
	SDL_Event event;

	while( SDL_PollEvent(&event) )
		ProcessSDLEvent( &event );
}

void IN_Move (usercmd_t *cmd)
{
	// panzer - stub, do something with it later
}

void IN_ClearStates (void)
{
	// panzer - stub, do something with it later
}

void IN_Accumulate (void)
{
	// panzer - stub, do something with it later
}
