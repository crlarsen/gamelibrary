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

#define OBJ_FILE ( char * )"scene.obj"

OBJ *obj = NULL;

TEMPLATEAPP templateApp = { templateAppInit,
							templateAppDraw };


void program_bind_attrib_location( void *ptr ) {

	PROGRAM *program = ( PROGRAM * )ptr;

	glBindAttribLocation( program->pid, 0, "POSITION"  );
	glBindAttribLocation( program->pid, 2, "TEXCOORD0" );
}


void material_draw_callback( void *ptr ) {

   OBJMATERIAL *objmaterial = ( OBJMATERIAL * )ptr;
   
   PROGRAM *program = objmaterial->program;

	unsigned int i = 0;

	while( i != program->uniform_count ) {
	
		if( !strcmp( program->uniform_array[ i ].name, "DIFFUSE" ) ) {
			
			glUniform1i( program->uniform_array[ i ].location, 1 );
		}

		else if( !strcmp( program->uniform_array[ i ].name, "MODELVIEWPROJECTIONMATRIX" ) ) {

			glUniformMatrix4fv( program->uniform_array[ i ].location,
								1,
								GL_FALSE,
								( float * )GFX_get_modelview_projection_matrix() );
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
						 -90.0f );

	obj = OBJ_load( OBJ_FILE, 1 );
	
	unsigned int i = 0;

	while( i != obj->n_objmesh ) {

		OBJ_build_mesh( obj, i );

		OBJ_free_mesh_vertex_data( obj, i ); 

		++i;
	}	


	i = 0;
	while( i != obj->n_texture ) { 

		OBJ_build_texture( obj,
						   i,
						   obj->texture_path,
						   TEXTURE_MIPMAP,
						   TEXTURE_FILTER_2X,
						   0.0f );
		++i;
	}	


	i = 0;

	MEMORY *vertex_shader = mopen( ( char * )"vertex.glsl", 1 );

	while( i != obj->n_objmaterial ) { 
	
		MEMORY *fragment_shader = mopen( ( char * )"fragment.glsl", 1 );

		OBJMATERIAL *objmaterial = &obj->objmaterial[ i ];
		
		OBJ_build_material( obj, i, NULL );		
	
		if( objmaterial->dissolve == 1.0f )
			minsert( fragment_shader, ( char * )"#define SOLID_OBJECT\n", 0 );

		else if( !objmaterial->dissolve ) 
			minsert( fragment_shader, ( char * )"#define ALPHA_TESTED_OBJECT\n", 0 );

		else
			minsert( fragment_shader, ( char * )"#define TRANSPARENT_OBJECT\n", 0 );

		objmaterial->program = PROGRAM_init( objmaterial->name );

		objmaterial->program->vertex_shader = SHADER_init( ( char * )"vertex", GL_VERTEX_SHADER );

		objmaterial->program->fragment_shader = SHADER_init( ( char * )"fragment", GL_FRAGMENT_SHADER );

		SHADER_compile( objmaterial->program->vertex_shader,
					    ( char * )vertex_shader->buffer,
						1 );
		
		SHADER_compile( objmaterial->program->fragment_shader,
						( char * )fragment_shader->buffer,
						1 );

		PROGRAM_set_bind_attrib_location_callback( objmaterial->program,
												   program_bind_attrib_location );

		PROGRAM_link( objmaterial->program, 1 );

		OBJ_set_draw_callback_material( obj, i, material_draw_callback );

		mclose( fragment_shader );
		
		++i;
	}
	
	mclose( vertex_shader );
}


void templateAppDraw( void ) {

	glClearColor( 0.5f, 0.5f, 0.5f, 1.0f );
	glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );

	GFX_set_matrix_mode( MODELVIEW_MATRIX );

	GFX_load_identity();

	vec3 e = {  0.0f, -6.0f, 1.35f }, 
		 c = {  0.0f, -5.0f, 1.35f },
		 u = {  0.0f,  0.0f, 1.0f  };
	
	GFX_look_at( &e, &c, &u );

	unsigned int i = 0;
	
	while( i != obj->n_objmesh ) { 

		OBJMATERIAL *objmaterial = obj->objmesh[ i ].objtrianglelist[ 0 ].objmaterial;

		if( objmaterial->dissolve == 1.0f ) {
		
			GFX_push_matrix();
			
			GFX_translate( obj->objmesh[ i ].location.x, 
						   obj->objmesh[ i ].location.y,
						   obj->objmesh[ i ].location.z );
			
			OBJ_draw_mesh( obj, i );
			
			GFX_pop_matrix();
		}
		
		++i;
	}
	
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	i = 0;
	while( i != obj->n_objmesh ) { 

		OBJMATERIAL *objmaterial = obj->objmesh[ i ].objtrianglelist[ 0 ].objmaterial;

		if( objmaterial->dissolve != 1.0f ) {

			GFX_push_matrix();

			GFX_translate( obj->objmesh[ i ].location.x, 
						   obj->objmesh[ i ].location.y,
						   obj->objmesh[ i ].location.z );

			glCullFace( GL_FRONT );
			OBJ_draw_mesh( obj, i );
			
			glCullFace( GL_BACK );
			OBJ_draw_mesh( obj, i );

			GFX_pop_matrix();
		}
		
		++i;
	}

	glDisable( GL_BLEND );	
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