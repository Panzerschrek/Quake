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
#include <SDL_audio.h>
#include <SDL_Mutex.h>

#include "quakedef.h"

typedef short sample_t;

struct
{
	qboolean		initialized;
	SDL_AudioSpec	format;
	int				device_id;

	SDL_mutex*		mutex;
} g_sdl_audio = { false };

static int NearestPowerOfTwoFloor( int x )
{
	int r = 1<<30;
	while(r > x) r>>= 1;
	return r;
}

static void SDLCALL AudioCallback( void* userdata, Uint8* stream, int len )
{
	int				i;
	sample_t*		samples;
	int				len_in_samples;

	samples = (sample_t*) stream;
	len_in_samples = len / ( sizeof(sample_t) * g_sdl_audio.format.channels );

	SNDDMA_LockSoundData();

	shm->buffer = stream;
	S_PaintChannels( paintedtime + len_in_samples );

	SNDDMA_UnlockSoundData();
}

qboolean SNDDMA_Init(void)
{
	SDL_AudioSpec	audio_format;
	int				num_devices;
	int				i;
	const char*		device_name;

	SDL_InitSubSystem( SDL_INIT_AUDIO );


	audio_format.channels = 2;
	audio_format.freq = 22050;
	audio_format.format = AUDIO_S16;
	audio_format.callback = AudioCallback;

	// ~ 1 callback call per two frames (60fps)
	audio_format.samples = NearestPowerOfTwoFloor( audio_format.freq / 30 );

	g_sdl_audio.device_id = 0;
	num_devices = SDL_GetNumAudioDevices(0);

	// Can't get explicit devices list. Trying to use first device
	if( num_devices == -1 )
		num_devices = 1;

	for( i = 0; i < num_devices; i++ )
	{
		device_name = SDL_GetAudioDeviceName(i, 0);
		g_sdl_audio.device_id =
			SDL_OpenAudioDevice( device_name, 0, &audio_format, &g_sdl_audio.format, 0 );

		if( g_sdl_audio.device_id >= 2 )
		{
			Con_Printf( "Open audio device: %s", device_name );
			break;
		}
	}

	if (g_sdl_audio.device_id < 2)
	{
		Con_Print( "Could not open audio device" );
		return false;
	}

	if (g_sdl_audio.format.channels != 2 ||
		SDL_AUDIO_BITSIZE( g_sdl_audio.format.format ) != 16 )
	{
		SDL_CloseAudioDevice( g_sdl_audio.device_id );

		Con_Printf(
			"Could not open audio device with 16bits stereo. Got: %dbits %d channels",
			SDL_AUDIO_BITSIZE( g_sdl_audio.format.format ),
			g_sdl_audio.format.channels);

		return false;
	}

	g_sdl_audio.mutex = SDL_CreateMutex();

	SDL_PauseAudioDevice( g_sdl_audio.device_id , 0 );

	shm = (void *) malloc(sizeof(dma_t));
	shm->splitbuffer = 0;
	shm->samplebits = SDL_AUDIO_BITSIZE( g_sdl_audio.format.format );
	shm->speed = g_sdl_audio.format.freq;
	shm->channels = g_sdl_audio.format.channels;
	shm->samples = g_sdl_audio.format.size / sizeof(sample_t);
	shm->samplepos = 0;
	shm->soundalive = true;
	shm->gamealive = true;
	shm->submission_chunk = 1;
	shm->buffer = NULL;

	g_sdl_audio.initialized = true;

	return true;
}

void SNDDMA_Shutdown(void)
{
	if( !g_sdl_audio.initialized )
		return;

	SDL_CloseAudioDevice( g_sdl_audio.device_id );
	SDL_CloseAudio();
	SDL_DestroyMutex( g_sdl_audio.mutex );

	g_sdl_audio.initialized = false;
}

void SNDDMA_LockSoundData(void)
{
	SDL_LockMutex( g_sdl_audio.mutex );
}

void SNDDMA_UnlockSoundData(void)
{
	SDL_UnlockMutex( g_sdl_audio.mutex );
}