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
#include <stdio.h>

#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#else
#include <sys/stat.h>
#endif

#include <SDL.h>
#include <SDL_messagebox.h>
#include <SDL_filesystem.h>

#include "quakedef.h"

qboolean isDedicated = 0;

#define MAX_FILE_HANDLES 32
FILE* g_file_handles[ MAX_FILE_HANDLES ] = { 0 };

int Sys_FileOpenRead (char *path, int *hndl)
{
	FILE*	f;
	int		size;
	int		i;
	int		handle_num = -1;

	for (i = 0; i < MAX_FILE_HANDLES; i++)
	{
		if( g_file_handles[i] == NULL )
		{
			handle_num = i;
			break;
		}
	}

	if (handle_num == -1)
		Sys_Error( "Not enough system file handles." );

	f = fopen( path, "rb" );
	if( f == NULL ) return -1;

	g_file_handles[handle_num] = f;
	*hndl = handle_num;

	fseek( f, 0, SEEK_END );
	size = ftell(f);
	fseek( f, 0, SEEK_SET );

	return size;
}

int Sys_FileOpenWrite (char *path)
{
	FILE*	f;
	int		i;
	int		handle_num = -1;

	for (i = 0; i < MAX_FILE_HANDLES; i++)
	{
		if( g_file_handles[i] == NULL )
		{
			handle_num = i;
			break;
		}
	}

	if (handle_num == -1)
		Sys_Error( "Not enough system file handles." );

	f = fopen( path, "rb" );
	if( f == NULL ) return -1;

	g_file_handles[handle_num] = f;
	return handle_num;
}

void Sys_FileClose (int handle)
{
	int		i;
	FILE*	f;
	
	if ( handle < 0 || handle >= MAX_FILE_HANDLES )
		Sys_Error( "Invalid file handle." );

	f = g_file_handles[handle];
	if ( f == NULL )
		Sys_Error( "Trying to close closed file." );

	g_file_handles[handle] = NULL;
	fclose(f);
}

void Sys_FileSeek (int handle, int position)
{
	FILE*	f;

	if ( handle < 0 || handle >= MAX_FILE_HANDLES )
		Sys_Error( "Invalid file handle." );

	f = g_file_handles[handle];
	if ( f == NULL )
		Sys_Error( "Trying to seek closed file." );

	fseek( f, position, SEEK_SET );
}

int Sys_FileRead (int handle, void *dest, int count)
{
	FILE*	f;

	if ( handle < 0 || handle >= MAX_FILE_HANDLES )
		Sys_Error( "Invalid file handle." );

	f = g_file_handles[handle];
	if ( f == NULL )
		Sys_Error( "Trying to read closed file." );

	return fread( dest, 1, count, f );
}

int Sys_FileWrite (int handle, void *data, int count)
{
	FILE*	f;

	if ( handle < 0 || handle >= MAX_FILE_HANDLES )
		Sys_Error( "Invalid file handle." );

	f = g_file_handles[handle];
	if ( f == NULL )
		Sys_Error( "Trying to read closed file." );

	return fwrite( data, 1, count, f );
}

int	Sys_FileTime (char *path)
{
	FILE*	f;

	f = fopen( path, "rb" );
	if ( f != NULL )
	{
		fclose(f);
		return 1;
	}
	else
		return -1;
}

void Sys_mkdir (char *path)
{
	// panzer - windows stuff, remove this
#ifdef _WIN32
	_mkdir (path);
#else
	mkdir (path, 0777);
#endif
}

void Sys_Error (char *error, ...)
{
	va_list		argptr;
	char		text[4096];

	va_start (argptr, error);
	vsprintf (text, error, argptr);
	va_end (argptr);

	SDL_ShowSimpleMessageBox( 0, "Error", text, NULL );

	Sys_Quit();
}

void Sys_Printf (char *fmt, ...)
{
	va_list		argptr;
	char		text[4096];

	va_start (argptr, fmt);
	vsprintf (text, fmt, argptr);
	va_end (argptr);

	printf( "%s", text );
}

void Sys_Quit (void)
{
	SDL_Quit();
	exit(0);
}

double Sys_FloatTime (void)
{
	return ( (double)SDL_GetTicks() ) * 0.001;
}

char *Sys_ConsoleInput (void)
{
	// panzer - stub
	return NULL;
}

void Sys_Sleep (void)
{
	SDL_Delay(1);
}

void Sys_SendKeyEvents (void)
{
	IN_Commands();
}

#ifdef _WIN32
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	char		*argv[MAX_NUM_ARGVS];
#else
int main(int argc, const char* argv[])
{
#endif

	quakeparms_t	parms;
	double			oldtime;
	double			newtime;
	double			time;

	SDL_Init( SDL_INIT_TIMER | SDL_INIT_EVENTS );

#ifdef _WIN32
	parms.argc = 1;
	argv[0] = "";
	while (*lpCmdLine && (parms.argc < MAX_NUM_ARGVS))
	{
		while (*lpCmdLine && ((*lpCmdLine <= 32) || (*lpCmdLine > 126)))
			lpCmdLine++;

		if (*lpCmdLine)
		{
			argv[parms.argc] = lpCmdLine;
			parms.argc++;

			while (*lpCmdLine && ((*lpCmdLine > 32) && (*lpCmdLine <= 126)))
				lpCmdLine++;

			if (*lpCmdLine)
			{
				*lpCmdLine = 0;
				lpCmdLine++;
			}
			
		}
	}
	parms.argv = argv;
#else
	parms.argc = argc;
	parms.argv = argv;
#endif

	COM_InitArgv (parms.argc, parms.argv );

	parms.basedir = ".";
	parms.cachedir = NULL;
	parms.memsize = 128 * 1024 * 1024;
	parms.membase = malloc( parms.memsize );

	Sys_Printf ("Host_Init\n");
	Host_Init (&parms);

	oldtime = Sys_FloatTime ();

	while (1)
	{
		do
		{
			newtime = Sys_FloatTime ();
			time = newtime - oldtime;

			if( time >= 0.001 ) break;
			Sys_Sleep();
		}while(1);

		Host_Frame (time);
		oldtime = newtime;
	}

	SDL_Quit();
}