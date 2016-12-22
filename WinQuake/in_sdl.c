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

#include <SDL.h>

#include "quakedef.h"

static qboolean g_mouse_captured = false;
static int g_mouse_dx = 0, g_mouse_dy = 0;
static int g_prev_mouse_dx = 0, g_prev_mouse_dy = 0;

extern cvar_t sensitivity;
static cvar_t	m_filter = {"m_filter","0", true};

static void CaptureMouse( qboolean need_capture )
{
	if( g_mouse_captured == need_capture )
		return;

	g_mouse_captured = need_capture;

	if( need_capture )
	{
		SDL_SetRelativeMouseMode(true);
		SDL_ShowCursor(false);
		g_mouse_dx = 0;
		g_mouse_dy = 0;
		g_prev_mouse_dx = 0;
		g_prev_mouse_dy = 0;
	}
	else
	{
		SDL_SetRelativeMouseMode(false);
		SDL_ShowCursor(true);
	}
}

static int TranslateSDLKey( SDL_Scancode scan_code )
{
	switch(scan_code)
	{
	case SDL_SCANCODE_LEFT:		return K_LEFTARROW;
	case SDL_SCANCODE_RIGHT:	return K_RIGHTARROW;
	case SDL_SCANCODE_UP:		return K_UPARROW;
	case SDL_SCANCODE_DOWN:		return K_DOWNARROW;

	case SDL_SCANCODE_ESCAPE:	return K_ESCAPE;
	case SDL_SCANCODE_RETURN:	return K_ENTER;
	case SDL_SCANCODE_TAB:		return K_TAB;

	case SDL_SCANCODE_F1:		return K_F1;
	case SDL_SCANCODE_F2:		return K_F2;
	case SDL_SCANCODE_F3:		return K_F3;
	case SDL_SCANCODE_F4:		return K_F4;
	case SDL_SCANCODE_F5:		return K_F5;
	case SDL_SCANCODE_F6:		return K_F6;
	case SDL_SCANCODE_F7:		return K_F7;
	case SDL_SCANCODE_F8:		return K_F8;
	case SDL_SCANCODE_F9:		return K_F9;
	case SDL_SCANCODE_F10:		return K_F10;
	case SDL_SCANCODE_F11:		return K_F11;
	case SDL_SCANCODE_F12:		return K_F12;

	case SDL_SCANCODE_SPACE:	 return K_SPACE;
	case SDL_SCANCODE_BACKSPACE: return K_BACKSPACE;

	case SDL_SCANCODE_PAUSE:	return K_PAUSE;

	case SDL_SCANCODE_LSHIFT:
	case SDL_SCANCODE_RSHIFT:	return K_SHIFT;

	case SDL_SCANCODE_LCTRL:
	case SDL_SCANCODE_RCTRL:	return K_CTRL;

	case SDL_SCANCODE_LALT:
	case SDL_SCANCODE_RALT:		return K_ALT;

	case SDL_SCANCODE_INSERT:	return K_INS;
	case SDL_SCANCODE_DELETE:	return K_DEL;

	case SDL_SCANCODE_PAGEUP:	return K_PGUP;
	case SDL_SCANCODE_PAGEDOWN:	return K_PGDN;
	
	case SDL_SCANCODE_HOME:		return K_HOME;
	case SDL_SCANCODE_END:		return K_END;

	case SDL_SCANCODE_GRAVE:	return '`';

	case SDL_SCANCODE_0:		return '0';
	case SDL_SCANCODE_MINUS:	return '-';
	case SDL_SCANCODE_EQUALS:	return '=';
	case SDL_SCANCODE_LEFTBRACKET:	return '[';
	case SDL_SCANCODE_RIGHTBRACKET:	return ']';
	case SDL_SCANCODE_BACKSLASH:	return '\\';
	case SDL_SCANCODE_SEMICOLON:	return ';';
	case SDL_SCANCODE_APOSTROPHE:	return '\'';
	case SDL_SCANCODE_COMMA:		return ',';
	case SDL_SCANCODE_PERIOD:		return '.';
	case SDL_SCANCODE_SLASH:		return '/';

	default:
		if (scan_code >= SDL_SCANCODE_A && scan_code <= SDL_SCANCODE_Z)
			return scan_code - SDL_SCANCODE_A + 'a';
		if (scan_code >= SDL_SCANCODE_1 && scan_code <= SDL_SCANCODE_9)
			return scan_code - SDL_SCANCODE_1 + '1';

	// Left unstranslated
	return scan_code;
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
		Key_Event( TranslateSDLKey( event->key.keysym.scancode ), true );
		break;

	case SDL_KEYUP:
		Key_Event( TranslateSDLKey( event->key.keysym.scancode ), false );
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
		g_mouse_dx += event->motion.xrel;
		g_mouse_dy += event->motion.yrel;
		break;

	case SDL_WINDOWEVENT_ENTER:
		CaptureMouse(true);
		break;

	case SDL_WINDOWEVENT_LEAVE:
		CaptureMouse(false);
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
	Cvar_RegisterVariable (&m_filter);

	CaptureMouse(true);
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
	// cmd not used - because we do not use mouse for moving, only for rotation.

	if( !cl.paused && key_dest == key_game )
	{
		float dx, dy;

		if( m_filter.value != 0.0 )
		{
			dx = ( g_mouse_dx + g_prev_mouse_dx ) * 0.5;
			dy = ( g_mouse_dy + g_prev_mouse_dy ) * 0.5;
		}
		else
		{
			dx = g_mouse_dx;
			dy = g_mouse_dy;
		}

		V_StopPitchDrift();

		cl.viewangles[YAW  ] -= 0.015 * sensitivity.value * dx;
		cl.viewangles[PITCH] += 0.015 * sensitivity.value * dy;

		if( cl.viewangles[PITCH] > +90 ) cl.viewangles[PITCH] = +90;
		if( cl.viewangles[PITCH] < -90 ) cl.viewangles[PITCH] = -90;

		CaptureMouse(true);
	}
	else
		CaptureMouse(false);

	g_prev_mouse_dx = g_mouse_dx;
	g_prev_mouse_dy = g_mouse_dy;
	g_mouse_dx = 0;
	g_mouse_dy = 0;
}

void IN_ClearStates (void)
{
	g_mouse_dx = 0;
	g_mouse_dy = 0;
	g_prev_mouse_dx = 0;
	g_prev_mouse_dy = 0;
}

void IN_Accumulate (void)
{
	// panzer - stub, do something with it later
}
