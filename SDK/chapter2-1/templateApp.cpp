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

/* The main structure of the template. This is a pure C struct, you initialize the structure
   as demonstrated below. Depending on the type of your type of app simply comment / uncomment
   which event callback you want to use. */

TEMPLATEAPP templateApp = {
							/* Will be called once when the program start. */
							templateAppInit,
							
							/* Will be called every frame. This is the best location to plug your drawing. */
							templateAppDraw,
						  };

#define VERTEX_SHADER   (char *)"first.vs"
#define FRAGMENT_SHADER (char *)"first.fs"

#define DEBUG_SHADERS   1

PROGRAM *program = NULL;
MEMORY  *m = NULL;

void templateAppInit( int width, int height )
{
	// Setup the exit callback function.
	atexit( templateAppExit );
	
	// Initialize GLES.
	GFX_start();
	
	// Setup a GLES viewport using the current width and height of the screen.
	glViewport( 0, 0, width, height );
	
	/* Insert your initialization code here */
    GFX_set_matrix_mode(PROJECTION_MATRIX);

    float   half_width = static_cast<float>(width) * 0.5f,
            half_height = static_cast<float>(height) * 0.5f;

    GFX_load_identity();

    GFX_set_orthographic_2d(-half_width, half_width, -half_height, half_height);
    GFX_translate(-half_width, -half_height, 0.0f);

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    program = PROGRAM_init((char *)"default");

    program->vertex_shader = SHADER_init(VERTEX_SHADER, GL_VERTEX_SHADER);
    program->fragment_shader = SHADER_init(FRAGMENT_SHADER, GL_FRAGMENT_SHADER);
    
    m = mopen(VERTEX_SHADER, 1);
    if (m) {
        if (!SHADER_compile(program->vertex_shader, (char *)m->buffer, DEBUG_SHADERS)) {
            exit(1);
        }
    }
    m = mclose(m);

    m = mopen(FRAGMENT_SHADER, 1);
    if (m) {
        if (!SHADER_compile(program->fragment_shader, (char *)m->buffer, DEBUG_SHADERS)) {
            exit(1);
        }
    }
    m = mclose(m);

    if (!PROGRAM_link(program, DEBUG_SHADERS)) {
        exit(3);
    }
}


void templateAppDraw( void )
{
    static const float POSITION[8] = {
        0.0f, 0.0f, // Down left (pivot point)
        1.0f, 0.0f, // Up left
        0.0f, 1.0f, // Down right
        1.0f, 1.0f  // Up right
    };
    static const float COLOR[16] = {
        1.0f /* R */, 0.0f /* G */, 0.0f /* B */, 1.0f /* A */, /* Red */
        0.0f, 1.0f, 0.0f, 1.0f, /* Green */
        0.0f, 0.0f, 1.0f, 1.0f, /* Blue */
        1.0f, 1.0f, 0.0f, 1.0f  /* Yellow */
    };

    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    
	// Clear the depth, stencil and colorbuffer.
	glClear( GL_COLOR_BUFFER_BIT );

	/* Insert your drawing code here */

    /* Select the model view matrix. */
    GFX_set_matrix_mode(MODELVIEW_MATRIX);

    /* Reset it to make sure you are going to deal with a clean identity matrix. */
    GFX_load_identity();

    /* Scale the quad to be 100px by 100px. */
    GFX_scale(100.0f, 100.0f, 0.0f);

    if (program->pid) {
        char    attribute, uniform;

        glUseProgram(program->pid);

        uniform = PROGRAM_get_uniform_location(program, (char *)"MODELVIEWPROJECTIONMATRIX");
        glUniformMatrix4fv(uniform,     // The location value of the uniform.
                           1,           // How many 4x4 matrices
                           GL_FALSE,    // Specify to do not transpose the matrix.
                           (float *)GFX_get_modelview_projection_matrix()); // Use the GFX helper function
                                                                            // to calculate the result of the
                                                                            // current model view matrix
                                                                            // multiplied by the current
                                                                            // projection matrix.
        attribute = PROGRAM_get_vertex_attrib_location(program, (char *)"POSITION");
        glEnableVertexAttribArray(attribute);
        glVertexAttribPointer(attribute,    // The attribute location
                              2,            // How many elements; XY in this case, so 2.
                              GL_FLOAT,     // The variable type.
                              GL_FALSE,     //Do not normalize the data.
                              0,            // The stride in bytes of the array delimiting the elements,
                                            // in this case none.
                              POSITION);    //The vertex position array pointer.
        attribute = PROGRAM_get_vertex_attrib_location(program, (char *)"COLOR");
        glEnableVertexAttribArray(attribute);
        glVertexAttribPointer(attribute,
                              4,
                              GL_FLOAT,
                              GL_FALSE,
                              0,
                              COLOR);
        glDrawArrays(GL_TRIANGLE_STRIP, //The drawing mode.
                     0,                 // Start at which index.
                     4);                // Stop at which index.
    }

    GFX_error();
}


void templateAppExit( void )
{
	/* Code to run when the application exit, perfect location to free everything. */
    printf("templateAppExit...\n");

    if(m) m = mclose(m);

    if(program && program->vertex_shader)
        program->vertex_shader = SHADER_free(program->vertex_shader);
    if(program && program->fragment_shader)
        program->fragment_shader = SHADER_free(program->fragment_shader);
    if(program)
        program = PROGRAM_free(program);
}
