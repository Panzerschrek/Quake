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

static GLuint hatching_texture= ~0;
static int hatching_texture_size_log2= 9;
static int hatching_texture_bright_levels= 16;

static void GenerateHatchingTexture( byte* data )
{
	int size= 1 << hatching_texture_size_log2;

	memset( data, 0, size * size * 2 );
	for( int bright_level= 1; bright_level < hatching_texture_bright_levels - 1; )
	{
		byte* level_data= data + bright_level * size * size;
		for( int t= 0; t < size * size / 1024; ++t )
		{
			int x= rand() & ( size - 1 );
			int y= rand() & ( size - 1 );
			level_data[ x + y * size ]= 255;
		}

		// On each step add just a bit of random points and calculate average brightness.
		// Finish generating level, when target brightness achieved.

		int avg_brightness= 0;
		for( int i= 0; i < size * size; ++i )
			avg_brightness+= level_data[i];
		avg_brightness /= size * size;

		if( avg_brightness < bright_level * 255 / hatching_texture_bright_levels )
			continue;

		++bright_level;
		if( bright_level < hatching_texture_bright_levels - 1 )
			memcpy( level_data + size * size, level_data, size * size );
		else
			memset( level_data + size * size, 255, size * size );
	}
}

void GL_InitHatching()
{
    int size = 1 << hatching_texture_size_log2;
    byte* data= malloc( size * size * hatching_texture_bright_levels );
	GenerateHatchingTexture( data );

    glGenTextures( 1, &hatching_texture );
    glBindTexture( GL_TEXTURE_3D, hatching_texture );

	glTexImage3D( GL_TEXTURE_3D, 0, GL_R8, size, size, hatching_texture_bright_levels, 0, GL_RED, GL_UNSIGNED_BYTE, data );

	glTexParameterf( GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameterf( GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D,  GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );

    free( data );
}

void GL_HatchingBindTexture()
{
	glBindTexture( GL_TEXTURE_3D, hatching_texture );
}
