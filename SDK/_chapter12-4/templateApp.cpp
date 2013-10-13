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


#define MTL_FILE ( char * )"bob.mtl"

#define MD5_MESH ( char * )"bob.md5mesh"


MD5 *md5 = NULL;

OBJ *obj = NULL;

MD5ACTION *idle = NULL;

MD5ACTION *walk = NULL;

MD5JOINT *final_pose = NULL;


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
	
	
	obj = ( OBJ * ) calloc( 1, sizeof( OBJ ) );
	
	OBJ_load_mtl( obj, MTL_FILE, 1 );
	
	unsigned int i = 0;
	
	while( i != obj->n_texture ) { 

		OBJ_build_texture( obj,
						   i,
						   obj->texture_path,
						   TEXTURE_MIPMAP | TEXTURE_CLAMP | TEXTURE_16_BITS,
						   TEXTURE_FILTER_3X,
						   0.0f );
		++i;
	}


	i = 0;
	while( i != obj->n_program ) { 
		
		OBJ_build_program( obj,
						   i,
						   program_bind_attrib_location,
						   NULL,
						   1,
						   obj->program_path );
		++i;
	}


	i = 0;
	while( i != obj->n_objmaterial ) { 

		OBJ_build_material( obj, i, NULL );
		
		OBJ_set_draw_callback_material( obj, i, material_draw );
		
		++i;
	}
	

	md5 = MD5_load_mesh( MD5_MESH, 1 );

	MD5_optimize( md5, 128 );

	MD5_build( md5 );

	i = 0;
	while( i != md5->n_mesh ) {

		MD5MESH *md5mesh = &md5->md5mesh[ i ];

		MD5_set_mesh_material( md5mesh,
							   OBJ_get_material( obj,
												 md5mesh->shader,
												 0 ) );
		++i;
	}
	
	
	MD5_load_action( md5,
					 ( char * )"idle",
					 ( char * )"bob_idle.md5anim",
					 1 );

	MD5_load_action( md5,
					 ( char * )"walk",
					 ( char * )"bob_walk.md5anim",
					 1 );


	idle = MD5_get_action( md5, ( char * )"idle", 0 );

	MD5_set_action_fps( idle, 24.0f );

	MD5_action_play( idle,
					 MD5_METHOD_SLERP,
					 1 );
					 
					 
	walk = MD5_get_action( md5, ( char * )"walk", 0 );

	MD5_set_action_fps( walk, 24.0f );
					 
	MD5_action_play( walk,
					 MD5_METHOD_SLERP,
					 1 );
	
	
	final_pose = ( MD5JOINT * ) calloc( md5->n_joint, sizeof( MD5JOINT ) );
	
	MD5_free_mesh_data( md5 );

	glDisable( GL_CULL_FACE );	
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
	
	
	GFX_push_matrix();	

	if( auto_rotate ) rot_angle.z += 1.0f;
							
	GFX_rotate( rot_angle.x, 1.0f, 0.0f, 0.0f );
	GFX_rotate( rot_angle.z, 0.0f, 0.0f, 1.0f );


 	static unsigned char blend_direction = 0;
	
	static float blend_factor = 0.0f;
	
	
	if( !blend_direction ) blend_factor += 0.001f;
	
	else blend_factor -= 0.001f;

	if( blend_factor < 0.0f || blend_factor > 1.0f )
	{ blend_direction = !blend_direction; }
	
	MD5_draw_action( md5, 1.0f / 60.0f );
	
	MD5_add_pose( md5,
				  final_pose,
				  idle,
				  walk,
				  MD5_METHOD_SLERP,
				  CLAMP( blend_factor, 0.0f, 1.0f ) );
	
	MD5_set_pose( md5, final_pose );
	
	
	MD5_draw( md5 );
	
	GFX_pop_matrix();
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

	obj = OBJ_free( obj );
	
	md5 = MD5_free( md5 );
	
	light = LIGHT_free( light );
}
