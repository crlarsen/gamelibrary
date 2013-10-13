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
							templateAppDraw,
							templateAppToucheBegan,
							templateAppToucheMoved };

int viewport_matrix[ 4 ];


LIGHT *light = NULL;


vec2 touche = { 0.0f, 0.0f }; 

vec3 rot_angle = { 0.0f, 0.0f, 0.0f };

unsigned char auto_rotate = 0; 


void program_bind_attrib_location( void *ptr ) {

	PROGRAM *program = ( PROGRAM * )ptr;

	glBindAttribLocation( program->pid, 0, "POSITION"  );
	glBindAttribLocation( program->pid, 1, "NORMAL"    );
	glBindAttribLocation( program->pid, 2, "TEXCOORD0" );
	glBindAttribLocation( program->pid, 3, "TANGENT0"  );
}


void material_draw( void *ptr )
{
	OBJMATERIAL *objmaterial = ( OBJMATERIAL * )ptr;
	
	PROGRAM *program = objmaterial->program;


	unsigned int i = 0;
	
	while( i != program->uniform_count )
	{
		if( program->uniform_array[ i ].constant ) 
		{
			++i;
			continue;
		}
	
		else if( !strcmp( program->uniform_array[ i ].name, "DIFFUSE" ) )
		{
			glUniform1i( program->uniform_array[ i ].location,
						 1 );
			
			program->uniform_array[ i ].constant = 1;
		}

		else if( !strcmp( program->uniform_array[ i ].name, "BUMP" ) )
		{
			glUniform1i( program->uniform_array[ i ].location,
						 4 );
						 
			program->uniform_array[ i ].constant = 1;
		}

		// Matrix Data
		else if( !strcmp( program->uniform_array[ i ].name, "MODELVIEWMATRIX" ) )
		{
			glUniformMatrix4fv( program->uniform_array[ i ].location,
								1,
								GL_FALSE,
								( float * )GFX_get_modelview_matrix() );			
		}

		else if( !strcmp( program->uniform_array[ i ].name, "PROJECTIONMATRIX" ) )
		{
			glUniformMatrix4fv( program->uniform_array[ i ].location,
								1,
								GL_FALSE,
								( float * )GFX_get_projection_matrix() );
			
			program->uniform_array[ i ].constant = 1;
		}

		else if( !strcmp( program->uniform_array[ i ].name, "NORMALMATRIX" ) )
		{
			glUniformMatrix3fv( program->uniform_array[ i ].location,
								1,
								GL_FALSE,
								( float * )GFX_get_normal_matrix() );			
		}


		// Material Data
		else if( !strcmp( program->uniform_array[ i ].name, "MATERIAL.ambient" ) )
		{
			glUniform4fv( program->uniform_array[ i ].location,
						  1,
						  ( float * )&objmaterial->ambient );
						 
			program->uniform_array[ i ].constant = 1;
		}		

		else if( !strcmp( program->uniform_array[ i ].name, "MATERIAL.diffuse" ) )
		{
			glUniform4fv( program->uniform_array[ i ].location,
						  1,
						  ( float * )&objmaterial->diffuse );
		}		

		else if( !strcmp( program->uniform_array[ i ].name, "MATERIAL.specular" ) )
		{
			glUniform4fv( program->uniform_array[ i ].location,
						  1,
						  ( float * )&objmaterial->specular );
		}		

		else if( !strcmp( program->uniform_array[ i ].name, "MATERIAL.shininess" ) )
		{
			glUniform1f( program->uniform_array[ i ].location,
						 objmaterial->specular_exponent * 0.128f );

			program->uniform_array[ i ].constant = 1;
		}
		

		// Lamp Data
		else if( !strcmp( program->uniform_array[ i ].name, "LIGHT_FS.color" ) )
		{
			glUniform4fv( program->uniform_array[ i ].location,
						  1,
						  ( float * )&light->color );

			program->uniform_array[ i ].constant = 1;						  
		}

		else if( !strcmp( program->uniform_array[ i ].name, "LIGHT_VS.direction" ) )
		{
			vec3 direction;
			
			LIGHT_get_direction_in_eye_space( light,
											  &gfx.modelview_matrix[ gfx.modelview_matrix_index - 1 ],
											  &direction );
			
			glUniform3fv( program->uniform_array[ i ].location,
						  1,
						  ( float * )&direction );
						  
			program->uniform_array[ i ].constant = 1;
		}

		++i;
	}
}


void templateAppInit( int width, int height ) {

	atexit( templateAppExit );

	GFX_start();

	glViewport( 0.0f, 0.0f, width, height );
	
	glGetIntegerv( GL_VIEWPORT, viewport_matrix );
	
	vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
	
	light = LIGHT_create_directional( ( char * )"point", &color, 45.0f, 0.0f, 0.0f );
}


void templateAppDraw( void ) {

	glClearColor( 0.5f, 0.5f, 0.5f, 1.0f );
	glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );


	GFX_set_matrix_mode( PROJECTION_MATRIX );
	GFX_load_identity();
	
	GFX_set_perspective( 45.0f,
						 ( float )viewport_matrix[ 2 ] / ( float )viewport_matrix[ 3 ],
						 0.1f,
						 100.0f,
						 0.0f );


	GFX_set_matrix_mode( MODELVIEW_MATRIX );
	GFX_load_identity();

	GFX_translate( 0.0f, -14.0f, 3.0f );

	GFX_rotate( 90.0, 1.0f, 0.0f, 0.0f );
	
	mat4_invert( GFX_get_modelview_matrix() );
}


void templateAppToucheBegan( float x, float y, unsigned int tap_count ) {

	if( tap_count == 2 ) auto_rotate = !auto_rotate;

	touche.x = x;
	touche.y = y;
}


void templateAppToucheMoved( float x, float y, unsigned int tap_count ) {

	auto_rotate = 0;

	rot_angle.z += x - touche.x;
	rot_angle.x += y - touche.y;

	touche.x = x;
	touche.y = y;
}


void templateAppExit( void ) {

	light = LIGHT_free( light );
}
