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
    unsigned char type;
} LAMP;

LAMP *lamp = NULL;

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
    /* Create the uniform name for the color of the lamp. */
    sprintf( tmp, "LAMP_FS.color" );
    /* Get the uniform location and send over the current lamp color. */
    glUniform4fv( PROGRAM_get_uniform_location( program, tmp ),
                 1,
                 ( float * )&lamp->color );

    /* Check if the lamp type is directional.  If yes, you need to send
     * over the normalized light direction vector in eye space.
     */
    if( lamp->type == 0 )
    {
        /* Temp variable to hold the direction in eye space. */
        vec3 direction;
        /* Create the lamp direction property name. */
        sprintf( tmp, "LAMP_VS.direction" );
        /* Call the function that you created in the previous step to
         * convert the current world space direction vector of the lamp
         * to eye space.  Note that at this point, the current model view
         * matrix stack is pushed because you are currently drawing the
         * object.  In order to calculate the right direction vector of
         * the lamp, what you are interested in is gaining access to the
         * camera model view matrix.  To do this, all you have to do is
         * request the previous model view matrix, because you push it
         * once in the templateAppDraw function.
         */
        LAMP_get_direction_in_eye_space( lamp,
                                         &gfx.modelview_matrix[ gfx.modelview_matrix_index - 1 ],
                                         &direction );

        glUniform3fv( PROGRAM_get_uniform_location( program, tmp ),
                      1,
                      ( float * )&direction );
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

    lamp = LAMP_create_directional( ( char * )"sun",    // Internal name of lamp
                                   &color, // The lamp color.
                                   -25.0f,  // The XYZ rotation angle in degrees
                                   0.0f,  // that will be used to create the
                                   -45.0f );// direction vector.
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
    lamp = LAMP_free(lamp);

	OBJ_free( obj );
}
