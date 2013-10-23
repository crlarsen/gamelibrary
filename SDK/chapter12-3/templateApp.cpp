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

/* The material file. */
#define MTL_FILE ( char * )"bob.mtl"

/* The MD5 mesh file. */
#define MD5_MESH ( char * )"bob.md5mesh"

/* A fresh OBJ pointer which you will use strictly to load the materials
 * associated with the MD5 mesh.
 */
OBJ *obj = NULL;

/* An empty MD5 structure pointer.  For more information about this
 * structure, feel free to consult the file md5.cpp and its associated
 * header located inside the common directory of the SDK.
 */
MD5 *md5 = NULL;

/* To store the idle MD5ACTION pointer. */
MD5ACTION *idle = NULL;

/* The walk action pointer. */
MD5ACTION *walk = NULL;

/* The joints array that will be used to hold the final pose of the
 * skeleton after the idle and walk actions are blended together.
 */
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

    /* Manually initialize a blank OBJ structure.  You do not need to
     * use the OBJ_load function this time, because there's no geometry
     * to load, only a material file.
     */
    obj = ( OBJ * ) calloc( 1, sizeof( OBJ ) );

    /* Manually load the material file using the filename you defined at
     * the beginning of the current source file.
     */
    OBJ_load_mtl( obj, MTL_FILE, 1 );

    /* Build the textures. */
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

    /* Build the shader programs (in this case, there's only one). */
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

    /* Build the materials ans associate the material_draw callback
     * function to each of them to be able to set the uniform variables
     * of the shader program.
     */
    i = 0;
    while( i != obj->n_objmaterial ) {
        OBJ_build_material( obj, i, NULL );
        /* Set a material callback so every time the material is about
         * to be used for drawing, the material_draw function will be
         * triggered by the execution pointer.
         */
        OBJ_set_draw_callback_material( obj, i, material_draw );
        ++i;
    }

    /* Load the MD5 mesh file from disk. */
    md5 = MD5_load_mesh( MD5_MESH, 1 );

    /* Convert the triangles to triangle strips. */
    MD5_optimize( md5, 128 );

    /* Build the VBO and VAO and construct the normals and tangents for
     * each face of the meshes.
     */
    MD5_build( md5 );

    /* Loop while there are some mesh parts. */
    i = 0;
    while( i != md5->n_mesh ) {
        /* The current mesh pointer. */
        MD5MESH *md5mesh = &md5->md5mesh[ i ];

        /* Query the OBJ material database to get the objmaterial
         * pointer for the current mesh part.  Note that for the MD5
         * format, each part name is considered as a shader that
         * corresponds to the same material entry name in the OBJ
         * material file.
         */
        MD5_set_mesh_material( md5mesh,
                               OBJ_get_material( obj,
                                                 md5mesh->shader,
                                                 0 ) );
        /* Next mesh please... */
        ++i;
    }

    /* Load the actions from the disk. */
    MD5_load_action( md5,
                     ( char * )"idle",  // Internal name for this action.
                     ( char * )"bob_idle.md5anim",  // The action file name.
                     1 );   // Use a relative path to load the action file.
    MD5_load_action( md5,
                     ( char * )"walk",
                     ( char * )"bob_walk.md5anim",
                     1 );

    /* Retrieve the pointer of the idle action. */
    idle = MD5_get_action( md5, ( char * )"idle", 0 );

    /* Set the frame rate that want to use to play back the animation. */
    MD5_set_action_fps( idle, 24.0f );

    /* Start playing the animation using looping. */
//    MD5_action_play( idle,
//                    /* The method to use to interpolate between frames.
//                     * For this first example, simply use the
//                     * MD5_METHOD_FRAME method to represent that each
//                     * frame of the sequence will be played sequentially
//                     * one after the other (no interpolation between
//                     * each frame).
//                     */
//                     MD5_METHOD_FRAME,
//                    /* Specify whether or not the animation should loop
//                     * when the end frame is reached (either 1 or 0).
//                     */
//                     1 );

//    MD5_action_play( idle,
//                     MD5_METHOD_LERP,
//                     1 );

    MD5_action_play( idle,
                     MD5_METHOD_SLERP,
                     1 );

    /* Get the walk animation pointer. */
    walk = MD5_get_action( md5, ( char * )"walk", 0 );

    /* Set the rate of the animation playback. */
    MD5_set_action_fps( walk, 24.0f );

    /* Play the walk action using spherical interpolation between frames
     * and loop when the end frame is reached.
     */
    MD5_action_play( walk,
                     MD5_METHOD_SLERP,
                     1 );
    
    /* Initialize a temporary skeleton to be able to store the final
     * pose after blending the idle and walk animation.
     */
    final_pose = ( MD5JOINT * ) calloc( md5->n_joint, sizeof( MD5JOINT ) );

    /* Free the mesh data that used to build the mesh, because this data
     * is no longer required for drawing.
     */
    MD5_free_mesh_data( md5 );
    
    /* Disable the cull face to make sure that even backfaces will drawn
     * onscreen.
     */
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

    /* If auto-rotate is ON, simply turn the geometry on the Z axis,
     * demo reel style.
     */
    if( auto_rotate ) rot_angle.z += 1.0f;

    /* Rotate the X and Z axis based on the rotation specified by the
     * user.
     */
    GFX_rotate( rot_angle.x, 1.0f, 0.0f, 0.0f );
    GFX_rotate( rot_angle.z, 0.0f, 0.0f, 1.0f );

    /* Control the direction of the blending, from idle to walk and from
     * walk to idle (for demonstration purposes only, of course).
     */
    static unsigned char blend_direction = 0;

    /* The blend factor to use to mix the two animation sequences
     * together.  This factor is basically a value from 0 to 1 that
     * determines how much the two actions will be blended together
     * (from the first to the second).
     */
    static float blend_factor = 0.0f;

    /* If the blend_direction is 0, increase the blend factor to
     * progressively push lazy Bob to start walking.
     */
    if( !blend_direction ) blend_factor += 0.001f;
    /* If the blend_direction is a value other than 0, progressivly
     * decrease the blend factor (to let Bob have a rest) and
     * interpolate the walk animation back to the idle action.
     */
    else blend_factor -= 0.001f;

    /* If the blend factor is less than 0 or greater than 1, invert the
     * blend_direction.
     */
    if( blend_factor < 0.0f || blend_factor > 1.0f )
    { blend_direction = !blend_direction; }

    /* Increase the time step for all actions with statuses that are set
     * to PLAY.
     */
    MD5_draw_action( md5, 1.0f / 60.0f );

    /* This is the interesting part.  This function receives two
     * skeleton poses and blends them together based on the a blend
     * factor.  The resulting skeleton will then be stored in the
     * final_pose array of joints.
     */
    MD5_blend_pose( md5,
                   /* The final skeleton. */
                    final_pose,
                   /* The first action to use for blending. */
                    idle->pose,
                   /* The second action to use for blending. */
                    walk->pose,
                   /* The method to use to blend the two skeletons
                    * together based on the blend factor passed to the
                    * function.  Note that joint's position will always
                    * be linearly interpolated, and the rotation
                    * interpolation will use the method specified by
                    * this parameter.
                    */
                    MD5_METHOD_SLERP,
                   /* Make sure that the blend_factor always stays in
                    * bewteen 0 and 1.
                    */
                    CLAMP( blend_factor, 0.0f, 1.0f ) );

    /* Use the final pose skeleton to calculate and update the vertex
     * positions of the skeleton's skin and the VBO with the latest
     * interpolated skeleton.
     */
    MD5_set_pose( md5, final_pose );
    
    /* Draw the model onscreen. */
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
    free(final_pose);
    obj = OBJ_free(obj);
    md5 = MD5_free(md5);

	light = LIGHT_free( light );
}