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

typedef struct {
    char    name[ MAX_CHAR ];
    vec4    color;
    vec3    direction;
    vec4    position;   // Position of the lamp in world coordinates
    /* Affect the attenuation of the light based on its distance from
     * the fragment.
     */
    float   linear_attenuation;
    /* Affect the attenuation of the light based on the square of the
     * distance of the fragment from the light.
     */
    float   quadratic_attenuation;
    /* The falloff distance of the light.  The light will be at half of
     * its original intensity at this distance.
     */
    float   distance;
    /* The cosine of half the field of view of the spot (in radians). */
    float   spot_cos_cutoff;
    /* Factor ranging from 0 to 1 to smooth the edge of the spot circle. */
    float   spot_blend;
    /* The spot direction is calculated by multiplying the direction vector
     * by the invert of the modelview matrix of the camera.
     */
    vec3    spot_direction;
    unsigned char type;
} LAMP;

/* Define the maximum amount of lamps your implementation can handle
 * simultaneously.  For this example, you're defining 2.
 */
#define MAX_LAMP    2
/* Declare an array of lamp pointers. */
LAMP *lamp[MAX_LAMP];

LAMP *LAMP_create_directional( char *name,
                               vec4 *color,
                               float rotx,
                               float roty,
                               float rotz )
{
    /* Declare the up axis vector to be static, because it won't change. */
    vec3 up_axis = { 0.0f, 0.0f, 1.0f };
    /* Allocate memory for a new LAMP. */
    LAMP *lamp = ( LAMP * ) calloc( 1, sizeof( LAMP ) );
    /* Assign the name received in parameter to the structure, because it is
     * always nice to have an internal name for each structure.
     */
    strcpy( lamp->name, name );
    /* Assign the color to the lamp. */
    memcpy( &lamp->color, color, sizeof( vec4 ) );
    /* Set the type of the lamp as 0 for directional. */
    lamp->type = 0;
    /* Use the following helper function (which can be found in utils.cpp)
     * to rotate the up axis by the XYZ rotation angle received as parameters.
     * I think it's a lot easier to deal with angles when it comes to direction
     * vectors.
     */
    create_direction_vector( &lamp->direction, &up_axis, rotx, roty, rotz );
    /* Return the new lamp pointer. */
    return lamp;
}

LAMP *LAMP_create_point( char *name, vec4 *color, vec3 *position )
{
    LAMP *lamp = ( LAMP * ) calloc( 1, sizeof( LAMP ) );
    strcpy( lamp->name, name );
    memcpy( &lamp->color, color, sizeof( vec4 ) );
    /* Assign the position received in parameter to the current lamp
     * pointer.  In addition, make sure thatyou specify 1 as the W
     * component of the position, because you are going to need to
     * multiply it by the modelview matrix the same way as if you were
     * dealing with a vertex position in eye space.
     */
    memcpy( &lamp->position, position, sizeof( vec3 ) );
    lamp->position.w = 1.0f;
    /* Specify that 1 represents a basic point light that emits constant
     * omnidirectional light.
     */
    lamp->type = 1;
    return lamp;
}

LAMP *LAMP_create_point_with_attenuation( char *name, vec4 *color, vec3 *position, float distance, float linear_attenuation, float quadratic_attenuation )
{
    LAMP *lamp = ( LAMP * ) calloc( 1, sizeof( LAMP ) );
    strcpy( lamp->name, name );
    memcpy( &lamp->color, color, sizeof( vec4 ) );
    memcpy( &lamp->position, position, sizeof( vec3 ) );
    lamp->position.w = 1.0f;
    /* Store the double distance, beacuse the falloff distance parameter
     * represents the half distance where the light starts to be
     * attenuated.
     */
    lamp->distance = distance * 2.0f;
    /* Store the linear attentuation. */
    lamp->linear_attenuation = linear_attenuation;
    /* Strore the quadratic attenuation. */
    lamp->quadratic_attenuation = quadratic_attenuation;
    /* Create a new lamp type. */
    lamp->type = 2;

    return lamp;
}

/* Basically create a point light, but with a distance parameter. */
LAMP *LAMP_create_point_sphere( char *name,
                                vec4 *color,
                                vec3 *position,
                                float distance )
{
    /* Redirect the execution pointer to create a simple point light, and
     * then adjust and tweak the other parameters to fit a new lamp type.
     */
    LAMP *lamp = LAMP_create_point( name, color, position );

    lamp->distance = distance;

    lamp->type = 3;

    return lamp;
}

LAMP *LAMP_create_spot( char *name,
                        vec4 *color,
                        vec3 *position,
                       /* The XYZ rotation angle of the spot direction
                        * vector in degrees.
                        */
                        float rotx,
                        float roty,
                        float rotz,
                       /* The field of view of the spot, also in degrees. */
                        float fov,
                       /* The spot blend to smooth the edge of the spot.
                        * This value is between the range of 0 and 1, where
                        * 0 represents no smoothing.
                        */
                       float spot_blend ) {
    static vec3 up_axis = { 0.0f, 0.0f, 1.0f };
    LAMP *lamp = ( LAMP * ) calloc( 1, sizeof( LAMP ) );
    strcpy( lamp->name, name );
    memcpy( &lamp->color, color, sizeof( vec4 ) );
    /* Calculate the spot cosine cut off. */
    lamp->spot_cos_cutoff = cosf( ( fov * 0.5f ) * DEG_TO_RAD );
    /* Clamp the spot blend to make sure that there won't be a division by 0
     * inside the shader program.
     */
    lamp->spot_blend = CLAMP( spot_blend, 0.001, 1.0f );
    memcpy( &lamp->position, position, sizeof( vec3 ) );
    lamp->position.w = 1.0f;
    lamp->type = 4;
    /* Create the direction vector for the spot based on the XYZ rotation
     * angle that the function receives.
     */
    create_direction_vector( &lamp->spot_direction,
                            &up_axis,
                            rotx,
                            roty,
                            rotz );
    return lamp;
}

void LAMP_get_direction_in_object_space( LAMP *lamp, mat4 *m, vec3 *direction )
{
    mat4 invert;

    mat4_copy_mat4( &invert, m );

    mat4_invert( &invert );

    vec3_multiply_mat4( direction,
                       &lamp->spot_direction,
                       m );

    vec3_normalize( direction,
                   direction );

    vec3_invert( direction,
                direction );
}

void LAMP_get_direction_in_eye_space( LAMP *lamp, mat4 *m, vec3 *direction )
{
    /* Multiply the current lamp direction by the view matrix received in
     * parameter to be able to calculate the lamp direction in eye space.
     */
    vec3_multiply_mat4( direction,
                       &lamp->direction,
                       m );
    /* Invert the vector, because in eye space, the direction is simply the
     * inverted vector.
     */
    vec3_invert( direction, direction );
}

/* This function is basically very easy.  In the same way that you
 * convert the position in your vertex shader, handle the conversion
 * to eye space here so you you do not have to pass the modelview
 * matrix of the camera to the shader, and offload a bit of work from
 * the CPU.
 */
void LAMP_get_position_in_eye_space( LAMP *lamp, mat4 *m, vec4 *position )
{
    /* Multiply the position by the matrix received in parameters and
     * assign the result to the position vector.
     */
    vec4_multiply_mat4( position,
                        &lamp->position,
                        m );
}

LAMP *LAMP_free( LAMP *lamp )
{
    free( lamp );
    return NULL;
}

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
            /* In this scene, all the materials (in this case, there are
             * only two) have the exact same properties, so simply tag the
             * uniforms for the current material to be constant.  This will
             * also allow you to get better performance at runtime, because
             * the data will not be sent over and over for nothing.
             */
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

		++i;
	}

    /* A temp string to dynamically create the LAMP property names. */
    char tmp[ MAX_CHAR ] = {""};

//	sprintf( tmp, "LAMP_FS.color" );
//	glUniform4fv( PROGRAM_get_uniform_location( program, tmp ),
//                 1,
//                 ( float * )&lamp->color );
//    
//    /* Check if the lamp type is directional.  If yes, you need to send
//     * over the normalized light direction vector in eye space.
//     */
//    if( lamp->type == 0 )
//    {
//        /* Temp variable to hold the direction in eye space. */
//        vec3 direction;
//        /* Create the lamp direction property name. */
//        sprintf( tmp, "LAMP_VS.direction" );
//        /* Call the function that you created in the previous step to
//         * convert the current world space direction vector of the lamp
//         * to eye space.  Note that at this point, the current model view
//         * matrix stack is pushed because you are currently drawing the
//         * object.  In order to calculate the right direction vector of
//         * the lamp, what you are interested in is gaining access to the
//         * camera model view matrix.  To do this, all you have to do is
//         * request the previous model view matrix, because you push it
//         * once in the templateAppDraw function.
//         */
//        LAMP_get_direction_in_eye_space( lamp,
//
//                                        &gfx.modelview_matrix[ gfx.modelview_matrix_index - 1 ],
//
//                                        &direction );
//
//        glUniform3fv( PROGRAM_get_uniform_location( program, tmp ),
//                     1,
//                     ( float * )&direction );
//    }
//    else if( lamp->type == 1 )
//    {
//        vec4 position;
//
//        sprintf( tmp, "LAMP_VS.position" );
//        LAMP_get_position_in_eye_space( lamp,
//                                       &gfx.modelview_matrix[ gfx.modelview_matrix_index - 1 ],
//                                       &position );
//
//        glUniform3fv( PROGRAM_get_uniform_location( program, tmp ),
//                     1,
//                     ( float * )&position );
//    }
//    else if( lamp->type == 2 )
//    {
//        vec4 position;
//
//        sprintf( tmp, "LAMP_VS.position" );
//
//        LAMP_get_position_in_eye_space( lamp,
//                                       &gfx.modelview_matrix[ gfx.modelview_matrix_index - 1 ],
//                                       &position );
//
//        glUniform3fv( PROGRAM_get_uniform_location( program, tmp ),
//                     1,
//                     ( float * )&position );
//
//        sprintf( tmp, "LAMP_FS.distance" );
//        glUniform1f( PROGRAM_get_uniform_location( program, tmp ),
//                    lamp->distance );
//
//        sprintf( tmp, "LAMP_FS.linear_attenuation" );
//        glUniform1f( PROGRAM_get_uniform_location( program, tmp ),
//                    lamp->linear_attenuation );
//
//        sprintf( tmp, "LAMP_FS.quadratic_attenuation" );
//        glUniform1f( PROGRAM_get_uniform_location( program, tmp ),
//                    lamp->quadratic_attenuation );
//
//    }
//    /* This basically the same as for type # 1 (basic point light), except
//     * that the distance is sent over to the shader.
//     */
//    else if( lamp->type == 3 )
//    {
//        vec4 position;
//
//        sprintf( tmp, "LAMP_VS.position" );
//
//        LAMP_get_position_in_eye_space( lamp,
//                                        &gfx.modelview_matrix[ gfx.modelview_matrix_index - 1 ],
//                                        &position );
//
//        glUniform3fv( PROGRAM_get_uniform_location( program, tmp ),
//                     1,
//                     ( float * )&position );
//
//        sprintf( tmp, "LAMP_FS.distance" );
//        glUniform1f( PROGRAM_get_uniform_location( program, tmp ),
//                    lamp->distance );
//    }
//    else if( lamp->type == 4 )
//    {
//        vec4 position;
//
//        sprintf( tmp, "LAMP_VS.position" );
//
//        LAMP_get_position_in_eye_space( lamp,
//                                       &gfx.modelview_matrix[ gfx.modelview_matrix_index - 1 ],
//                                       &position );
//
//        glUniform3fv( PROGRAM_get_uniform_location( program, tmp ),
//                     1,
//                     ( float * )&position );
//        /* Calculating the direction of a spot is slightly different than
//         * for directional lamp, because the cone has to be projected in
//         * the same space as the object that might receive the light.
//         */
//        vec3 direction;
//
//        sprintf( tmp, "LAMP_VS.spot_direction" );
//        LAMP_get_direction_in_object_space( lamp,
//                                           &gfx.modelview_matrix[ gfx.modelview_matrix_index - 1 ],
//                                           &direction );
//
//        glUniform3fv( PROGRAM_get_uniform_location( program, tmp ),
//                     1,
//                     ( float * )&direction );
//        /* Send the spot cos cutoff to let the shader determine if a
//         * specific fragment is inside or outside the cone of light.
//         */
//        sprintf( tmp, "LAMP_FS.spot_cos_cutoff" );
//        glUniform1f( PROGRAM_get_uniform_location( program, tmp ),
//                    lamp->spot_cos_cutoff );
//
//        sprintf( tmp, "LAMP_FS.spot_blend" );
//        glUniform1f( PROGRAM_get_uniform_location( program, tmp ),
//                    lamp->spot_blend );
//    }

    i = 0;
    /* Since your lamps are now in an array, simply loop and dynamically
     * create the uniform name for the lamp index in the shader program,
     * and gather the necessary data for a specific lamp index as long as
     * the loop is rolling.
     */
    while( i != MAX_LAMP )
    {
        sprintf( tmp, "LAMP_FS[%d].color", i );
        glUniform4fv( PROGRAM_get_uniform_location( program, tmp ),
                      1,
                      ( float * )&lamp[ i ]->color );

        vec4 position;

        sprintf( tmp, "LAMP_VS[%d].position", i );

        LAMP_get_position_in_eye_space( lamp[ i ],
                                        &gfx.modelview_matrix[ gfx.modelview_matrix_index - 1 ],
                                        &position );

        glUniform3fv( PROGRAM_get_uniform_location( program, tmp ),
                      1,
                      ( float * )&position );

        sprintf( tmp, "LAMP_FS[%d].distance", i );
        glUniform1f( PROGRAM_get_uniform_location( program, tmp ),
                     lamp[ i ]->distance );

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

//    lamp = LAMP_create_directional( ( char * )"sun",    // Internal name of lamp
//                                   &color, // The lamp color.
//                                   -25.0f,  // The XYZ rotation angle in degrees
//                                   0.0f,  // that will be used to create the
//                                   -45.0f );// direction vector.
    /* The 3D position in world space of the point light. */
    vec3 position = { 3.5f, 3.0f, 6.0f };
//    /* Create a new LAMP pointer and declare it as a simple point light. */
//    lamp = LAMP_create_point((char *)"point", &color, &position);
//    /* The linear and quadratic attenuation are values that range from 0
//     * to 1, which will be directly affected by the falloff distance of
//     * the lamp.  1 means fully attenuated, and 0 represents constant (same
//     * as in the regular point light calculations in the previous section).
//     */
//    lamp = LAMP_create_point_with_attenuation( ( char * )"point1",
//                                               &color,
//                                               &position,
//                                               10.0f,
//                                                0.5f,
//                                                1.0f );
//    lamp = LAMP_create_point_sphere((char *)"point2",
//                                    &color,
//                                    &position,
//                                    10.0f);
//    lamp = LAMP_create_spot( ( char * )"spot",
//                             &color,
//                             &position,
//                            /* The spot XYZ rotation angles in degrees. */
//                             -25.0f, 0.0f, -45.0f,
//                            /* The field of view in degrees. */
//                             75.0f,
//                            /* The spot blend. */
//                             0.05f );

    /* Create the first lamp, basically the same as you did before, except
     * you are initializing it at index 0 of the lamp point array.
     */
    lamp[ 0 ] = LAMP_create_point_sphere( ( char * )"point1", &color, &position, 10.0f );

    /* Invert the XY position. */
    position.x = -position.x;
    position.y = -position.y;

    /* Modify the color to be red. */
    color.y =
    color.z = 0.0f;

    /* Create the second lamp. */
    lamp[ 1 ] = LAMP_create_point_sphere( ( char * )"point2", &color, &position, 10.0f );
}


void templateAppDraw( void ) {

	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
	glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );


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


void templateAppExit( void ) {
    unsigned int i = 0;
    while( i != MAX_LAMP ) {
        lamp[ i ] = LAMP_free( lamp[ i ] );
        ++i;
    }

	OBJ_free( obj );
}
