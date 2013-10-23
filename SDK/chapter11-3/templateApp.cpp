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

OBJ *obj = NULL;

TEMPLATEAPP templateApp = { templateAppInit,
							templateAppDraw };

OBJMESH *objmesh = NULL;

int viewport_matrix[ 4 ];

LIGHT *light = NULL;


vec3 center  = { 0.0f, 0.0f, 0.0f },
	 up_axis = { 0.0f, 0.0f, 1.0f };


mat4 projector_matrix;

/* Variable to remember the original frame buffer ID.  On iOS, by default,
 * the screen uses a frame buffer, but not on Android.  This will allow you to
 * either set back the original frame buffer ID, or to simply detach the frame
 * buffer (by passing the value 0).
 */
int main_buffer;

unsigned int depth_texture, // Depth texture ID
             shadowmap_buffer,  // Frame buffer ID used for creating the shadow
                                // map in real time.
             shadowmap_width  = 128,    // The width and height of the
             shadowmap_height = 256;    // depth texture that will be
                                        // attached to the frame buffer.
                                        // The higher the width and
                                        // height, the smoother the
                                        // shadow will be, at the cost
                                        // of performance and more
                                        // video memory usage.

void program_bind_attrib_location( void *ptr ) {

	PROGRAM *program = ( PROGRAM * )ptr;

	glBindAttribLocation( program->pid, 0, "POSITION"  );
	glBindAttribLocation( program->pid, 1, "NORMAL"    );
	glBindAttribLocation( program->pid, 2, "TEXCOORD0" );
	glBindAttribLocation( program->pid, 3, "TANGENT0"  );
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
		
		else if( !strcmp( program->uniform_array[ i ].name, "PROJECTOR" ) )
		{
			glUniform1i( program->uniform_array[ i ].location,
						 0 );

			program->uniform_array[ i ].constant = 1;
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
		
		else if( !strcmp( program->uniform_array[ i ].name, "PROJECTORMATRIX" ) )
		{
			glUniformMatrix4fv( program->uniform_array[ i ].location,
								1,
								GL_FALSE,
								( float * )&projector_matrix );			
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
			glUniform4fv( program->uniform_array[ i ].location,
						  1,
						  ( float * )&objmesh->current_material->diffuse );

			program->uniform_array[ i ].constant = 1;
		}		

		else if( !strcmp( program->uniform_array[ i ].name, "MATERIAL.specular" ) )
		{
			glUniform4fv( program->uniform_array[ i ].location,
						  1,
						  ( float * )&objmesh->current_material->specular );

			program->uniform_array[ i ].constant = 1;
		}		

		else if( !strcmp( program->uniform_array[ i ].name, "MATERIAL.shininess" ) )
		{
			glUniform1f( program->uniform_array[ i ].location,
						 objmesh->current_material->specular_exponent * 0.128f );

			program->uniform_array[ i ].constant = 1;
		}
		
		
		// Light Data
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
		
		else if( !strcmp( program->uniform_array[ i ].name, "LIGHT_VS.spot_direction" ) )
		{
			vec3 direction;
			
			vec3_diff( &light->spot_direction, &center, ( vec3 * )&light->position );
		
			vec3_normalize( &light->spot_direction,
							&light->spot_direction );

			LIGHT_get_direction_in_object_space( light,
												 &gfx.modelview_matrix[ gfx.modelview_matrix_index - 1 ],
												 &direction );

			glUniform3fv( program->uniform_array[ i ].location,
						  1,
						  ( float * )&direction );
		}
		
		else if( !strcmp( program->uniform_array[ i ].name, "LIGHT_FS.spot_cos_cutoff" ) )
		{
			glUniform1f( program->uniform_array[ i ].location,
						 light->spot_cos_cutoff );

			program->uniform_array[ i ].constant = 1;
		}


		else if( !strcmp( program->uniform_array[ i ].name, "LIGHT_FS.spot_blend" ) )
		{
			glUniform1f( program->uniform_array[ i ].location,
						 light->spot_blend );

			program->uniform_array[ i ].constant = 1;
		}		
		
		++i;
	}
}


void templateAppInit( int width, int height ) {

	atexit( templateAppExit );

	glViewport( 0.0f, 0.0f, width, height );

	glGetIntegerv( GL_VIEWPORT, viewport_matrix );

	GFX_start();

	obj = OBJ_load( OBJ_FILE, 1 );

	unsigned int i = 0;

	while( i != obj->n_objmesh ) {
		
		OBJ_optimize_mesh( obj, i, 128 );

		OBJ_build_mesh2( obj, i );

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

	light = LIGHT_create_spot( ( char * )"spot", &color, &position, 0.0f, 0.0f, 0.0f, 75.0f, 0.05f );
	

	OBJ_get_mesh( obj, ( char * )"projector", 0 )->visible = 0;

    /* Get the current frame buffer ID that is bound to the current GL
     * context.  If there is none, the value returned will be less than
     * 0 (if you are using Android); in this case, clamp the main_buffer
     * variable to 0 to unbind the framebuffer.  If you are running the
     * app on iOS you will get the current frame buffer ID and will
     * reuse it to switch between the shadowmap_buffer ID and the
     * original ID created by the system.  Very convenient!
     */
    glGetIntegerv( GL_FRAMEBUFFER_BINDING, &main_buffer );

    if( main_buffer < 0 ) main_buffer = 0;

    /* Generate a new frame buffer ID. */
    glGenFramebuffers( 1, &shadowmap_buffer );

    /* Bind the new framebuffer ID. */
    glBindFramebuffer( GL_FRAMEBUFFER, shadowmap_buffer );

    /* Create a new texture ID. */
    glGenTextures( 1, &depth_texture );

    /* Bind the new texture ID. */
    glBindTexture( GL_TEXTURE_2D, depth_texture );

    /* Set the magnification and minification filters to not use
     * interpolation.
     */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

    /* Set the texture wrap to be clamped to the edge of the texture.
     * Just like in the projector tutorial, this will force the UV
     * values to stay in the range of 0 to 1.
     */
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

    /* Create a blank depth texture using the size of the shadow map
     * specified above.  Note that your GL implementation needs to
     * have the extension GL_OES_depth_texture available for this
     * tutorial to work and be able to create a texture using the
     * GL_DEPTH_COMPONENT pixel format.
     */
    glTexImage2D( GL_TEXTURE_2D,
                  0,
                  GL_DEPTH_COMPONENT,
                  shadowmap_width,
                  shadowmap_height,
                  0,
                  GL_DEPTH_COMPONENT,
                 /* Request a 16-bit depth buffer. */
                  GL_UNSIGNED_SHORT,
                  NULL );

    /* Unbind the texture. */
    glBindTexture( GL_TEXTURE_2D, 0 );

    /* Attach the depth texture to the frame buffer.  This will allow
     * you to use this texture as a depth buffer.
     */
    glFramebufferTexture2D( GL_FRAMEBUFFER,
                            GL_DEPTH_ATTACHMENT,
                            GL_TEXTURE_2D,
                            depth_texture,
                            0 );
}


void draw_scene_from_projector( void )
{
	GFX_set_matrix_mode( PROJECTION_MATRIX );
	GFX_load_identity();
	
	GFX_set_perspective( light->spot_fov,
						 ( float )viewport_matrix[ 2 ] / ( float )viewport_matrix[ 3 ],
						 1.0f,
						 20.0f,
						 -90.0f );
	
	GFX_set_matrix_mode( MODELVIEW_MATRIX );
	GFX_load_identity();
	
	GFX_look_at( ( vec3 * )&light->position, &center, &up_axis );

	projector_matrix.m[ 0 ].x = 0.5f;
	projector_matrix.m[ 0 ].y = 0.0f; 
	projector_matrix.m[ 0 ].z = 0.0f;
	projector_matrix.m[ 0 ].w = 0.0f;

	projector_matrix.m[ 1 ].x = 0.0f;
	projector_matrix.m[ 1 ].y = 0.5f;
	projector_matrix.m[ 1 ].z = 0.0f;
	projector_matrix.m[ 1 ].w = 0.0f;

	projector_matrix.m[ 2 ].x = 0.0f;
	projector_matrix.m[ 2 ].y = 0.0f;
	projector_matrix.m[ 2 ].z = 0.5f;
	projector_matrix.m[ 2 ].w = 0.0f;

	projector_matrix.m[ 3 ].x = 0.5f;
	projector_matrix.m[ 3 ].y = 0.5f;
	projector_matrix.m[ 3 ].z = 0.5f;
	projector_matrix.m[ 3 ].w = 1.0f;

	mat4_multiply_mat4( &projector_matrix, &projector_matrix, GFX_get_modelview_projection_matrix() );

    /* Bind the shadowmap buffer to redirect the drawing to the shadowmap
     * frame buffer.
     */
    glBindFramebuffer( GL_FRAMEBUFFER, shadowmap_buffer );

    /* Resize the viewport to fit the shadow map width and height. */
    glViewport( 0, 0, shadowmap_width, shadowmap_height );

    /* Clear the depth buffer, which will basically clear the content
     * of the depth_texture.
     */
    glClear( GL_DEPTH_BUFFER_BIT );

    /* Cull the front faces.  Because you are trying to render real-time
     * shadows, you are only interested in the back face of the object,
     * which basically is the surface that casts the shadow.  By culling
     * the front face, you will be able to cast shadows for the allow
     * objects to cast shadows on themselves.
     */
    glCullFace( GL_FRONT );

    unsigned int i = 0;

    /* Get the writedepth shader program. */
    PROGRAM *program = OBJ_get_program( obj, "writedepth", 0 );
    
    /* Assign the shader to all materials. */
    while( i != obj->n_objmaterial )
    {
        obj->objmaterial[ i ].program = program;
        ++i;
    }

    /* Draw the scene as you normally do.  This will fill the
     * depth_texture values.
     */
    i = 0;
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

    /* Restore that back faces should be culled. */
    glCullFace( GL_BACK );
}


void draw_scene( void )
{
    glBindFramebuffer( GL_FRAMEBUFFER, main_buffer );

	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
	glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );

	glViewport( 0, 0, viewport_matrix[ 2 ], viewport_matrix[ 3 ] );

	GFX_set_matrix_mode( PROJECTION_MATRIX );
	GFX_load_identity();
	
	GFX_set_perspective( 45.0f,
						 ( float )viewport_matrix[ 2 ] / ( float )viewport_matrix[ 3 ],
						 0.1f,
						 100.0f,
						 -90.0f );
	
	GFX_set_matrix_mode( MODELVIEW_MATRIX );
	GFX_load_identity();

	GFX_translate( 14.0f, -12.0f, 7.0f );

	GFX_rotate( 48.5f, 0.0f, 0.0f, 1.0f );

	GFX_rotate( 72.0, 1.0f, 0.0f, 0.0f );
	
	mat4_invert( GFX_get_modelview_matrix() );
	

	mat4 projector_matrix_copy;
	
	mat4_copy_mat4( &projector_matrix_copy, &projector_matrix );
	
	
	unsigned int i = 0;

    /* Get the lighting shader program. */
    PROGRAM *program = OBJ_get_program( obj, "lighting", 0 );

    /* Attach the lighting program to all materials for the current OBJ
     * structure.
     */
    while( i != obj->n_objmaterial )
    {
        obj->objmaterial[ i ].program = program;
        ++i;
    }
    
    /* Bind and make the depth_texture active on the texture channel 0. */
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, depth_texture );

    /* Reset the counter to loop through the objects. */
    i = 0;
	while( i != obj->n_objmesh ) {

		objmesh = &obj->objmesh[ i ];

		GFX_push_matrix();

		GFX_translate( objmesh->location.x,
					   objmesh->location.y,
					   objmesh->location.z );


		mat4_copy_mat4( &projector_matrix, &projector_matrix_copy );
		
		mat4_translate( &projector_matrix, &projector_matrix, &objmesh->location );


		OBJ_draw_mesh( obj, i );

		GFX_pop_matrix();
		
		++i;
	}
}


void templateAppDraw( void ) {

	draw_scene_from_projector();

	draw_scene();
}


void templateAppExit( void ) {
    glDeleteFramebuffers(1, &shadowmap_buffer);
    glDeleteTextures(1, &depth_texture);

	light = LIGHT_free( light );
	
	OBJ_free( obj );
}