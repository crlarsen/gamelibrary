/*

Book:      	Game and Graphics Programming for iOS and Android with OpenGL(R) ES 2.0
Author:    	Romain Marucchi-Foino
ISBN-10: 	1119975913
ISBN-13: 	978-1119975915
Publisher: 	John Wiley & Sons	

Copyright (C) 2011 Romain Marucchi-Foino

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of
this software. Permission is granted to anyone who either own or purchase a copy of
the book specified above, to use this software for any purpose, including commercial
applications subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that
you wrote the original software. If you use this software in a product, an acknowledgment
in the product would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be misrepresented
as being the original software.

3. This notice may not be removed or altered from any source distribution.

*/

#include "templateApp.h"

TEMPLATEAPP templateApp = { templateAppInit,
							templateAppDraw };

#define VERTEX_SHADER ( char * )"vertex.glsl"

#define FRAGMENT_SHADER ( char * )"fragment.glsl"

#define DEBUG_SHADERS 1

PROGRAM *program = NULL;

MEMORY *m = NULL;

void templateAppInit( int width, int height )
{
	atexit( templateAppExit );

	GFX_start();

	glViewport( 0.0f, 0.0f, width, height );

	GFX_set_matrix_mode( PROJECTION_MATRIX );
	{
		GFX_load_identity();
		
		GFX_set_perspective( 45.0f,
							 ( float )width / ( float )height,
							 0.01f,
							 100.0f,
							 0.0f );
		
		glDisable( GL_CULL_FACE );
	}
	
	program = PROGRAM_init( ( char * )"default" );
	
	program->vertex_shader = SHADER_init( VERTEX_SHADER, GL_VERTEX_SHADER );
   
	program->fragment_shader = SHADER_init( FRAGMENT_SHADER, GL_FRAGMENT_SHADER );	
	
	m = mopen( VERTEX_SHADER, 1 );
	
	if( m ) {
	
		if( !SHADER_compile( program->vertex_shader,
						     ( char * )m->buffer,
							 DEBUG_SHADERS ) ) exit( 1 );
	}
	m = mclose( m );

	m = mopen( FRAGMENT_SHADER, 1 );
	
	if( m ) {
	
		if( !SHADER_compile( program->fragment_shader,
						     ( char * )m->buffer,
						     DEBUG_SHADERS ) ) exit( 2 ); 
	}
	  
	m = mclose( m );

   if( !PROGRAM_link( program, DEBUG_SHADERS ) ) exit( 3 );
}


void templateAppDraw( void )
{
	static const float POSITION[ 12 ] = {
	-0.5f, 0.0f, -0.5f, // Bottom left
	 0.5f, 0.0f, -0.5f,
	-0.5f, 0.0f,  0.5f,
	 0.5f, 0.0f,  0.5f // Top right
	};
	
	static const float COLOR[ 16 ] = {
	1.0f, 0.0f, 0.0f, 1.0f, // Red
	0.0f, 1.0f, 0.0f, 1.0f, // Green
	0.0f, 0.0f, 1.0f, 1.0f, // Blue
	1.0f, 1.0f, 0.0f, 1.0f  // Yellow
	};	

	glClearColor( 0.5f, 0.5f, 0.5f, 1.0f );

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	
	GFX_set_matrix_mode( MODELVIEW_MATRIX );
	
	GFX_load_identity();

	vec3 e = { 0.0f, -3.0f, 0.0f },
		 c = { 0.0f,  0.0f, 0.0f },
		 u = { 0.0f,  0.0f, 1.0f };

	GFX_look_at( &e, &c, &u );

	static float y = 0.0f;
	
	y += 0.1f;
	
	//GFX_translate( 0.0f, y, 0.0f );

	GFX_rotate( y * 50.0f,
				1.0f,
				1.0f,
				1.0f );
		
	if( program->pid ) {
	
		char attribute, uniform;
		
		glUseProgram( program->pid );
	
		uniform = PROGRAM_get_uniform_location( program, 
												( char * )"MODELVIEWPROJECTIONMATRIX" );
		
		glUniformMatrix4fv( uniform,
							1 /* How many 4x4 matrix */,
							GL_FALSE /* Transpose the matrix? */, 
							( float * )GFX_get_modelview_projection_matrix() );		

		attribute = PROGRAM_get_vertex_attrib_location( program,
														( char * )"POSITION" );

		glEnableVertexAttribArray( attribute );
		
		glVertexAttribPointer( attribute, 3, GL_FLOAT, GL_FALSE, 0, POSITION );
		
		attribute = PROGRAM_get_vertex_attrib_location( program,
														( char * )"COLOR" );

		glEnableVertexAttribArray( attribute );
		
		glVertexAttribPointer( attribute, 4, GL_FLOAT, GL_FALSE, 0, COLOR );

		glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
	}		
}


void templateAppExit( void )
{
	printf("templateAppExit...\n");
	
	if( program && program->vertex_shader ) 
		program->vertex_shader = SHADER_free( program->vertex_shader );

	if( program && program->fragment_shader )
		program->fragment_shader = SHADER_free( program->fragment_shader );

	if( program )
		program = PROGRAM_free( program );
}
