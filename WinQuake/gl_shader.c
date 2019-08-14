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


#define MAX_UNIFORMS 16
#define MAX_UNIFORM_NAME_LEN 24

#define PI_S "3.1415926535"

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
	glCompileShader( handle );

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
	float time_freq = "PI_S" * 0.6;\
	vec2 omega = 0.01 * "PI_S" * tex_size;\
	vec2 amplitude = 6.0 / tex_size;\
	vec2 tc = gl_TexCoord[0].xy;\
	tc = tc + amplitude * sin( time_freq * vec2(time, time) + omega * tc.yx );\
	gl_FragColor = texture2D( tex, tc );\
}\
";

static const char outlines_shader_v[]= "\
#version 120\n\
\
void main(void)\
{\
	gl_TexCoord[0] = gl_MultiTexCoord0;\
	gl_Position = gl_Vertex;\
}\
";

static const char outlines_shader_f[]= "\
#version 120\n\
\
uniform sampler2D tex;\
uniform sampler2D depth_buffer;\
uniform vec2 tex_size;\
uniform float time;\
\
void main(void)\
{\
	vec4 color= texture2D( tex, gl_TexCoord[0].xy );\
	vec2 texel_step= vec2(1.0, 1.0) / tex_size;\
	float d[5];\
	d[0]= texture2D( depth_buffer, gl_TexCoord[0].xy ).x;\
	d[1]= texture2D( depth_buffer, gl_TexCoord[0].xy + vec2(+texel_step.x,  0) ).x;\
	d[2]= texture2D( depth_buffer, gl_TexCoord[0].xy + vec2(-texel_step.x,  0) ).x;\
	d[3]= texture2D( depth_buffer, gl_TexCoord[0].xy + vec2( 0, +texel_step.y) ).x;\
	d[4]= texture2D( depth_buffer, gl_TexCoord[0].xy + vec2( 0, -texel_step.y) ).x;\
	const float depth_eps= 32.0 / 16777216.0; /* estimated accuracy of depth buffer */ \
	/* \
	1 / z must be linear inside polygon \
	detect pixels, where (1 / z)dx and (1 / z)dy both nonlinear \
	and take sum of neighbor pixels \
	*/ \
	float two_d_0= 2.0 * d[0];\
	if( ( abs( d[1] + d[2] - two_d_0 ) > depth_eps || abs( d[3] + d[4] - two_d_0 ) > depth_eps ) && color.a > 0.5 )\
	{\
		const int[16] dither_matrix= int[16]( 0, 8, 2, 10,   12, 4, 14, 16,  3, 11, 1, 9,  15, 7, 13, 5 );\
		ivec2 texel_coord= ivec2( gl_TexCoord[0].xy * tex_size );\
		float dither= float(dither_matrix[ int(mod(texel_coord.x, 2)) + 4 * int(mod(texel_coord.y, 4)) ]) / 16.0;\
\
		float color_brightness= dot( color.xyz, vec3( 0.299, 0.587, 0.114 ) ); \
		float brightness= sqrt(color_brightness);\
		gl_FragColor= vec4( brightness, brightness, brightness, 1.0 );\
	}\
	else\
		 gl_FragColor= color;\
}\
";

static const char water_turb_shader_v[]= "\
#version 120\n\
\
void main(void)\
{\
	gl_TexCoord[0] = gl_MultiTexCoord0;\
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\
}\
";

static const char water_turb_shader_f[]= "\
#version 120\n\
\
uniform sampler2D tex;\
uniform float time;\
uniform float alpha;\
\
void main(void)\
{\
	const float pi = 3.1415926535;\
	float time_freq = "PI_S" * 0.4;\
	float omega = 2.0 * "PI_S";\
	float amplitude = 0.0625;\
	vec2 tc = gl_TexCoord[0].xy;\
	tc = tc + amplitude * sin( time_freq * vec2(time, time) + omega * tc.yx );\
	gl_FragColor = vec4( texture2D( tex, tc ).xyz, alpha );\
}\
";

static const char sky_shader_v[]= "\
#version 120\n\
\
void main(void)\
{\
	gl_TexCoord[0] = gl_MultiTexCoord0;\
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\
}\
";

static const char sky_shader_f[]= "\
#version 120\n\
\
uniform sampler2D tex0;\
uniform sampler2D tex1;\
uniform float time;\
\
void main(void)\
{\
	const float speed0 = 1.0 / 16.0;\
	const float speed1 = 1.0 /  8.0;\
	vec3 n = normalize( gl_TexCoord[0].xyz );\
	vec2 tc = n.xy * ( 1.25 / ( abs(n.z) + 0.25 ) );\
	vec2 tc0 = tc + time * speed0 * vec2(1.0, 1.0);\
	vec2 tc1 = tc + time * speed1 * vec2(1.0, 1.0);\
	vec4 c0 = texture2D( tex0, tc0 );\
	vec4 c1 = texture2D( tex1, tc1 );\
	gl_FragColor = mix( c0, c1, c1.a );\
	gl_FragDepth= 1.0f; \
}\
";

static const char world_shader_v[]= "\
#version 120\n\
\
void main(void)\
{\
	gl_TexCoord[0] = gl_MultiTexCoord0;\
	gl_TexCoord[1] = gl_MultiTexCoord1;\
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\
}\
";

static const char world_shader_f[]= "\
#version 120\n\
\
uniform sampler2D tex;\
uniform sampler2D lightmap;\
uniform float light_gamma = 1.0;\
uniform float light_overbright = 1.0;\
\
void main(void)\
{\
	vec4 c = texture2D( tex, gl_TexCoord[0].xy );\
	float l = 2.0 * texture2D( lightmap, gl_TexCoord[1].xy ).x;\
	\
	l = pow(l, light_gamma) * light_overbright;\
	\
	/* mix lightmap and selft texture glow, stored in alpha texture component */ \
	gl_FragColor = vec4( c.xyz * mix( 1.0, l, c.a ), 1.0 );\
}\
";

static const char alias_shader_v[]= "\
#version 120\n\
\
varying float f_light;\
\
void main(void)\
{\
	f_light = 2.0 * gl_Color.r;\
	gl_TexCoord[0] = gl_MultiTexCoord0;\
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\
}\
";

static const char alias_shader_f[]= "\
#version 120\n\
\
uniform sampler2D tex;\
uniform float light_gamma = 1.0;\
uniform float light_overbright = 1.0;\
\
varying float f_light;\
\
void main(void)\
{\
	vec4 c = texture2D( tex, gl_TexCoord[0].xy );\
	float l = f_light;\
	\
	l = pow(l, light_gamma) * light_overbright;\
	\
	/* mix lightmap and selft texture glow, stored in alpha texture component */ \
	gl_FragColor = vec4( c.xyz * mix( 1.0, l, c.a ), 0.0 ); /* wtire zero to alpha to prevent outlines on models */ \
}\
";

static const char world_hatching_shader_v[]= "\
#version 120\n\
\
void main(void)\
{\
	gl_TexCoord[0] = gl_MultiTexCoord0;\
	gl_TexCoord[1] = gl_MultiTexCoord1;\
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\
}\
";

static const char world_hatching_shader_f[]= "\
#version 120\n\
#extension GL_EXT_texture_array : require\n\
uniform sampler2D tex;\
uniform sampler2D lightmap;\
uniform sampler2DArray hatching_texture;\
uniform float light_gamma = 1.0;\
uniform float light_overbright = 1.0;\
\
float HatchingFetch( float hatching_level )\
{\
	vec2 tc_scaled= gl_TexCoord[1].xy * vec2( 8.0, 8.0 );\
	const float tex_layers= 24.0;\
	float layer_num= floor( hatching_level * tex_layers );\
	float m= hatching_level * tex_layers - layer_num;\
	return mix(\
		texture2DArray( hatching_texture, vec3( tc_scaled, layer_num ) ).x,\
		texture2DArray( hatching_texture, vec3( tc_scaled, layer_num + 1.0 ) ).x,\
		m );\
}\
void main(void)\
{\
	vec4 c = texture2D( tex, gl_TexCoord[0].xy );\
	float l = 2.0 * texture2D( lightmap, gl_TexCoord[1].xy ).x;\
	\
	l = pow(l, light_gamma) * light_overbright;\
	\
	/* mix lightmap and selft texture glow, stored in alpha texture component */ \
	vec3 color= c.xyz * mix( 1.0, l, c.a );\
\
	float max_color_component= max( color.x, max( color.y, color.z ) );\
	float color_brightness= dot( color, vec3( 0.299, 0.587, 0.114 ) ); \
	float brightness= pow( mix( color_brightness, max_color_component, 0.5 ), 0.75 );\
	float hatching_level= clamp( 1.0 - brightness, 0.0, 1.0 ); \
	float hatching= HatchingFetch( hatching_level );\
	/*hatching= step( 0.5, hatching );*/\
	gl_FragColor = vec4( hatching, hatching, hatching, 1.0 );\
}\
";

static const char alias_hatching_shader_v[]= "\
#version 120\n\
\
varying float f_light;\
\
void main(void)\
{\
	f_light = 2.0 * gl_Color.r;\
	gl_TexCoord[0] = gl_MultiTexCoord0;\
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\
}\
";

static const char alias_hatching_shader_f[]= "\
#version 120\n\
#extension GL_EXT_texture_array : require\n\
uniform sampler2D tex;\
uniform sampler2DArray hatching_texture;\
uniform float light_gamma = 1.0;\
uniform float light_overbright = 1.0;\
varying float f_light;\
\
float HatchingFetch( float hatching_level )\
{\
	vec2 tc_scaled= gl_TexCoord[0].xy * vec2( 2.0, 2.0 );\
	const float tex_layers= 24.0;\
	float layer_num= floor( hatching_level * tex_layers );\
	float m= hatching_level * tex_layers - layer_num;\
	return mix(\
		texture2DArray( hatching_texture, vec3( tc_scaled, layer_num ) ).x,\
		texture2DArray( hatching_texture, vec3( tc_scaled, layer_num + 1.0 ) ).x,\
		m );\
}\
void main(void)\
{\
	vec4 c = texture2D( tex, gl_TexCoord[0].xy );\
	float l = f_light;\
	l = pow(l, light_gamma) * light_overbright;\
	vec3 color= c.xyz * mix( 1.0, l, c.a ); /* mix lightmap and selft texture glow, stored in alpha texture component */\
\
	float max_color_component= max( color.x, max( color.y, color.z ) );\
	float color_brightness= dot( color, vec3( 0.299, 0.587, 0.114 ) ); \
	float brightness= pow( mix( color_brightness, max_color_component, 0.5 ), 0.75 );\
	float hatching_level= clamp( 1.0 - brightness, 0.0, 1.0 ); \
	float hatching= HatchingFetch( hatching_level );\
	/*hatching= step( 0.5, hatching );*/\
	gl_FragColor = vec4( hatching, hatching, hatching, 0.0 ); /* wtire zero to alpha to prevent outlines on models */\
}\
";

void GL_InitShaders(void)
{
	programs[ SHADER_NONE ].handle = 0;

	InitProgram( SHADER_SCREEN_WARP, warp_shader_v, warp_shader_f );

	InitProgram( SHADER_OUTLINES, outlines_shader_v, outlines_shader_f );
	GL_BindShader( SHADER_OUTLINES );
	GL_ShaderUniformInt( "tex", 0 );
	GL_ShaderUniformInt( "depth_buffer", 1 );

	InitProgram( SHADER_WATER_TURB, water_turb_shader_v, water_turb_shader_f );
	GL_BindShader( SHADER_WATER_TURB );
	GL_ShaderUniformInt( "tex", 0 );

	InitProgram( SHADER_SKY, sky_shader_v, sky_shader_f );
	GL_BindShader( SHADER_SKY );
	GL_ShaderUniformInt( "tex0", 0 );
	GL_ShaderUniformInt( "tex1", 1 );

	InitProgram( SHADER_WORLD, world_shader_v, world_shader_f );
	GL_BindShader( SHADER_WORLD );
	GL_ShaderUniformInt( "tex", 0 );
	GL_ShaderUniformInt( "lightmap", 1 );

	InitProgram( SHADER_ALIAS, alias_shader_v, alias_shader_f );
	GL_BindShader( SHADER_ALIAS );
	GL_ShaderUniformInt( "tex", 0 );

	InitProgram( SHADER_WORLD_HATCHING, world_hatching_shader_v, world_hatching_shader_f );
	GL_BindShader( SHADER_WORLD_HATCHING );
	GL_ShaderUniformInt( "tex", 0 );
	GL_ShaderUniformInt( "lightmap", 1 );
	GL_ShaderUniformInt( "hatching_texture", 2 );

	InitProgram( SHADER_ALIAS_HATCHING, alias_hatching_shader_v, alias_hatching_shader_f );
	GL_BindShader( SHADER_ALIAS_HATCHING );
	GL_ShaderUniformInt( "tex", 0 );
	GL_ShaderUniformInt( "hatching_texture", 2 );

	GL_BindShader( SHADER_NONE );
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
