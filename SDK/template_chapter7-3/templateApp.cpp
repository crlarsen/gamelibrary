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

#define OBJ_FILE ( char * )"Scene.obj"

#define VERTEX_SHADER ( char * )"vertex.glsl"

#define FRAGMENT_SHADER ( char * )"fragment.glsl"

OBJ *obj = NULL;

PROGRAM *program = NULL;


vec3 location = { 0.0f, 0.0f, 1.84f };

float rotz = 0.0f,
	  rotx = 90.0f;

vec4 frustum[ 6 ];

TEMPLATEAPP templateApp = { templateAppInit,
							templateAppDraw,
							templateAppToucheBegan,
							templateAppToucheMoved,
							templateAppToucheEnded };


void program_bind_attrib_location( void *ptr ) {

	PROGRAM *program = ( PROGRAM * )ptr;

	glBindAttribLocation( program->pid, 0, "POSITION" );
	glBindAttribLocation( program->pid, 2, "TEXCOORD0" );
}


void templateAppInit( int width, int height ) {

	atexit( templateAppExit );

	GFX_start();

	glViewport( 0.0f, 0.0f, width, height );

	GFX_set_matrix_mode( PROJECTION_MATRIX );
	GFX_load_identity();
	GFX_set_perspective( 80.0f,
						 ( float )width / ( float )height,
						 0.1f,
						 100.0f,
						 -90.0f );

	obj = OBJ_load( OBJ_FILE, 1 );

	unsigned int i = 0;

	while( i != obj->n_objmesh ) {
	
		OBJ_optimize_mesh( obj, i, 128 );

		OBJ_build_mesh( obj, i );

		OBJ_free_mesh_vertex_data( obj, i ); 

		++i;
	}
	
	i = 0;
	while( i != obj->n_texture ) { 

		OBJ_build_texture( obj,
						   i,
						   obj->texture_path,
						   TEXTURE_MIPMAP | TEXTURE_16_BITS,
						   TEXTURE_FILTER_2X,
						   0.0f );
		++i;
	}


	i = 0;
	while( i != obj->n_objmaterial ) { 

		OBJ_build_material( obj, i, NULL );
		
		++i;
	}
		
	
	program = PROGRAM_create( ( char * )"default",
							  VERTEX_SHADER,
							  FRAGMENT_SHADER,
							  1,
							  0,
							  program_bind_attrib_location,
							  NULL );

	PROGRAM_draw( program );
	
	glUniform1i( PROGRAM_get_uniform_location( program, ( char * )"DIFFUSE" ), 1 );
}


void templateAppDraw( void ) {

	glClearColor( 0.5f, 0.5f, 0.5f, 1.0f );
	glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );


	GFX_set_matrix_mode( MODELVIEW_MATRIX );
	GFX_load_identity();


	GFX_translate( location.x, location.y, location.z );
	
	GFX_rotate( rotz, 0.0f, 0.0f, 1.0f );

	GFX_rotate( rotx, 1.0f, 0.0f, 0.0f );
	
	mat4_invert( GFX_get_modelview_matrix() );
	

	build_frustum( frustum,
				   GFX_get_modelview_matrix(),
				   GFX_get_projection_matrix() );
	

	unsigned int i = 0;

	while( i != obj->n_objmesh ) {

		OBJMESH *objmesh = &obj->objmesh[ i ];

		objmesh->distance = sphere_distance_in_frustum( frustum,
														&objmesh->location,
														objmesh->radius );

		if( objmesh->distance > 0.0f )
		{
			GFX_push_matrix();

			GFX_translate( objmesh->location.x,
						   objmesh->location.y,
						   objmesh->location.z );
						   
			glUniformMatrix4fv( program->uniform_array[ 0 ].location,
								1,
								GL_FALSE,
								( float * )GFX_get_modelview_projection_matrix() );

			OBJ_draw_mesh( obj, i );

			GFX_pop_matrix();
		}
		
		++i;
	}
}


void templateAppToucheBegan( float x, float y, unsigned int tap_count )
{

}


void templateAppToucheMoved( float x, float y, unsigned int tap_count )
{

}


void templateAppToucheEnded( float x, float y, unsigned int tap_count )
{


}


void templateAppExit( void ) {

	SHADER_free( program->vertex_shader );

	SHADER_free( program->fragment_shader );

	PROGRAM_free( program );

	OBJ_free( obj );
}
