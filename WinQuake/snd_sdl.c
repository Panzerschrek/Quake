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

enum MusicState
{
	MUSIC_PLAYING,
	MUSIC_PAUSED,
	MUSIC_STOPPED,
};

static struct
{
	sample_t*	buffer;
	int			sample_bits;
	int			channels;
	int			freq; // samples/sec

	// Size - in samples
	int			buffer_size;
	int			buffer_pos;

	qboolean	looping;

	enum MusicState		state;
} g_music;

static int NearestPowerOfTwoFloor( int x )
{
	int r = 1<<30;
	while(r > x) r>>= 1;
	return r;
}

static void MixMusic(Uint8* stream, int len)
{
	int			out_len_in_samples;
	int			out_len_to_write;
	int			in_samples_count;
	int			in_end;
	int			i;
	int			channels[2], sample, sample_pos;
	fixed8_t	volume;
	fixed16_t	step, inv_step;
	sample_t*	out;
	Uint8*		src_8bit;

	if (g_music.state != MUSIC_PLAYING)
		return;

	volume= bgmvolume.value * 256;
	if( volume < 0 ) volume = 0;
	if( volume > 256 ) volume = 256;

	out_len_in_samples = len / ( sizeof(sample_t) * g_sdl_audio.format.channels );
	out = (sample_t*) stream;

	step = ( ((long long int)g_music.freq) << 16 ) / g_sdl_audio.format.freq;
	in_samples_count = (step * out_len_in_samples) >> 16;
	in_end = g_music.buffer_pos + in_samples_count;

	if( in_end > g_music.buffer_size )
	{
		in_end = g_music.buffer_size;
		inv_step = ( ((long long int)g_sdl_audio.format.freq) << 16 ) / g_music.freq;

		out_len_to_write= Fixed16Mul( in_end - g_music.buffer_pos, inv_step );

		// Cut just a little - we have calculating errors - we do not want read outside src buffer.
		if( out_len_to_write > 0 )
			out_len_to_write--;
	}
	else
		out_len_to_write = out_len_in_samples;

	if( g_music.sample_bits == 16 )
	{
		if( g_music.channels == 1 )
		{
			for (i= 0; i < out_len_to_write; i++)
			{
				channels[0] = out[i*2  ];
				channels[1] = out[i*2+1];
				sample = g_music.buffer[ g_music.buffer_pos + Fixed16Mul(step, i) ];
				channels[0] += ( sample * volume ) >> 8;
				channels[1] += ( sample * volume ) >> 8;

				if( channels[0] >  32767 ) channels[0] =  32767;
				if( channels[0] < -32768 ) channels[0] = -32768;
				if( channels[1] >  32767 ) channels[1] =  32767;
				if( channels[1] < -32768 ) channels[1] = -32768;
				out[i*2  ] = channels[0];
				out[i*2+1] = channels[1];
			}
		} // mono
		else
		{
			for (i= 0; i < out_len_to_write; i++)
			{
				channels[0] = out[i*2  ];
				channels[1] = out[i*2+1];
				sample_pos = 2 * ( g_music.buffer_pos + Fixed16Mul(step, i) );
				channels[0] += ( g_music.buffer[ sample_pos     ] * volume ) >> 8;
				channels[1] += ( g_music.buffer[ sample_pos + 1 ] * volume ) >> 8;

				if( channels[0] >  32767 ) channels[0] =  32767;
				if( channels[0] < -32768 ) channels[0] = -32768;
				if( channels[1] >  32767 ) channels[1] =  32767;
				if( channels[1] < -32768 ) channels[1] = -32768;
				out[i*2  ] = channels[0];
				out[i*2+1] = channels[1];
			}
		} // stereo
	} //16bit
	else
	{
		src_8bit= (Uint8*) g_music.buffer;
		if( g_music.channels == 1 )
		{
			for (i= 0; i < out_len_to_write; i++)
			{
				channels[0] = out[i*2  ];
				channels[1] = out[i*2+1];
				sample = (int)src_8bit[ g_music.buffer_pos + Fixed16Mul(step, i) ] - 128;
				channels[0] += sample * volume;
				channels[1] += sample * volume;

				if( channels[0] >  32767 ) channels[0] =  32767;
				if( channels[0] < -32768 ) channels[0] = -32768;
				if( channels[1] >  32767 ) channels[1] =  32767;
				if( channels[1] < -32768 ) channels[1] = -32768;
				out[i*2  ] = channels[0];
				out[i*2+1] = channels[1];
			}
		} // mono
		else
		{
			for (i= 0; i < out_len_to_write; i++)
			{
				channels[0] = out[i*2  ];
				channels[1] = out[i*2+1];
				sample_pos = 2 * ( g_music.buffer_pos + Fixed16Mul(step, i) );
				channels[0] += ((int)src_8bit[ sample_pos     ] - 128) * volume;
				channels[1] += ((int)src_8bit[ sample_pos + 1 ] - 128) * volume;

				if( channels[0] >  32767 ) channels[0] =  32767;
				if( channels[0] < -32768 ) channels[0] = -32768;
				if( channels[1] >  32767 ) channels[1] =  32767;
				if( channels[1] < -32768 ) channels[1] = -32768;
				out[i*2  ] = channels[0];
				out[i*2+1] = channels[1];
			}
		} // stereo
	} // 8bit

	g_music.buffer_pos= in_end;

	if( g_music.buffer_pos == g_music.buffer_size )
	{
		if( g_music.looping )
			g_music.buffer_pos= 0;
		else
		{
			g_music.state = MUSIC_STOPPED;
			SDL_FreeWAV( (Uint8*) g_music.buffer );
		}
	}
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

	MixMusic(stream, len);

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

	g_music.state= MUSIC_STOPPED;

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

void CDAudio_Play(byte track, qboolean looping)
{
	char			track_name[1024];
	int				cd_path_param;
	SDL_AudioSpec	spec;
	Uint8*			buff;
	Uint32			len;

	cd_path_param = COM_CheckParm( "-cdpath" );
	if( cd_path_param <= 0 )
		return;

	sprintf( track_name, "%s/%02d.wav", com_argv[cd_path_param + 1], track );

	// Yes, we load full wav file.
	// For 10 minutes 22kHz 16bits mono we spend about 25 megabytes of memory.
	// But who cares about memory in year 2016?
	if( !SDL_LoadWAV( track_name, &spec, &buff, &len ) )
	{
		Con_Printf( "Failed to load \"%s\"\n", track_name );
		return;
	}

	if( !( spec.channels == 1 || spec.channels == 2 ) ||
		!( spec.format == AUDIO_S16 || spec.format == AUDIO_U8 ) )
	{
		SDL_FreeWAV( buff );
		Con_Printf( "\"%s\" - invalid audio format. Supported 8 or 16 bits mono or stereo.\n",track_name );
		return;
	}

	g_music.buffer= (sample_t*) buff;
	g_music.freq= spec.freq;
	g_music.channels= spec.channels;
	g_music.sample_bits= SDL_AUDIO_BITSIZE(spec.format);
	
	g_music.buffer_size= len / (g_music.channels * g_music.sample_bits / 8);

	g_music.buffer_pos= 0;

	g_music.state = MUSIC_PLAYING;
	g_music.looping = looping;
}

void CDAudio_Stop(void)
{
	SNDDMA_LockSoundData();

	if( g_music.state != MUSIC_STOPPED )
	{
		g_music.state = MUSIC_STOPPED;
		SDL_FreeWAV( (Uint8*) g_music.buffer );
	}

	SNDDMA_UnlockSoundData();
}

void CDAudio_Pause(void)
{
	SNDDMA_LockSoundData();

	if( g_music.state == MUSIC_PLAYING )
		g_music.state = MUSIC_PAUSED;

	SNDDMA_UnlockSoundData();
}

void CDAudio_Resume(void)
{
	SNDDMA_LockSoundData();

	if( g_music.state == MUSIC_PAUSED )
		g_music.state = MUSIC_PLAYING;

	SNDDMA_UnlockSoundData();
}

void CDAudio_Update(void)
{
}

int CDAudio_Init(void)
{
	return 0;
}

void CDAudio_Shutdown(void)
{
	CDAudio_Stop();
}