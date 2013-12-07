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
/*
 * Source code modified by Chris Larsen to make the following data types into
 * proper C++ classes:
 * - MEMORY
 * - OBJ
 * - OBJMATERIAL
 * - OBJMESH
 * - OBJTRIANGLEINDEX
 * - OBJTRIANGLELIST
 * - OBJVERTEXDATA
 * - PROGRAM
 * - SHADER
 */

#include "templateApp.h"

/* The OBJ file name on disk. */
#define OBJ_FILE    (char *)"model.obj"
/* Your vertex and fragment shader files. */
#define VERTEX_SHADER   (char *)"momo_nc.vs"
#define FRAGMENT_SHADER (char *)"momo_nc.fs"
#define DEBUG_SHADERS   true
/* The main OBJ structure that you will use to load the .obj. */
OBJ *obj = NULL;
/* Pointer to an mesh inside the OBJ object. */
OBJMESH *objmesh = NULL;
/* Shader program structure pointer. */
PROGRAM *program = NULL;

/* The main structure of the template. This is a pure C struct, you initialize the structure
   as demonstrated below. Depending on the type of your type of app simply comment / uncomment
   which event callback you want to use. */

TEMPLATEAPP templateApp = {
    /* Will be called once when the program starts. */
    templateAppInit,

    /* Will be called every frame. This is the best location to plug your drawing. */
    templateAppDraw,

    /* This function will be triggered when a new touche is recorded on screen. */
    templateAppToucheBegan,

    /* This function will be triggered when an existing touche is moved on screen. */
    templateAppToucheMoved,
};

void program_draw_callback(void *ptr)
{
    /* Convert the void * in the parameter to a valid PROGRAM pointer. */
    PROGRAM *curr_program = (PROGRAM *)ptr;

    for (auto it=curr_program->uniform_map.begin();
         it!=curr_program->uniform_map.end(); ++it) {
        auto    &name = it->first;
        auto    &uniform = it->second;
        if (name == "MODELVIEWPROJECTIONMATRIX") {
            /* Update the matrix. */
            glUniformMatrix4fv(uniform.location,                                // The uniform location.
                               1,                                               // Number of matrices.
                               GL_FALSE,                                        // Don't transpose the matrix.
                               (float *)GFX_get_modelview_projection_matrix()); // The result of the current projection
                                                                                // matrix multiplied by the model view
                                                                                // matrix.
        } /* End if */
    }
// You can use this loop if your compiler supports it instead of the
// loop above.  Supposedly "range for" loops are a C++ 11 feature but
// the default Xcode 5.0.1 C++ compiler supports them even if you're not
// using the -std=c++11 compiler option.  Likewise, for the "range for"
// loop below in templateAppInit().
//    for (auto it : curr_program->uniform_map) {
//        auto    &name    = it.first;
//        auto    &uniform = it.second;
//        if (name == "MODELVIEWPROJECTIONMATRIX") {
//            /* Update the matrix. */
//            glUniformMatrix4fv(uniform.location,                                // The uniform location.
//                               1,                                               // Number of matrices.
//                               GL_FALSE,                                        // Don't transpose the matrix.
//                               (float *)GFX_get_modelview_projection_matrix()); // The result of the current projection
//                                                                                // matrix multiplied by the model view
//                                                                                // matrix.
//        } /* End if */
//    }
}

void templateAppInit(int width, int height)
{
    // Setup the exit callback function.
    atexit(templateAppExit);

    // Initialize GLES.
    GFX_start();

    // Setup a GLES viewport using the current width and height of the screen.
    glViewport(0, 0, width, height);
    
    GFX_set_matrix_mode(PROJECTION_MATRIX);
    GFX_load_identity();
    GFX_set_perspective( 45.0f,
                        (float)width / (float)height,
                          0.1f,
                        100.0f,
                          0.0f);

    program = new PROGRAM((char *)"default",        // The shader program name.
                          VERTEX_SHADER,            // The vertex shader file.
                          FRAGMENT_SHADER,          // The fragment shader file.
                          true,                     // Use relative file path.
                          DEBUG_SHADERS,            // Debug program and shaders.
                          NULL,                     // Not in use for now.
                          program_draw_callback);   // The draw function callback
                                                    // that you previously declared
                                                    // in steps 2 and 3.

    obj = new OBJ(OBJ_FILE, true);

    objmesh = &obj->objmesh[0];

    /* To hold the vertex data. */
    unsigned char *vertex_array = NULL,
    /* The start position of the vertex data array. */
    *vertex_start = NULL;
    /* To hold the current vertex index. */
    unsigned int index = 0,
    /* Store the size in bytes between each vertex data type. */
    stride = 0,
    /* The total size in bytes of the vertex data array. */
    size = 0;
    /* Calculate the total size of the array based on the number of
     * independent vertex data multiplied by the size of a vertex
     * position and the size of a vertex normal.
     */
    // BTW, the book gives the wrong formula.  The one here is correct.
    size = objmesh->objvertexdata.size() * (sizeof(vec3) + sizeof(vec3));
    /* Allocate the total amount of bytes in memory. */
    vertex_array = (unsigned char *) malloc(size);
    /* Remember the starting memory address of the vertex array. */
    vertex_start = vertex_array;

    for (int i=0; i!=objmesh->objvertexdata.size(); ++i) {
        /* Get the current vertex data index. */
        index = objmesh->objvertexdata[i].vertex_index;
        /* Append the vertex position to the vertex data array. */
        *reinterpret_cast<vec3*>(vertex_array) = obj->indexed_vertex[index];
        /* Increment the current memory position to move on to the next insertion point. */
        vertex_array += sizeof(vec3);
        /* Insert the vertex normal at the current position. */
        *reinterpret_cast<vec3*>(vertex_array) = obj->indexed_normal[index];
        /* Move on to the next insertion point. */
        vertex_array += sizeof(vec3);
    }
// You can use this loop if your compiler supports it instead of the
// loop above.  Supposedly "range for" loops are a C++ 11 feature but
// the default Xcode 5.0.1 C++ compiler supports them even if you're not
// using the -std=c++11 compiler option.  Likewise, for the "range for"
// loop above in program_draw_callback().
//    for (auto objvertexdata : objmesh->objvertexdata) {
//        /* Get the current vertex data index. */
//        auto    &index = objvertexdata.vertex_index;
//        /* Append the vertex position to the vertex data array. */
//        *reinterpret_cast<vec3*>(vertex_array) = obj->indexed_vertex[index];
//        /* Increment the current memory position to move on to the next insertion point. */
//        vertex_array += sizeof(vec3);
//        /* Insert the vertex normal at the current position. */
//        *reinterpret_cast<vec3*>(vertex_array) = obj->indexed_normal[index];
//        /* Move on to the next insertion point. */
//        vertex_array += sizeof(vec3);
//    }

    /* Generate a new VBO id. */
    glGenBuffers(1, &objmesh->vbo);
    /* Make the id active and tell GLES that it should be represented
     * as a vertex data array buffer.
     */
    glBindBuffer(GL_ARRAY_BUFFER, objmesh->vbo);
    glBufferData(GL_ARRAY_BUFFER,   // The type of data to associate the array with.
                 size,              // The total size in bytes of the array.
                 vertex_start,      // The starting position of the array.
                 GL_STATIC_DRAW);   // Since the data will not be updated every
                                    // frame, tell GLES that the data is static
                                    // for internal driver optimization.

    /* Free the array from the local memory. */
    free(vertex_start);
    
    /* Deactivate the current VBO id attached as a vertex array buffer. */
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    /* Generate a new VBO id for the indices. */
    glGenBuffers(1, &objmesh->objtrianglelist[0].vbo);
    /* Make the current index active, and specify to GLES that the index is
     * for an indices array (aka Element Array).
     */
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, objmesh->objtrianglelist[0].vbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,                   // The type of array.
                 objmesh->objtrianglelist[0].n_indice_array * sizeof(unsigned short),   // The total size of the indices array.
                 &objmesh->objtrianglelist[0].indice_array[0],  // The indices array.
                 GL_STATIC_DRAW);
    /* Once again specify that the array is static as the indices won't change. */
    /* Deactivate the current VBO id attached as an indices array. */
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    GLint   attribute;                      // Vertex position size in bytes.
    stride = sizeof(vec3) + sizeof(vec3);   // Vertex normal size in bytes.

    glGenVertexArraysOES(1, &objmesh->vao);
    glBindVertexArrayOES(objmesh->vao);
    glBindBuffer(GL_ARRAY_BUFFER, objmesh->vbo);
    /* Get the attribute location from the shader program. */
    attribute = program->get_vertex_attrib_location((char *)"POSITION");
    /* Enable the attribute location. */
    glEnableVertexAttribArray(attribute);
    glVertexAttribPointer(attribute,        // The location of the attribute.
                          3,                // The size of each component (in this case, 3 for XYZ).
                          GL_FLOAT,         // The type of data.
                          GL_FALSE,         // Do not normalize the vertex data.
                          stride,           // The size in bytes of the next vertex position.
                          (void *)NULL);    // No need to pass the vertex position array because you are using a VBO.

    attribute = program->get_vertex_attrib_location((char *)"NORMAL");
    glEnableVertexAttribArray(attribute);
    glVertexAttribPointer(attribute,
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          stride,
                          BUFFER_OFFSET(sizeof(vec3)));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,           // Bind the index as an indices buffer.
                 objmesh->objtrianglelist[0].vbo);  // Pass the indices VBO index to activate its usage.

    glBindVertexArrayOES(0);
}

void templateAppDraw(void)
{
    // Clear the depth, stencil and colorbuffer.
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    
    GFX_set_matrix_mode(MODELVIEW_MATRIX);
    GFX_load_identity();
    {
        vec3 e = { 0.0f, -4.0f, 0.0f },
             c = { 0.0f,  0.0f, 0.0f },
             u = { 0.0f,  0.0f, 1.0f };
        GFX_look_at( &e, &c, &u );
    }

    glBindVertexArrayOES(objmesh->vao);

    program->draw();
    /* Function to use when drawing using elements (aka indices). */
    glDrawElements(GL_TRIANGLES,                                // The order in which the indices are listed.
                   objmesh->objtrianglelist[0].n_indice_array,  // How many indices have to be used for drawing.
                   GL_UNSIGNED_SHORT,                           // The type of indices.
                   (void *)NULL);                               // The start offset in bytes of the first index;
                                                                // in this case, 0 or NULL, since you want to
                                                                // start drawing from the first index in the array.
}


void templateAppToucheBegan(float x, float y, unsigned int tap_count)
{
}


void templateAppToucheMoved(float x, float y, unsigned int tap_count)
{
}


void templateAppExit(void)
{
    delete program;
    program = NULL;
    delete obj;
}
