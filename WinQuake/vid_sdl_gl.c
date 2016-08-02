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
#include "vid_common.h"

// gl functions pointers
#define PROCESS_GL_FUNC( type, name ) type name
#include "gl_funcs_list.h"
#undef PROCESS_GL_FUNC


cvar_t		gl_texanisotropy = { "gl_texanisotropy", "8", true };
int			gl_max_texanisotropy = 0;

// Some subsystem needs it
modestate_t	modestate = MS_UNINIT;
cvar_t		_windowed_mouse = {"_windowed_mouse","0", true};

unsigned		d_8to24table[256];

float gldepthmin = -1.0f, gldepthmax = 1.0f;

struct
{
	SDL_Window* window;
	SDL_GLContext* context;

} g_sdl_gl;

static void GetGLFuncs(void)
{
	#define PROCESS_GL_FUNC( type, name )\
		name = (type) SDL_GL_GetProcAddress( #name );\
		if (!name) \
			Sys_Printf( "Warning, function \""#name"\" not found\n" );

	#include "gl_funcs_list.h"
	#undef PROCESS_GL_FUNC
}

static void CheckGLExtensions(void)
{
	const GLubyte *ext_str;

	ext_str = glGetString( GL_EXTENSIONS );

	if ( strstr( (const char*)ext_str, "GL_EXT_texture_filter_anisotropic" ) != NULL )
		glGetIntegerv( GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &gl_max_texanisotropy );
	else
	{
		Con_Print("Textures anisotropy not supported\n");
		gl_max_texanisotropy = 0;
	}
}

static void SetupGLState(void)
{
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
}

void	VID_Init (unsigned char *palette)
{
	SDL_DisplayMode	display_mode;
	int			param_width ;
	int			param_height;
	qboolean	fullscreen;
	int			param_display;
	int			display;
	int			param_msaa;
	int			msaa_samples;

	Cvar_RegisterVariable( &gl_texanisotropy );

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

	fullscreen = COM_CheckParm( "-fullscreen" ) != 0;

	param_display = COM_CheckParm( "-display" );
	if (param_display != 0)
		display = Q_atoi( com_argv[ param_display + 1 ] );
	else
		display = 0;

	param_msaa = COM_CheckParm("-msaa");
	if (param_msaa != 0)
		msaa_samples = Q_atoi( com_argv[ param_msaa + 1 ] );
	else
		msaa_samples = 0;

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

	if (fullscreen)
		fullscreen = VID_SelectVideoMode( display, vid.width, vid.height, &display_mode );

	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 2 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );

	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24);

	if (msaa_samples != 0)
	{
		if (msaa_samples > 16) msaa_samples = 16;
		if (msaa_samples <  2) msaa_samples =  2;

		SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 1 );
		SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, msaa_samples );
	}

	g_sdl_gl.window =
		SDL_CreateWindow(
			"Quake",
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			vid.width, vid.height,
			SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | ( fullscreen ? SDL_WINDOW_FULLSCREEN : 0 ) );

	if (!g_sdl_gl.window)
		Sys_Error("Can not create window.");

	if (fullscreen)
		fullscreen = VID_SwitchToMode( g_sdl_gl.window, &display_mode );

	g_sdl_gl.context = SDL_GL_CreateContext( g_sdl_gl.window );

	if (!g_sdl_gl.context)
		Sys_Error("Can not get window context.");

	SDL_GL_MakeCurrent( g_sdl_gl.window, g_sdl_gl.context );
	SDL_GL_SetSwapInterval(1);

	VID_SetPalette(palette);

	GetGLFuncs();

	CheckGLExtensions();
	
	SetupGLState();

	VID_FPSInit();
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
	unsigned short i;
	unsigned	*table;

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

		d_8to24table[i] = (255<<24) + (r<<0) + (g<<8) + (b<<16);
	}
	d_8to24table[255] = 0x00000000;	// 255 is transparent - make in black and transparent
}

void	VID_ShiftPalette (unsigned char *palette)
{
	// panzer - stub, do something with it later
}

void VID_HandlePause (qboolean pause)
{
	// panzer - stub, do something with it later
}

void GL_EndRendering (void)
{
	VID_FPSUpdate();
	SDL_GL_SwapWindow( g_sdl_gl.window );
}
