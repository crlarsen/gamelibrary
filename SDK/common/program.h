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

#ifndef PROGRAM_H
#define PROGRAM_H


typedef struct
{
	char			name[ MAX_CHAR ];
	
	unsigned int	type;
	
	int				location;
	
	unsigned char	constant;

} UNIFORM;


typedef struct
{
	char			name[ MAX_CHAR ];
	
	unsigned int	type;
	
	int				location;
	
} VERTEX_ATTRIB;


typedef void( PROGRAMDRAWCALLBACK( void * ) );

typedef void( PROGRAMBINDATTRIBCALLBACK( void * ) );


typedef struct
{
	char						 name[ MAX_CHAR ];
	
	SHADER						 *vertex_shader;
	
	SHADER						 *fragment_shader;
	
	unsigned int				 pid;
	
	unsigned char				 uniform_count;
	
	UNIFORM						 *uniform_array;	
	
	unsigned char				 vertex_attrib_count;
	
	VERTEX_ATTRIB				 *vertex_attrib_array;
	
	PROGRAMDRAWCALLBACK			 *programdrawcallback;
	
	PROGRAMBINDATTRIBCALLBACK	 *programbindattribcallback;

} PROGRAM;


PROGRAM *PROGRAM_init( char *name );

PROGRAM *PROGRAM_free( PROGRAM *program );

PROGRAM *PROGRAM_create( char *name, char *vertex_shader_filename, char *fragment_shader_filename, unsigned char relative_path, unsigned char debug_shader, PROGRAMBINDATTRIBCALLBACK *programbindattribcallback, PROGRAMDRAWCALLBACK *programdrawcallback );

unsigned char PROGRAM_link( PROGRAM *program, unsigned char debug );

void PROGRAM_set_draw_callback( PROGRAM *program, PROGRAMDRAWCALLBACK *programdrawcallback );

void PROGRAM_set_bind_attrib_location_callback( PROGRAM *program, PROGRAMBINDATTRIBCALLBACK *programbindattribcallback );

char PROGRAM_get_vertex_attrib_location( PROGRAM *program, char *name );

char PROGRAM_get_uniform_location( PROGRAM *program, char *name );

void PROGRAM_delete_id( PROGRAM *program );

void PROGRAM_draw( PROGRAM *program );

void PROGRAM_reset( PROGRAM *program );

unsigned char PROGRAM_load_gfx( PROGRAM *program, PROGRAMBINDATTRIBCALLBACK	*programbindattribcallback, PROGRAMDRAWCALLBACK	*programdrawcallback, char *filename, unsigned char	debug_shader, unsigned char relative_path );

#endif
