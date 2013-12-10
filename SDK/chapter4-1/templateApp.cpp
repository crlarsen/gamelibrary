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
 * - TEXTURE
 */

#include "templateApp.h"
#define OBJ_FILE (char *)"scene.obj"
PROGRAM *program = NULL;
OBJ *obj = NULL;
TEMPLATEAPP templateApp = {
    templateAppInit,
    templateAppDraw
};

void program_bind_attrib_location(void *ptr) {
    PROGRAM *program = (PROGRAM *)ptr;
    glBindAttribLocation(program->pid, VA_Position,  VA_Position_String);
    glBindAttribLocation(program->pid, VA_TexCoord0, VA_TexCoord0_String);
}

/* This time you will use the material draw callback instead of the
 * program draw callback, since in this chapter you will work on a
 * material basis, not on a shader program basis.
 */
void material_draw_callback(void *ptr)
{
    OBJMATERIAL *objmaterial = (OBJMATERIAL *)ptr;
    PROGRAM *program = objmaterial->program;

    for (auto it=program->uniform_map.begin(); it!=program->uniform_map.end(); ++it) {
        auto    &name = it->first;
        auto    &uniform = it->second;
        if (name == TM_Diffuse_String) {
            /* If a diffuse texture is specified inside the MTL file, it
             * will always be bound to the second texture channel
             * (GL_TEXTURE1).
             */
            glUniform1i(uniform.location, TM_Diffuse);
        } else if (name == "MODELVIEWPROJECTIONMATRIX") {
            /* Send over the current model view matrix multiplied by the
             * projection matrix.
             */
            glUniformMatrix4fv(uniform.location, 1, GL_FALSE, (float *)GFX_get_modelview_projection_matrix());
        }
    }
}

void templateAppInit(int width, int height)
{
    atexit(templateAppExit);

    GFX_start();
    
    glViewport(0.0f, 0.0f, width, height);
    
    GFX_set_matrix_mode(PROJECTION_MATRIX);
    GFX_load_identity();
    GFX_set_perspective( 45.0f,
                        (float)width / (float)height,
                          0.1f,
                        100.0f,
                        -90.0f);    // This time you will use a landscape
                                    // view, so rotate the projection
                                    // matrix 90 degrees.

    obj = new OBJ(OBJ_FILE, true);

    /* While there are some objects. */
    for (auto objmesh = obj->objmesh.begin();
         objmesh != obj->objmesh.end(); ++objmesh) {
        /* Generate the VBOs and VAO for the current object. */
        objmesh->build();  // The object index inside the OBJ structure.

        /* Free all the vertex data related arrays. At this point, they
         * have all been transferred to the video memory by the
         * OBJ::build_mesh call.
         */
        objmesh->free_vertex_data();
    }

    for (auto texture=obj->texture.begin();
         texture!=obj->texture.end(); ++texture) {
        (*texture)->build(obj->texture_path,    // By default the same as where the .mtl is located.
                          TEXTURE_MIPMAP,
                          TEXTURE_FILTER_2X,
                          0.0f);
    }

    for (auto objmaterial=obj->objmaterial.begin();
         objmaterial!=obj->objmaterial.end(); ++objmaterial) {
        /* Link all textures to the material(s). */
        objmaterial->build(NULL);

        /* Use the following helper function to create a shader
         * program for each material using the same vertex and
         * fragment shader file. */
        objmaterial->program = new PROGRAM((char *)"default",
                                           (char *)"Uber.vs",
                                           (char *)"Uber.fs",
                                           true, /* Use a relative path. */
                                           true, /* Debug the shaders and program linking. */
                                           /* Custom callback to be
                                            * able to specify the
                                            * attribute location
                                            * before the linking phase
                                            * of the shader program.
                                            */
                                           program_bind_attrib_location,
                                           /* Do not link the draw callback this time */
                                           NULL);

        /* Set the material draw callback to have direct access to the material data before drawing. */
        objmaterial->set_draw_callback(material_draw_callback);
    }
}

void templateAppDraw(void)
{
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    GFX_set_matrix_mode(MODELVIEW_MATRIX);
    GFX_load_identity();
    vec3 e(0.0f, -6.0f, 1.35f), /* The location of the camera. */
         c(0.0f, -5.0f, 1.35f), /* Where the camera is looking. */
         u(0.0f, 0.0f, 1.0f);
    
     GFX_look_at(&e, &c, &u);

    /* Loop for each OBJMESH. */
    for (int i=0; i != obj->objmesh.size(); ++i) {
        /* Push the current model view matrix down. */
        GFX_push_matrix();

        
        /* Translate the model view matrix use the location XYZ of the
         * current mesh.
         */
        GFX_translate(obj->objmesh[i].location.x,
                      obj->objmesh[i].location.y,
                      obj->objmesh[i].location.z);

        /* Draw the mesh and its associated material(s) onscreen. */
        obj->objmesh[i].draw();

        /* Pop the model view matrix back. */
        GFX_pop_matrix();
    }
}

void templateAppExit(void)
{
//    for (int i=0; i<obj->objmaterial.size(); ++i) {
//        delete obj->objmaterial[i].program;
//    }

    delete obj;
}
