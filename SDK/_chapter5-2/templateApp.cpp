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

#define OBJ_FILE ( char * )"ram.obj"

#define VERTEX_SHADER ( char * )"vertex.glsl"

#define FRAGMENT_SHADER ( char * )"fragment.glsl"

OBJ *obj = NULL;

unsigned char auto_rotate = 0; 

vec2 touche = { 0.0f, 0.0f }; 

vec3 rot_angle = { 0.0f, 0.0f, 0.0f };

TEMPLATEAPP templateApp = { templateAppInit,
							templateAppDraw,
							templateAppToucheBegan,
							templateAppToucheMoved };


void program_bind_attrib_location( void *ptr ) {

	PROGRAM *program = ( PROGRAM * )ptr;

	glBindAttribLocation( program->pid, 0, "POSITION"  );
	glBindAttribLocation( program->pid, 2, "TEXCOORD0" );
}


void material_draw_callback( void *ptr )
{
	OBJMATERIAL *objmaterial = ( OBJMATERIAL * )ptr;
	
	PROGRAM *program = objmaterial->program;
	
	unsigned int i = 0;

	while( i != program->uniform_count ) {

		if( !strcmp( program->uniform_array[ i ].name, "MODELVIEWMATRIX" ) ) {
		
			glUniformMatrix4fv( program->uniform_array[ i ].location,
								1,
								GL_FALSE,
								( float * )GFX_get_modelview_matrix() ); }

		else if( !strcmp( program->uniform_array[ i ].name, "PROJECTIONMATRIX" ) ) {
		
			glUniformMatrix4fv( program->uniform_array[ i ].location,
								1,
								GL_FALSE,
								( float * )GFX_get_projection_matrix() ); }

		else if( !strcmp( program->uniform_array[ i ].name, "DIFFUSE" ) &&
				 !program->uniform_array[ i ].constant ) {
		
			program->uniform_array[ i ].constant = 1;
			
			glUniform1i( program->uniform_array[ i ].location, 1 ); }


		else if( !strcmp( program->uniform_array[ i ].name, "DIFFUSE_COLOR" ) ) {

			glUniform3fv( program->uniform_array[ i ].location,
						  1,
						  ( float * )&objmaterial->diffuse );
		}

		++i;
	}
}


void templateAppInit( int width, int height ) {

	atexit( templateAppExit );

	GFX_start();

	glViewport( 0.0f, 0.0f, width, height );

	GFX_set_matrix_mode( PROJECTION_MATRIX );
	GFX_load_identity();
	GFX_set_perspective( 45.0f,
						 ( float )width / ( float )height,
						 0.1f,
						 100.0f,
						 0.0f );

	obj = OBJ_load( OBJ_FILE, 1 );

	unsigned int i = 0;

	while( i != obj->n_objmesh ) {

		console_print( "%s: %d: GL_TRIANGLES\n",
					   obj->objmesh[ i ].name,
					   obj->objmesh[ i ].objtrianglelist[ 0 ].n_indice_array );

		OBJ_optimize_mesh( obj, i, 128 );

		console_print( "%s: %d: GL_TRIANGLE_STRIP\n",
					   obj->objmesh[ i ].name,
					   obj->objmesh[ i ].objtrianglelist[ 0 ].n_indice_array );

		OBJ_build_mesh( obj, i );

		OBJ_free_mesh_vertex_data( obj, i );

		++i;
	}	


	i = 0;
	while( i != obj->n_texture ) { 

		OBJ_build_texture( obj,
						   i,
						   obj->texture_path,
						   TEXTURE_MIPMAP | TEXTURE_16_BITS | TEXTURE_16_BITS_5551,
						   TEXTURE_FILTER_2X,
						   0.0f );
		++i;
	}	


	i = 0;

	while( i != obj->n_objmaterial ) {
	
		OBJ_build_material( obj,
							i,
							PROGRAM_create( ( char * )"default",
											VERTEX_SHADER,
											FRAGMENT_SHADER,
											1,
											1,
											program_bind_attrib_location,
											NULL ) );
		
		OBJ_set_draw_callback_material( obj, i, material_draw_callback );
		
		++i;
	}
}


void templateAppDraw( void ) {

	glClearColor( 0.5f, 0.5f, 0.5f, 1.0f );
	glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );


	GFX_set_matrix_mode( MODELVIEW_MATRIX );
	GFX_load_identity(); {
	
		vec3 e = { 0.0, -3.0f, 0.0f },
			 c = { 0.0f, 0.0f, 0.0f },
			 u = { 0.0f, 0.0f, 1.0f };

		GFX_look_at( &e, &c, &u ); 
	}

	unsigned int i = 0;

	while( i != obj->n_objmesh ) {

		GFX_push_matrix();
	
		if( auto_rotate ) rot_angle.z += 2.0f;
		
		GFX_rotate( rot_angle.x, 1.0f, 0.0f, 0.0f );
		GFX_rotate( rot_angle.z, 0.0f, 0.0f, 1.0f );
	
		OBJ_draw_mesh( obj, i );

		GFX_pop_matrix();
		
		++i;
	}
}


void templateAppToucheBegan( float x, float y, unsigned int tap_count ) {

	if( tap_count == 2 ) auto_rotate = !auto_rotate;

	touche.x = x;
	touche.y = y;
}


void templateAppToucheMoved( float x, float y, unsigned int tap_count ) {

	auto_rotate = 0;

	rot_angle.z += -( touche.x - x );
	rot_angle.x += -( touche.y - y );

	touche.x = x;
	touche.y = y;
}


void templateAppExit( void ) {

	unsigned i = 0;

	while( i != obj->n_objmaterial ) { 

		SHADER_free( obj->objmaterial[ i ].program->vertex_shader );

		SHADER_free( obj->objmaterial[ i ].program->fragment_shader );

		PROGRAM_free( obj->objmaterial[ i ].program );

		++i;
	}

	OBJ_free( obj );
}
