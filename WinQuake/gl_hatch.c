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

static void PatternGen_Hatch( byte* level_data )
{
	int size= 1 << hatching_texture_size_log2;
	int size_mask= size - 1;
	int hatch_half_width= 0;

	int start_x= rand() & size_mask;
	int start_y= rand() & size_mask;
	int length= rand() & ( ( size / 32 ) - 1 ) + ( size / 32 );
	if( rand() & 1 )
	{
		for( int d= -hatch_half_width; d <= hatch_half_width; ++d )
		for( int x= 0; x < length; ++x )
			level_data[
				( ( x + start_x ) & size_mask ) +
				( ( d + start_y ) & size_mask ) * size ]= 255;
	}
	else
	{
		for( int d= -hatch_half_width; d <= hatch_half_width; ++d )
		for( int y= 0; y < length; ++y )
			level_data[
				( ( d + start_x ) & size_mask ) +
				( ( y + start_y ) & size_mask ) * size ]= 255;
	}
}

static void PatternGen_Rand( byte* level_data )
{
	int size= 1 << hatching_texture_size_log2;
	int size_mask= size - 1;
	for( int i= 0; i < 8; ++i )
	{
		int x= rand() & size_mask;
		int y= rand() & size_mask;
		level_data[ x + y * size ]= 255;
	}
}


static void GenerateHatchingTexture( byte* data )
{
	int size= 1 << hatching_texture_size_log2;

	memset( data, 0, size * size * 2 );
	for( int bright_level= 1; bright_level < hatching_texture_bright_levels - 1; )
	{
		byte* level_data= data + bright_level * size * size;
		for( int t= 0; t < size * size / ( 128 * 128 ); ++t )
		{
			//PatternGen_Rand( level_data );
			PatternGen_Hatch( level_data );
		}

		// On each step add just a bit of random points and calculate average brightness.
		// Finish generating level, when target brightness achieved.

		int avg_brightness= 0;
		for( int i= 0; i < size * size; ++i )
			avg_brightness+= level_data[i];
		avg_brightness /= size * size;

		if( avg_brightness < bright_level * 255 / ( hatching_texture_bright_levels - 1 ) )
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
