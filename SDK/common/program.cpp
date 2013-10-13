/*

GFX Lightweight OpenGLES 2.0 Game and Graphics Engine

Copyright (C) 2011 Romain Marucchi-Foino http://gfx.sio2interactive.com

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of
this software. Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it freely,
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that
you wrote the original software. If you use this software in a product, an acknowledgment
in the product would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be misrepresented
as being the original software.

3. This notice may not be removed or altered from any source distribution.

*/

#include "gfx.h"

PROGRAM *PROGRAM_init( char *name )
{
	PROGRAM *program = ( PROGRAM * ) calloc( 1, sizeof( PROGRAM ) );

	strcpy( program->name, name );
	
	return program;
}


PROGRAM *PROGRAM_free( PROGRAM *program )
{
	if( program->uniform_array ) free( program->uniform_array );
	
	if( program->vertex_attrib_array ) free( program->vertex_attrib_array );
	
	if( program->pid ) PROGRAM_delete_id( program );

	free( program );
	return NULL;
}


PROGRAM *PROGRAM_create( char						  *name, 
						 char						  *vertex_shader_filename,
						 char						  *fragment_shader_filename, 
						 unsigned char				  relative_path, 
						 unsigned char				  debug_shader,
						 PROGRAMBINDATTRIBCALLBACK	  *programbindattribcallback, 
						 PROGRAMDRAWCALLBACK		  *programdrawcallback )
{
	PROGRAM *program = PROGRAM_init( name );
	
	MEMORY *m = mopen( vertex_shader_filename, relative_path );

	if( m )
	{
		program->vertex_shader = SHADER_init( vertex_shader_filename, GL_VERTEX_SHADER );
		
		SHADER_compile( program->vertex_shader, ( char * )m->buffer, debug_shader );
		
		mclose( m );
	}
	
	m = mopen( fragment_shader_filename, relative_path );
	
	if( m )
	{
		program->fragment_shader = SHADER_init( fragment_shader_filename, GL_FRAGMENT_SHADER );
		
		SHADER_compile( program->fragment_shader, ( char * )m->buffer, debug_shader );
		
		mclose( m );
	}
	
	program->programbindattribcallback = programbindattribcallback;
	
	program->programdrawcallback = programdrawcallback;
	
	PROGRAM_link( program, debug_shader );
	
	return program;
}


unsigned char PROGRAM_add_uniform( PROGRAM *program, char *name, unsigned int type )
{
	unsigned char uniform_index = program->uniform_count;
	
	++program->uniform_count;

	program->uniform_array = ( UNIFORM * ) realloc( program->uniform_array,
													program->uniform_count * sizeof( UNIFORM ) );

	memset( &program->uniform_array[ uniform_index ], 0, sizeof( UNIFORM ) );
	
	strcpy( program->uniform_array[ uniform_index ].name, name );
	
	program->uniform_array[ uniform_index ].type = type;
	
	program->uniform_array[ uniform_index ].location = glGetUniformLocation( program->pid, name );
	
	return uniform_index;
}


unsigned char PROGRAM_add_vertex_attrib( PROGRAM *program, char *name, unsigned int type )
{
	unsigned char vertex_attrib_index = program->vertex_attrib_count;
	
	++program->vertex_attrib_count;

	program->vertex_attrib_array = ( VERTEX_ATTRIB * ) realloc( program->vertex_attrib_array,
																program->vertex_attrib_count * sizeof( VERTEX_ATTRIB ) );

	memset( &program->vertex_attrib_array[ vertex_attrib_index ], 0, sizeof( VERTEX_ATTRIB ) );
	
	strcpy( program->vertex_attrib_array[ vertex_attrib_index ].name, name );
	
	program->vertex_attrib_array[ vertex_attrib_index ].type = type;
	
	program->vertex_attrib_array[ vertex_attrib_index ].location = glGetAttribLocation( program->pid, name );
	
	return vertex_attrib_index;
}


unsigned char PROGRAM_link( PROGRAM *program, unsigned char debug )
{
	unsigned int i = 0,
				 type;
	
	char *log,
		 name[ MAX_CHAR ];
	
	int status,
		len,
		total,
		size;
	
	if( program->pid ) return 0;
	
	program->pid = glCreateProgram();
	
	glAttachShader( program->pid, program->vertex_shader->sid );

	glAttachShader( program->pid, program->fragment_shader->sid );
	
	if( program->programbindattribcallback ) program->programbindattribcallback( program );
	
	glLinkProgram( program->pid );
	
	
	if( debug )
	{
		glGetProgramiv( program->pid, GL_INFO_LOG_LENGTH, &len );
		
		if( len )
		{
			log = ( char * ) malloc( len );
			
			glGetProgramInfoLog( program->pid, len, &len, log );
			
			#ifdef __IPHONE_4_0
			
				printf("[ %s ]\n%s", program->name, log );
			#else			
				__android_log_print( ANDROID_LOG_ERROR, "", "[ %s ]\n%s", program->name, log );
			#endif

			free( log );
		}
	}
	
    glGetProgramiv( program->pid, GL_LINK_STATUS, &status );
	
    if( !status ) goto delete_program;
	
	
	if( debug )
	{
		glValidateProgram( program->pid );
		
		glGetProgramiv( program->pid, GL_INFO_LOG_LENGTH, &len );
		
		if( len )
		{
			log = ( char * ) malloc( len );
			
			glGetProgramInfoLog( program->pid, len, &len, log );
			
			#ifdef __IPHONE_4_0
			
				printf("[ %s ]\n%s", program->name, log );
			#else
				__android_log_print( ANDROID_LOG_ERROR, "", "[ %s ]\n%s", program->name, log );
			#endif

			free( log );
		}
		
		
		glGetProgramiv( program->pid, GL_VALIDATE_STATUS, &status );
		
		if( !status ) goto delete_program;
	}


	glGetProgramiv( program->pid, GL_ACTIVE_ATTRIBUTES, &total );

	i = 0;
	while( i != total )
	{
		glGetActiveAttrib( program->pid,
						   i,
						   MAX_CHAR,
						   &len,
						   &size,
						   &type,
						   name );

		PROGRAM_add_vertex_attrib( program, name, type );
	
		++i;
	}

	glGetProgramiv( program->pid, GL_ACTIVE_UNIFORMS, &total );

	i = 0;
	while( i != total )
	{
		glGetActiveUniform( program->pid,
							i,
							MAX_CHAR,
							&len,
							&size,
							&type,
							name );
	
		PROGRAM_add_uniform( program, name, type );
	
		++i;
	}
	
	return 1;

	
delete_program:

	PROGRAM_delete_id( program );
	
	return 0;	
}


void PROGRAM_set_draw_callback( PROGRAM *program, PROGRAMDRAWCALLBACK *programdrawcallback )
{ program->programdrawcallback = programdrawcallback; }


void PROGRAM_set_bind_attrib_location_callback( PROGRAM *program, PROGRAMBINDATTRIBCALLBACK *programbindattribcallback )
{ program->programbindattribcallback = programbindattribcallback; }


char PROGRAM_get_vertex_attrib_location( PROGRAM *program, char *name )
{
	unsigned char i = 0;
	
	while( i != program->vertex_attrib_count )
	{
		if( !strcmp( program->vertex_attrib_array[ i ].name, name ) )
		{ return program->vertex_attrib_array[ i ].location; }
		
		++i;
	}

	return -1;
}


char PROGRAM_get_uniform_location( PROGRAM *program, char *name )
{
	unsigned char i = 0;
	
	while( i != program->uniform_count )
	{
		if( !strcmp( program->uniform_array[ i ].name, name ) )
		{ return program->uniform_array[ i ].location; }
		
		++i;
	}

	return -1;
}


void PROGRAM_delete_id( PROGRAM *program )
{
	if( program->pid )
	{
		glDeleteProgram( program->pid );

		program->pid = 0;
	}
}


void PROGRAM_draw( PROGRAM *program )
{
	glUseProgram( program->pid );
	
	if( program->programdrawcallback ) program->programdrawcallback( program );	
}


unsigned char PROGRAM_load_gfx( PROGRAM						*program,
								PROGRAMBINDATTRIBCALLBACK	*programbindattribcallback,
								PROGRAMDRAWCALLBACK			*programdrawcallback,
								char						*filename,
								unsigned char				 debug_shader,
								unsigned char				 relative_path )
{
	MEMORY *m = mopen( filename, relative_path );
	
	if( m )
	{
		char vertex_token  [ MAX_CHAR ] = { "GL_VERTEX_SHADER"   },
			 fragment_token[ MAX_CHAR ] = { "GL_FRAGMENT_SHADER" },
			 *vertex_shader				= strstr( ( char * )m->buffer, vertex_token   ),
			 *fragment_shader			= strstr( ( char * )m->buffer, fragment_token );

		
		get_file_name( filename, program->name );
		
		
		if( ( vertex_shader && fragment_shader ) && ( fragment_shader > vertex_shader ) )
		{
			program->vertex_shader = SHADER_init( program->name, GL_VERTEX_SHADER );

			vertex_shader += strlen( vertex_token );
			
			*fragment_shader = 0;
			
			SHADER_compile( program->vertex_shader,
							vertex_shader,
							debug_shader );
		

			program->fragment_shader = SHADER_init( program->name, GL_FRAGMENT_SHADER );
			
			fragment_shader += strlen( fragment_token );
			
			SHADER_compile( program->fragment_shader,
							fragment_shader,
							debug_shader );

			
			program->programbindattribcallback = programbindattribcallback;
			
			program->programdrawcallback = programdrawcallback;
			
			PROGRAM_link( program, debug_shader );			
		}
		
		mclose( m );
		
		return 1;
	}
	
	return 0;
}

