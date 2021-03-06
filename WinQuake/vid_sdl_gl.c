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

#include "quakedef.h"
#include "winquake.h"

#include <stdio.h>

#include <SDL.h>
#include <SDL_opengl.h>
#include "vid_common.h"

// gl functions pointers
#define PROCESS_GL_FUNC( type, name ) type name
#include "gl_funcs_list.h"
#undef PROCESS_GL_FUNC


cvar_t		gl_texanisotropy = { "gl_texanisotropy", "8", true };
int			gl_max_texanisotropy = 0;

cvar_t	gl_width  = { "gl_width" , "640", false };
cvar_t	gl_height = { "gl_height", "480", false };
cvar_t	gl_display = { "gl_display", "0", false };
cvar_t	gl_fullscreen = { "gl_fullscreen", "0", false };
cvar_t	gl_vsync = { "gl_vsync", "1", false };
cvar_t	gl_msaa = { "gl_msaa", "0", false };

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

static int g_menu_cursor_line = 0;

enum
{
	MENU_LINE_LIGHTING_SCALE,
	MENU_LINE_LIGHTING_GAMMA,
	MENU_LINE_TEXTIRES_ANISOTROPY,
	MENU_LINE_MSAA,
	MENU_LINE_FULLSCREEN,
	MENU_LINE_VSYNC,
	MENU_LINE_WIDTH ,
	MENU_LINE_HEIGHT,
	MENU_LINE_COUNT
};

#define LIGHTING_SCALE_MIN 0.5
#define LIGHTING_SCALE_MAX 2.0

#define LIGHTING_GAMMA_MIN 0.5
#define LIGHTING_GAMMA_MAX 3.0

#define ANISOTROPY_MAX 16.0

#define MSAA_MAX 16.0

static void MenuDrawFn(void)
{
	qpic_t	*p;
	char	str[64];
	int		y0, y, x_print, x_ctrl, x_cursor, y_width, x_width;
	int		cursor_char;

	M_DrawTransPic (16, 4, Draw_CachePic ("gfx/qplaque.lmp") );

	p = Draw_CachePic ("gfx/vidmodes.lmp");
	M_DrawPic ( (320-p->width)/2, 4, p);
	
	y0 = y = 32;
	x_print = 16;
	x_ctrl = 220;
	x_cursor = 200;
	x_width = 210;

	sprintf( str, "    lighting scale %1.1f", gl_lightoverbright.value );
	M_Print (x_print, y, str);
	M_DrawSlider(
		x_ctrl, y,
		( gl_lightoverbright.value - LIGHTING_SCALE_MIN ) / (LIGHTING_SCALE_MAX - LIGHTING_SCALE_MIN) );
	y += 8;

	sprintf( str, "    lighting gamma %1.1f", gl_lightgamma.value );
	M_Print (x_print, y, str);
	M_DrawSlider(
		x_ctrl, y,
		( gl_lightgamma.value - LIGHTING_GAMMA_MIN ) / (LIGHTING_GAMMA_MAX - LIGHTING_GAMMA_MIN) );
	y += 8;

	if (gl_texanisotropy.value == 0.0 )
		strcpy(  str, "        anisotropy off" );
	else
		sprintf( str, "        anisotropy %d", (int) gl_texanisotropy.value );
	M_Print (x_print, y, str);
	M_DrawSlider(
		x_ctrl, y,
		gl_texanisotropy.value / ANISOTROPY_MAX );
	y += 8;

	if (gl_msaa.value < 2 )
		strcpy(  str, "              msaa off" );
	else
		sprintf( str, "              msaa %d", (int) gl_msaa.value );
	M_Print (x_print, y, str);
	M_DrawSlider(
		x_ctrl, y, gl_msaa.value < 2 ? 0.0 : ( log(gl_msaa.value) / log(MSAA_MAX) ) );
	y += 8;

	M_Print (x_print, y, "            fullscreen");
	M_DrawCheckbox (x_ctrl, y, gl_fullscreen.value);
	y += 8;

	M_Print (x_print, y, "                 vsync");
	M_DrawCheckbox (x_ctrl, y, gl_vsync.value);
	y += 8;

	M_DrawTextBox (x_width, y, 5, 1);
	y += 8;
	y_width = y;
	M_Print (x_print, y, "                 width");
	sprintf( str, "%d", (int)gl_width.value );
	M_Print( x_width + 8, y, str );
	y += 8;

	M_DrawTextBox (x_width, y, 5, 1);
	y += 8;
	M_Print (x_print, y, "                height");
	sprintf( str, "%d", (int)gl_height.value );
	M_Print( x_width + 8, y, str );
	y += 8;

	y += 8;
	M_PrintWhite (64, y, "some changes will affect");
	y += 8;
	M_PrintWhite (64, y, "only after game restart");

	cursor_char = 12+((int)(realtime*4)&1);

	if (g_menu_cursor_line == MENU_LINE_WIDTH || g_menu_cursor_line == MENU_LINE_HEIGHT)
	{
		cvar_t* var = g_menu_cursor_line == MENU_LINE_WIDTH ? &gl_width : &gl_height;
		sprintf( str, "%d", (int)var->value );

		M_DrawCharacter (
			x_width + 8 + 8 * strlen(str),
			y_width + (g_menu_cursor_line-MENU_LINE_WIDTH)*16,
			cursor_char - 2);
	}
	else
		M_DrawCharacter (x_cursor, y0 + g_menu_cursor_line*8, cursor_char);
}

static void MenuKeyFn(int key)
{
	int			s;
	qboolean	is_flag_key;

	if (key == K_ESCAPE)
	{
		M_Menu_Options_f ();
		return;
	}

	if (key == K_DOWNARROW)
	{
		S_LocalSound ("misc/menu1.wav");
		g_menu_cursor_line = (g_menu_cursor_line + 1) % MENU_LINE_COUNT;
		return;
	}
	if (key == K_UPARROW)
	{
		S_LocalSound ("misc/menu1.wav");
		g_menu_cursor_line = (g_menu_cursor_line - 1 + MENU_LINE_COUNT) % MENU_LINE_COUNT;
		return;
	}

	is_flag_key = key == K_LEFTARROW || key == K_RIGHTARROW || key == K_ENTER;

	if (g_menu_cursor_line == MENU_LINE_LIGHTING_SCALE)
	{
		s = (int)(gl_lightoverbright.value * 10.0 + 0.01);
		if (key == K_LEFTARROW)
		{
			S_LocalSound ("misc/menu3.wav");
			s--;
		}
		if (key == K_RIGHTARROW)
		{
			S_LocalSound ("misc/menu3.wav");
			s++;
		}

		if (s < LIGHTING_SCALE_MIN * 10.0)
			s = LIGHTING_SCALE_MIN * 10.0;
		if (s > LIGHTING_SCALE_MAX * 10.0)
			s = LIGHTING_SCALE_MAX * 10.0;

		Cvar_SetValue( gl_lightoverbright.name, ((double)s) / 10.0 );
	}
	else if (g_menu_cursor_line == MENU_LINE_LIGHTING_GAMMA)
	{
		s = (int)(gl_lightgamma.value * 10.0 + 0.01);
		if (key == K_LEFTARROW)
		{
			S_LocalSound ("misc/menu3.wav");
			s--;
		}
		if (key == K_RIGHTARROW)
		{
			S_LocalSound ("misc/menu3.wav");
			s++;
		}

		if (s < LIGHTING_GAMMA_MIN * 10.0)
			s = LIGHTING_GAMMA_MIN * 10.0;
		if (s > LIGHTING_GAMMA_MAX * 10.0)
			s = LIGHTING_GAMMA_MAX * 10.0;

		Cvar_SetValue( gl_lightgamma.name, ((double)s) / 10.0 );
	}
	else if (g_menu_cursor_line == MENU_LINE_TEXTIRES_ANISOTROPY)
	{
		s = (int)(gl_texanisotropy.value + 0.01);
		if (key == K_LEFTARROW)
		{
			S_LocalSound ("misc/menu3.wav");
			s--;
		}
		if (key == K_RIGHTARROW)
		{
			S_LocalSound ("misc/menu3.wav");
			s++;
		}

		if (s < 0)
			s = 0;
		if (s > ANISOTROPY_MAX)
			s = ANISOTROPY_MAX;

		Cvar_SetValue( gl_texanisotropy.name, s );
	}
	else if (g_menu_cursor_line == MENU_LINE_MSAA)
	{
		s = (int)(gl_msaa.value + 0.01);
		if (key == K_LEFTARROW)
		{
			S_LocalSound ("misc/menu3.wav");
			if (s <= 2)
				s = 0;
			else
				s >>= 1;
		}
		if (key == K_RIGHTARROW)
		{
			S_LocalSound ("misc/menu3.wav");
			if ( s <= 0 )
				s = 2;
			else
				s <<= 1;
		}

		if (s < 0)
			s = 0;
		if (s > MSAA_MAX)
			s = MSAA_MAX;

		Cvar_SetValue( gl_msaa.name, (double)s );
	}
	else if (g_menu_cursor_line == MENU_LINE_FULLSCREEN)
	{
		if (is_flag_key)
		{
			S_LocalSound ("misc/menu3.wav");
			Cvar_SetValue( gl_fullscreen.name, !((int)gl_fullscreen.value) );
		}
	}
	else if (g_menu_cursor_line == MENU_LINE_VSYNC)
	{
		if (is_flag_key)
		{
			S_LocalSound ("misc/menu3.wav");
			Cvar_SetValue( gl_vsync.name, !((int)gl_vsync.value) );
		}
	}
	else if (g_menu_cursor_line == MENU_LINE_WIDTH || g_menu_cursor_line == MENU_LINE_HEIGHT)
	{
		cvar_t* var = g_menu_cursor_line == MENU_LINE_WIDTH ? &gl_width : &gl_height;

		if (key >= '0' && key <= '9' && var->value <= 9999)
		{
			S_LocalSound ("misc/menu3.wav");
			var->value *= 10.0;
			var->value += key - '0';
		}
		if (key == K_BACKSPACE)
		{
			S_LocalSound ("misc/menu3.wav");
			var->value = ((int)var->value) / 10;
		}
		
		Cvar_SetValue( var->name, var->value );
	}
}


/*
Use special config for GL settings, which we must recieve at startup and never later.
We can not store this variables in main config, because if we try load it, we receive tons of uglu "cvar_name not found" messages.
*/

static void LoadGLConfig(void)
{
	Cbuf_InsertText ("exec gl_quake.cfg\n");
	Cbuf_Execute ();
}

static void SaveGLConfig(void)
{
	FILE*		f;
	char		path[256];
	const char*	format;

	sprintf( path, "%s/gl_quake.cfg", com_gamedir );
	f = fopen( path, "wb" );
	if (f)
	{
		format = "%s \"%s\"\n";

		fprintf( f, format, gl_width .name, gl_width .string );
		fprintf( f, format, gl_height.name, gl_height.string );
		fprintf( f, format, gl_display.name, gl_display.string );
		fprintf( f, format, gl_fullscreen.name, gl_fullscreen.string );
		fprintf( f, format, gl_vsync.name, gl_vsync.string );
		fprintf( f, format, gl_msaa.name, gl_msaa.string );

		fclose(f);
	}
}

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
	qboolean		fullscreen;
	int				display;
	int				msaa_samples;

	Cvar_RegisterVariable( &gl_texanisotropy );
	Cvar_RegisterVariable( &gl_width  );
	Cvar_RegisterVariable( &gl_height );
	Cvar_RegisterVariable( &gl_display );
	Cvar_RegisterVariable( &gl_vsync );
	Cvar_RegisterVariable( &gl_fullscreen );
	Cvar_RegisterVariable( &gl_msaa );

	LoadGLConfig();

	vid.width  = (int) gl_width. value;
	vid.height = (int) gl_height.value;

	fullscreen = (int) gl_fullscreen.value;
	display = (int)gl_display.value;

	msaa_samples = (int) gl_msaa.value;

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
			"PanzerQuake (OpenGL)",
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			vid.width, vid.height,
			SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | ( fullscreen ? SDL_WINDOW_FULLSCREEN : 0 ) );

	if (!g_sdl_gl.window)
		Sys_Error("Can not create window.");

	if (fullscreen)
		fullscreen = VID_SwitchToMode( g_sdl_gl.window, &display_mode );

	VID_SaveSystemGamma( g_sdl_gl.window );
	VID_UpdateGamma();

	g_sdl_gl.context = SDL_GL_CreateContext( g_sdl_gl.window );

	if (!g_sdl_gl.context)
		Sys_Error("Can not get window context.");

	SDL_GL_MakeCurrent( g_sdl_gl.window, g_sdl_gl.context );

	SDL_GL_SetSwapInterval( gl_vsync.value ? 1 : 0 );

	VID_SetPalette(palette);

	GetGLFuncs();

	CheckGLExtensions();
	
	SetupGLState();

	VID_FPSInit();

	vid_menudrawfn = MenuDrawFn;
	vid_menukeyfn = MenuKeyFn;
}

void	VID_Shutdown (void)
{
	VID_RestoreSystemGamma( g_sdl_gl.window );

	SDL_GL_DeleteContext( g_sdl_gl.context );
	SDL_DestroyWindow( g_sdl_gl.window );

	SaveGLConfig();
}

void	VID_UpdateGamma	(void)
{
	VID_UpdateGammaImpl( g_sdl_gl.window );
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

void VID_HandlePause (qboolean pause)
{
	// panzer - stub, do something with it later
}

void GL_EndRendering (void)
{
	VID_FPSUpdate();
	SDL_GL_SwapWindow( g_sdl_gl.window );
}
