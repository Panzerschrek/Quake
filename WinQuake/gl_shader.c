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


#define MAX_UNIFORMS 16
#define MAX_UNIFORM_NAME_LEN 16

typedef struct
{
	GLuint		handle;
	
	char		uniforms_names[ MAX_UNIFORMS ][ MAX_UNIFORM_NAME_LEN ];
	GLint		uniforms_id[ MAX_UNIFORMS ];
	int			uniform_num;

} program_t;

program_t		programs[ SHADER_NUM ];
gl_shader_t		current_shader = SHADER_NONE;

static char logs_buff[16384];

static void ProcessShader( GLenum shader_type, GLuint prog_handle, const char* text )
{
	GLuint			handle;
	const char*		shader_text_lines[1];
	GLint			shader_text_lines_size[1];
	GLint			log_length;

	handle = glCreateShader( shader_type );

	shader_text_lines[0] = text;
	shader_text_lines_size[0]= strlen(text);

	glShaderSource( handle, 1, shader_text_lines, shader_text_lines_size );

	glGetShaderiv( handle, GL_INFO_LOG_LENGTH, &log_length );
	if( log_length > 1 )
	{
		glGetShaderInfoLog( handle, sizeof(logs_buff) - 1, &log_length, logs_buff );
		Sys_Printf( "Error, while compiling shader: %s\n", logs_buff );
	}

	glAttachShader( prog_handle, handle );
}

static void InitProgram( gl_shader_t program_num, const char* vert_text, const char* frag_text )
{
	GLuint		prog_handle;
	GLint		program_log_length;

	prog_handle= glCreateProgram();

	ProcessShader(   GL_VERTEX_SHADER, prog_handle, vert_text );
	ProcessShader( GL_FRAGMENT_SHADER, prog_handle, frag_text );

	glLinkProgram( prog_handle );

	glGetProgramiv( prog_handle, GL_INFO_LOG_LENGTH, &program_log_length );
	if( program_log_length > 1 )
	{
		glGetProgramInfoLog( prog_handle, sizeof(logs_buff) - 1, &program_log_length, logs_buff );
		Sys_Printf( "Error, while linking program: %s\n", logs_buff );
	}

	programs[program_num].handle = prog_handle;
}

static const char warp_shader_v[]= "\
#version 120\n\
\
void main(void)\
{\
	gl_TexCoord[0] = gl_MultiTexCoord0;\
	gl_Position = gl_Vertex;\
}\
";

static const char warp_shader_f[]= "\
#version 120\n\
\
uniform sampler2D tex;\
uniform vec2 tex_size;\
uniform float time;\
\
void main(void)\
{\
	const float pi = 3.1415926535;\
	float time_freq = pi * 0.6;\
	vec2 omega = 0.01 * pi * tex_size;\
	vec2 amplitude = 6.0 / tex_size;\
	vec2 tc = gl_TexCoord[0].xy;\
	tc = tc + amplitude * sin( time_freq * vec2(time, time) + omega * tc.yx );\
	gl_FragColor = texture2D( tex, tc );\
}\
";

void GL_InitShaders(void)
{
	programs[ SHADER_NONE ].handle = 0;

	InitProgram( SHADER_SCREEN_WARP, warp_shader_v, warp_shader_f );
}

void GL_BindShader(gl_shader_t shader)
{
	if (shader != current_shader)
	{
		if (shader < 0 || shader >= SHADER_NUM )
			Sys_Error( "GL_BindShader: invalid shader number\n" );

		current_shader = shader;
		glUseProgram( programs[shader].handle );
	}
}

static GLint GetUniformLocation( const char* name )
{
	int			i;
	program_t*	program;
	GLint		uniform_location;

	program = &programs[current_shader];

	for (i = 0; i < program->uniform_num; i++)
		if (strcmp(name, program->uniforms_names[i]) == 0)
			return program->uniforms_id[i];

	uniform_location = glGetUniformLocation( program->handle, name );
	if (uniform_location == -1)
	{
		Sys_Printf( "Uniform location not found\n" );
		return uniform_location;
	}
	if (program->uniform_num == MAX_UNIFORMS)
	{
		Sys_Printf( "Too many uniforms\n" );
		return uniform_location;
	}
	if (strlen(name) >= MAX_UNIFORM_NAME_LEN)
	{
		Sys_Printf( "Uniform name too long\n" );
		return uniform_location;
	}

	strcpy( program->uniforms_names[ program->uniform_num ], name );
	program->uniforms_id[ program->uniform_num ] = uniform_location;
	program->uniform_num ++;

	return uniform_location;
}

void GL_ShaderUniformInt( const char* name, int uniform_val )
{
	glUniform1i( GetUniformLocation(name), uniform_val );
}

void GL_ShaderUniformFloat( const char* name, float uniform_val )
{
	glUniform1f( GetUniformLocation(name), uniform_val );
}

void GL_ShaderUniformVec2( const char* name, float val0, float val1 )
{
	glUniform2f( GetUniformLocation(name), val0, val1 );
}