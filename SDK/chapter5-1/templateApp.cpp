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

#define OBJ_FILE (char *)"ram.obj"

#define VERTEX_SHADER (char *)"vertex.glsl"

#define FRAGMENT_SHADER (char *)"fragment.glsl"

OBJ *obj = NULL;

bool    auto_rotate = false;

vec2 touche = { 0.0f, 0.0f }; 

vec3 rot_angle = { 0.0f, 0.0f, 0.0f };


TEMPLATEAPP templateApp = {
    templateAppInit,
    templateAppDraw,
    templateAppToucheBegan,
    templateAppToucheMoved
};


void program_bind_attrib_location(void *ptr) {
    PROGRAM *program = (PROGRAM *)ptr;

    glBindAttribLocation(program->pid, VA_Position,  VA_Position_String);
    glBindAttribLocation(program->pid, VA_TexCoord0, VA_TexCoord0_String);
}


void material_draw_callback(void *ptr)
{
    OBJMATERIAL *objmaterial = (OBJMATERIAL *)ptr;

    PROGRAM *program = objmaterial->program;

    for (auto it=program->uniform_map.begin(); it!=program->uniform_map.end(); ++it) {
        auto    &name = it->first;
        auto    &uniform = it->second;

        if (name == "MODELVIEWMATRIX") {
            glUniformMatrix4fv(uniform.location,
                               1,
                               GL_FALSE,
                               (float *)GFX_get_modelview_matrix());
        } else if (name == "PROJECTIONMATRIX") {
            glUniformMatrix4fv(uniform.location,
                               1,
                               GL_FALSE,
                               (float *)GFX_get_projection_matrix());
        } else if ((name == TM_Diffuse_String) && !uniform.constant) {
            uniform.constant = true;
            glUniform1i(uniform.location, TM_Diffuse);
        } else if (name == MP_Diffuse) {
            glUniform3fv(uniform.location,
                         1,
                         (float *)&objmaterial->diffuse);
        }
    }
}


void templateAppInit(int width, int height) {

    atexit(templateAppExit);

    GFX_start();

    glViewport(0.0f, 0.0f, width, height);

    GFX_set_matrix_mode(PROJECTION_MATRIX);
    GFX_load_identity();
    GFX_set_perspective(45.0f,
                        (float)width / (float)height,
                        0.1f,
                        100.0f,
                        0.0f);

    obj = new OBJ(OBJ_FILE, true);

    for (auto objmesh=obj->objmesh.begin();
         objmesh!=obj->objmesh.end(); ++objmesh) {
        console_print("%s: %d: GL_TRIANGLES\n",
                      objmesh->name,
                      objmesh->objtrianglelist[0].n_indice_array);

        /* Built-in method that implements the NvTriStrip library.
         * For more information, check the obj.cpp source code in
         * order to implement it inside your own apps.
         */
        objmesh->optimize(128);

        console_print("%s: %d: GL_TRIANGLE_STRIP\n",
                      objmesh->name,
                      objmesh->objtrianglelist[0].n_indice_array);


        objmesh->build();

        objmesh->free_vertex_data();
    }


    for (auto texture=obj->texture.begin();
         texture!=obj->texture.end(); ++texture) {
        (*texture)->build(obj->texture_path,
                          TEXTURE_MIPMAP | TEXTURE_16_BITS, // Automatically convert the texture to 16 bits.
                          TEXTURE_FILTER_2X,
                          0.0f);
    }


    for (auto objmaterial=obj->objmaterial.begin();
         objmaterial!=obj->objmaterial.end(); ++objmaterial) {
        objmaterial->build(new PROGRAM((char *)"default",
                                       VERTEX_SHADER,
                                       FRAGMENT_SHADER,
                                       true,
                                       true,
                                       program_bind_attrib_location,
                                       NULL));
        
        objmaterial->set_draw_callback(material_draw_callback);
    }
}


void templateAppDraw(void) {

    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    GFX_set_matrix_mode(MODELVIEW_MATRIX);
    GFX_load_identity();
    {

        vec3    e = { 0.0, -3.0f, 0.0f },
                c = { 0.0f, 0.0f, 0.0f },
                u = { 0.0f, 0.0f, 1.0f };

        GFX_look_at(&e, &c, &u);
    }

    for (auto objmesh=obj->objmesh.begin();
         objmesh!=obj->objmesh.end(); ++objmesh) {
        GFX_push_matrix();

        if (auto_rotate) rot_angle.z += 2.0f;

        GFX_rotate(rot_angle.x, 1.0f, 0.0f, 0.0f);
        GFX_rotate(rot_angle.z, 0.0f, 0.0f, 1.0f);

        objmesh->draw();

        GFX_pop_matrix();
    }
}


void templateAppToucheBegan(float x, float y, unsigned int tap_count) {
    if (tap_count == 2) auto_rotate = !auto_rotate;

    touche.x = x;
    touche.y = y;
}


void templateAppToucheMoved(float x, float y, unsigned int tap_count) {
    auto_rotate = false;

    rot_angle.z += -(touche.x - x);
    rot_angle.x += -(touche.y - y);
    
    touche.x = x;
    touche.y = y;
}


void templateAppExit(void) {
    delete obj;
}
