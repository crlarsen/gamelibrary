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


unsigned int colorbuffer_texture = 0,
			 colorbuffer_width	 = 128,
			 colorbuffer_height	 = 256;
			 
float blur_radius = 2.0f;

OBJMESH *fullscreen = NULL;

unsigned char pass = 0;


#define OBJ_FILE ( char * )"Scene.obj"

OBJ *obj = NULL;

TEMPLATEAPP templateApp = { templateAppInit,
							templateAppDraw };

OBJMESH *objmesh = NULL;

int viewport_matrix[ 4 ];

LIGHT *light = NULL;


void program_bind_attrib_location( void *ptr ) {

	PROGRAM *program = ( PROGRAM * )ptr;

	glBindAttribLocation( program->pid, 0, "POSITION"  );
	glBindAttribLocation( program->pid, 1, "NORMAL"    );
	glBindAttribLocation( program->pid, 2, "TEXCOORD0" );
	glBindAttribLocation( program->pid, 3, "TANGENT0" );
}


void program_draw( void *ptr )
{
	unsigned int i = 0;
	
	PROGRAM *program = ( PROGRAM * )ptr;
	
	while( i != program->uniform_count )
	{
		if( program->uniform_array[ i ].constant ) 
		{
			++i;
			continue;
		}
	
		else if( !strcmp( program->uniform_array[ i ].name, "MODELVIEWPROJECTIONMATRIX" ) )
		{
			glUniformMatrix4fv( program->uniform_array[ i ].location,
								1,
								GL_FALSE,
								( float * )GFX_get_modelview_projection_matrix() );			
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
						  ( float * )&objmesh->current_material->ambient );
						 
			program->uniform_array[ i ].constant = 1;
		}		

		else if( !strcmp( program->uniform_array[ i ].name, "MATERIAL.diffuse" ) )
		{
			if( pass == 1 )
			{
				vec4 black = { 0.0f, 0.0f, 0.0f, 1.0f };
				
				glUniform4fv( program->uniform_array[ i ].location,
							  1,
							  ( float * )&black );
			}
			else 
			{
				glUniform4fv( program->uniform_array[ i ].location,
							  1,
							  ( float * )&objmesh->current_material->diffuse );
			}
		}		

		else if( !strcmp( program->uniform_array[ i ].name, "MATERIAL.specular" ) )
		{
			if( pass == 2 )
			{
				vec4 black = { 0.0f, 0.0f, 0.0f, 1.0f };
						   /*
						   // Or your could use half of the orignal specular color like this (which also gives good results):
						   = { objmesh->current_material->specular.x * 0.5f,
							   objmesh->current_material->specular.y * 0.5f,
							   objmesh->current_material->specular.z * 0.5f,
							   1.0f };
							*/
 						    
				glUniform4fv( program->uniform_array[ i ].location,
							  1,
							  ( float * )&black );
			}
			else
			{
				glUniform4fv( program->uniform_array[ i ].location,
							  1,
							  ( float * )&objmesh->current_material->specular );
			}
		}		

		else if( !strcmp( program->uniform_array[ i ].name, "MATERIAL.shininess" ) )
		{
			glUniform1f( program->uniform_array[ i ].location,
						 objmesh->current_material->specular_exponent * 0.128f );

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

		else if( !strcmp( program->uniform_array[ i ].name, "LIGHT_VS.position" ) )
		{
			vec4 position;
		
			static float rot_angle = 0.0f;
		
			light->position.x = 7.5f * cosf( rot_angle * DEG_TO_RAD );
			light->position.y = 7.5f * sinf( rot_angle * DEG_TO_RAD );
		
			rot_angle += 0.25f;
			
			LIGHT_get_position_in_eye_space( light,
											 &gfx.modelview_matrix[ gfx.modelview_matrix_index - 1 ], 
											 &position );
			
			glUniform3fv( program->uniform_array[ i ].location,
						  1,
						  ( float * )&position );
		}

		++i;
	}	
}


void templateAppInit( int width, int height ) {

	atexit( templateAppExit );

	GFX_start();

	glViewport( 0.0f, 0.0f, width, height );
	
	glGetIntegerv( GL_VIEWPORT, viewport_matrix );

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
						   TEXTURE_FILTER_3X,
						   0.0f );
		++i;
	}


	i = 0;
	while( i != obj->n_program ) { 
		
		OBJ_build_program( obj,
						   i,
						   program_bind_attrib_location,
						   program_draw,
						   1,
						   obj->program_path );
		++i;
	}


	i = 0;
	while( i != obj->n_objmaterial ) { 

		OBJ_build_material( obj, i, NULL );
		++i;
	}
	
	
	vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
	
	vec3 position = { 7.5f, 0.0f, 6.0f };	
	
	light = LIGHT_create_point( ( char * )"point", &color, &position );
	
	
	
	glGenTextures( 1, &colorbuffer_texture );

	glBindTexture( GL_TEXTURE_2D, colorbuffer_texture );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	
	glTexImage2D( GL_TEXTURE_2D,
				  0,
				  GL_RGB,
				  colorbuffer_width,
				  colorbuffer_height,
				  0,
				  GL_RGB,
				  GL_UNSIGNED_SHORT_5_6_5,
				  NULL );	

	fullscreen = OBJ_get_mesh( obj, "fullscreen", 0 );
	fullscreen->visible = 0;		
}


void draw_scene( void )
{
	glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );

	GFX_set_matrix_mode( MODELVIEW_MATRIX );
	GFX_load_identity();

	GFX_translate( 14.0f, -12.0f, 7.0f );

	GFX_rotate( 48.5f, 0.0f, 0.0f, 1.0f );

	GFX_rotate( 72.0, 1.0f, 0.0f, 0.0f );
	
	mat4_invert( GFX_get_modelview_matrix() );


	unsigned int i = 0;

	while( i != obj->n_objmesh ) {

		objmesh = &obj->objmesh[ i ];

		GFX_push_matrix();

		GFX_translate( objmesh->location.x,
					   objmesh->location.y,
					   objmesh->location.z );
					   
		OBJ_draw_mesh( obj, i );

		GFX_pop_matrix();
		
		++i;
	}
}


void first_pass( void )
{
	pass = 1;
	
	glViewport( 0, 0, colorbuffer_width, colorbuffer_height );

	draw_scene();

	glBindTexture( GL_TEXTURE_2D, colorbuffer_texture );
	
	glCopyTexSubImage2D( GL_TEXTURE_2D,
						 0, 0, 0, 0, 0,
						 colorbuffer_width,
						 colorbuffer_height );
}


void second_pass( void )
{
	pass = 2;
	
	glViewport( 0, 0, viewport_matrix[ 2 ], viewport_matrix[ 3 ] );
	
	draw_scene();
}


void fullscreen_pass( void )
{
	GFX_set_matrix_mode( PROJECTION_MATRIX );
	GFX_load_identity();
	
	float half_width  = ( float )viewport_matrix[ 2 ] * 0.5f,
		  half_height = ( float )viewport_matrix[ 3 ] * 0.5f;

	GFX_load_identity();
	
	GFX_set_orthographic_2d( -half_width,
							  half_width,
							 -half_height,
							  half_height );

	GFX_set_matrix_mode( MODELVIEW_MATRIX );
	GFX_load_identity();
	
	
	glDisable( GL_DEPTH_TEST );
	glDepthMask( GL_FALSE );

	glDisable( GL_CULL_FACE );

	glEnable( GL_BLEND );

	glBlendEquation( GL_FUNC_ADD );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE );


	glActiveTexture( GL_TEXTURE1 );
	
	glBindTexture( GL_TEXTURE_2D, colorbuffer_texture );
	

	fullscreen->visible = 1;
	{
		PROGRAM *program = OBJ_get_program( obj, "blur", 0 );
	
		GFX_scale( ( float )viewport_matrix[ 2 ],
				   ( float )viewport_matrix[ 3 ],
				   1.0f );
	
		GFX_rotate( 180.0f, 1.0f, 0.0f, 0.0f );


		PROGRAM_draw( program );


		vec2 radius = { blur_radius / ( float )colorbuffer_width,
						0.0f };

		glUniform2fv( PROGRAM_get_uniform_location( program, ( char * )"BLUR_RADIUS" ), 
					  1,
					  ( float * )&radius );		
		
		OBJ_draw_mesh( obj,
					   OBJ_get_mesh_index( obj, "fullscreen", 0 ) );
					

		radius.x = 0.0f;
		radius.y = blur_radius / ( float )colorbuffer_height;
		
		glUniform2fv( PROGRAM_get_uniform_location( program, ( char * )"BLUR_RADIUS" ), 
					  1,
					  ( float * )&radius );		
	
		
		
		OBJ_draw_mesh( obj,
					   OBJ_get_mesh_index( obj, "fullscreen", 0 ) );
	}
	fullscreen->visible = 0;
	

	glDisable( GL_BLEND );

	glEnable( GL_CULL_FACE );	

	glEnable( GL_DEPTH_TEST );
	glDepthMask( GL_TRUE );
}


void templateAppDraw( void ) {

	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );

	GFX_set_matrix_mode( PROJECTION_MATRIX );
	GFX_load_identity();
	
	GFX_set_perspective( 45.0f,
						 ( float )viewport_matrix[ 2 ] / ( float )viewport_matrix[ 3 ],
						 0.1f,
						 100.0f,
						 -90.0f );
	
	first_pass();
	
	second_pass();
	
	//glClear( GL_COLOR_BUFFER_BIT );
	
	fullscreen_pass();	
}


void templateAppExit( void ) {

	glDeleteTextures( 1, &colorbuffer_texture );

	light = LIGHT_free( light );

	OBJ_free( obj );
}
