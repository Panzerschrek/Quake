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

static void GenerateHatchingTextureOrderedCircles( byte* data, int size_log2, int bright_levels )
{
	int size= 1 << size_log2;
	int circle_size= 8;
	if( circle_size > size )
		circle_size= size;

	memset( data, 0, size * size ); // Fill first level with zeros.
	float overbright_brightness= M_PI / 4.0;
	for( int level= 1; level < bright_levels - 1; ++level )
	{
		byte* level_data= data + level * size * size;
		int scale_factor= 32;

		float brightness_f= ((float)level) / ((float)bright_levels);
		int radius;
		if( brightness_f < overbright_brightness )
			radius= sqrt( brightness_f / overbright_brightness ) * scale_factor * circle_size / 2;
		else
			radius= ( 1.0f + ( sqrt(2.0f) - 1.0f ) * ( brightness_f - overbright_brightness ) / ( 1.0f - overbright_brightness ) ) * scale_factor * circle_size / 2;

		int square_radius_minus_half= (radius - scale_factor / 2 ) * ( radius  - scale_factor / 2 );
		int square_radius_plus_half = (radius + scale_factor / 2 ) * ( radius  + scale_factor / 2 );

		for( int cx= 0; cx < size / circle_size; ++cx )
		for( int cy= 0; cy < size / circle_size; ++cy )
		for( int x= 0; x < circle_size; ++x )
		for( int y= 0; y < circle_size; ++y )
		{
			int dx= x <= circle_size / 2 ? x : (circle_size - x);
			int dy= y <= circle_size / 2 ? y : (circle_size - y);
			float square_distance= ( dx * dx + dy * dy ) * scale_factor * scale_factor;
			byte* dst= &level_data[ ( x + cx * circle_size ) + ( y + cy * circle_size ) * size ];
			if( square_distance < square_radius_minus_half )
				*dst= 255;
			else if( square_distance < square_radius_plus_half )
				*dst= 255 * ( square_radius_plus_half - square_distance ) / ( square_radius_plus_half - square_radius_minus_half );
			else
				*dst= 0;
		}
	}
	memset( data + (bright_levels - 1) * size * size, 255, size * size ); // Fill last level with ones.
}

void GL_InitHatching()
{
	// If this changes, shaders must be changed too!
	const int hatching_texture_size_log2= 8;
	const int hatching_texture_bright_levels= 16;

	int size = 1 << hatching_texture_size_log2;
	byte* data= malloc( size * size * hatching_texture_bright_levels );

	glEnable( GL_TEXTURE_2D_ARRAY_EXT );
	glGenTextures( 1, &hatching_texture );
	glBindTexture( GL_TEXTURE_2D_ARRAY_EXT, hatching_texture );

	for( int mip= 0; mip < hatching_texture_size_log2 + 1; ++mip )
	{
		int mip_size= size >> mip;
		if( mip_size < 1 ) mip_size = 1;

		GenerateHatchingTextureOrderedCircles( data, hatching_texture_size_log2 - mip, hatching_texture_bright_levels );
		QglTexImage3D(
			GL_TEXTURE_2D_ARRAY_EXT, mip, GL_R8,
			mip_size, mip_size, hatching_texture_bright_levels,
			0, GL_RED, GL_UNSIGNED_BYTE, data );
	}

	glTexParameterf( GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
	glTexParameterf( GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameterf( GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_LOD_BIAS, 0.0f );
	glTexParameterf( GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_MAX_LOD, 5.0f );
	glTexParameteri( GL_TEXTURE_2D_ARRAY_EXT,  GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );

	if (gl_texanisotropy.value > 0.0)
	{
		int anisotropy = (int)gl_texanisotropy.value;
		if (anisotropy > gl_max_texanisotropy)
			anisotropy = gl_max_texanisotropy;

		glTexParameteri( GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy );
	}

	free( data );
}

void GL_HatchingPrepareShader()
{
	if( gl_hatching.value )
	{
		GL_BindShader( SHADER_WORLD_HATCHING );
		GL_SelectTexture( GL_TEXTURE2 );
		glBindTexture( GL_TEXTURE_2D_ARRAY_EXT, hatching_texture );
		GL_SelectTexture( GL_TEXTURE0 );
	}
}

void GL_HatchingPrepareShaderAlias()
{
	if( gl_hatching.value )
	{
		GL_BindShader( SHADER_ALIAS_HATCHING );
		GL_SelectTexture( GL_TEXTURE2 );
		glBindTexture( GL_TEXTURE_2D_ARRAY_EXT, hatching_texture );
		GL_SelectTexture( GL_TEXTURE0 );
	}
}

void GL_HatchingPrepareShaderWaterTurb()
{
	if( gl_hatching.value )
	{
		GL_BindShader( SHADER_WATER_TURB_HATCHING );
		GL_SelectTexture( GL_TEXTURE2 );
		glBindTexture( GL_TEXTURE_2D_ARRAY_EXT, hatching_texture );
		GL_SelectTexture( GL_TEXTURE0 );
	}
}

void GL_HatchingPrepareShaderSky()
{
	if( gl_hatching.value )
	{
		GL_BindShader( SHADER_SKY_HATCHING );
		GL_SelectTexture( GL_TEXTURE2 );
		glBindTexture( GL_TEXTURE_2D_ARRAY_EXT, hatching_texture );
		GL_SelectTexture( GL_TEXTURE0 );
	}
}
