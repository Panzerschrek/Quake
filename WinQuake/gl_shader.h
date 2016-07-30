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

typedef enum 
{
	SHADER_NONE = 0,
	SHADER_SCREEN_WARP,
	SHADER_WATER_TURB,
	SHADER_SKY,
	SHADER_WORLD,
	SHADER_ALIAS,
	SHADER_NUM,
} gl_shader_t;

void GL_InitShaders(void);

void GL_BindShader(gl_shader_t shader);

void GL_ShaderUniformInt( const char* name, int uniform_val );
void GL_ShaderUniformFloat( const char* name, float uniform_val );
void GL_ShaderUniformVec2( const char* name, float val0, float val1 );