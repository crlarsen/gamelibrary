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
 * - FONT
 * - GFX
 * - LIGHT
 * - MD5
 * - MEMORY
 * - NAVIGATION
 * - OBJ
 * - OBJMATERIAL
 * - OBJMESH
 * - OBJTRIANGLEINDEX
 * - OBJTRIANGLELIST
 * - OBJVERTEXDATA
 * - PROGRAM
 * - SHADER
 * - SOUND
 * - TEXTURE
 * - THREAD
 */

#include "templateApp.h"

/* The material file. */
#define MTL_FILE (char *)"bob.mtl"

/* The MD5 mesh file. */
#define MD5_MESH (char *)"bob.md5mesh"

/* A fresh OBJ pointer which you will use strictly to load the materials
 * associated with the MD5 mesh.
 */
OBJ *obj = NULL;

/* An empty MD5 structure pointer.  For more information about this
 * structure, feel free to consult the file md5.cpp and its associated
 * header located inside the common directory of the SDK.
 */
MD5 *md5 = NULL;

TEMPLATEAPP templateApp = {
    templateAppInit,
    templateAppDraw,
    templateAppToucheBegan,
    templateAppToucheMoved
};

int viewport_matrix[4];


LIGHT *light = NULL;


vec2 touche(0.0f, 0.0f);

vec3 rot_angle(0.0f, 0.0f, 0.0f);

bool auto_rotate = false;


void program_bind_attrib_location(void *ptr) {
    PROGRAM *program = (PROGRAM *)ptr;

    glBindAttribLocation(program->pid, VA_Position,  VA_Position_String);
    glBindAttribLocation(program->pid, VA_Normal,    VA_Normal_String);
    glBindAttribLocation(program->pid, VA_TexCoord0, VA_TexCoord0_String);
    glBindAttribLocation(program->pid, VA_Tangent0,  VA_Tangent0_String);
}


GFX *gfx = NULL;

void material_draw(void *ptr)
{
    OBJMATERIAL *objmaterial = (OBJMATERIAL *)ptr;

    PROGRAM *program = objmaterial->program;

    for (auto it=program->uniform_map.begin(); it!=program->uniform_map.end(); ++it) {
        auto    &name = it->first;
        auto    &uniform = it->second;

        if (uniform.constant) {
            continue;
        } else if (name == TM_Diffuse_String) {
            glUniform1i(uniform.location, TM_Diffuse);

            uniform.constant = true;
        } else if (name == TM_Bump_String) {
            glUniform1i(uniform.location, TM_Bump);

            uniform.constant = true;
        } else if (name == "MODELVIEWMATRIX") {
            // Matrix Data
            glUniformMatrix4fv(uniform.location,
                               1,
                               GL_FALSE,
                               gfx->get_modelview_matrix().m());
        } else if (name == "PROJECTIONMATRIX") {
            glUniformMatrix4fv(uniform.location,
                               1,
                               GL_FALSE,
                               gfx->get_projection_matrix().m());

            uniform.constant = true;
        } else if (name == "NORMALMATRIX") {
            glUniformMatrix3fv(uniform.location,
                               1,
                               GL_FALSE,
                               gfx->get_normal_matrix().m());
        } else if (name == "MATERIAL.ambient") {
            // Material Data
            glUniform4fv(uniform.location,
                         1,
                         objmaterial->ambient.v());
            /* In this scene, all the materials (in this case, there are
             * only two) have the exact same properties, so simply tag the
             * uniforms for the current material to be constant.  This will
             * also allow you to get better performance at runtime, because
             * the data will not be sent over and over for nothing.
             */
            uniform.constant = true;
        } else if (name == "MATERIAL.diffuse") {
            glUniform4fv(uniform.location,
                         1,
                         objmaterial->diffuse.v());
        } else if (name == "MATERIAL.specular") {
            glUniform4fv(uniform.location,
                         1,
                         objmaterial->specular.v());
        } else if (name == "MATERIAL.shininess") {
            glUniform1f(uniform.location,
                        objmaterial->specular_exponent * 0.128f);

            uniform.constant = true;
        }
    }

    // Lamp Data
    light->push_to_shader(gfx, program);
}


void templateAppInit(int width, int height)
{
    atexit(templateAppExit);

    gfx = new GFX;

    glViewport(0.0f, 0.0f, width, height);

    glGetIntegerv(GL_VIEWPORT, viewport_matrix);

    vec4 color(1.0f, 1.0f, 1.0f, 1.0f);

    light = new DirectionalLight((char *)"point", color, 45.0f, 0.0f, 0.0f);

    /* Manually initialize a blank OBJ structure.  You do not need to
     * use the OBJ::load function this time, because there's no geometry
     * to load, only a material file.
     */
    obj = new OBJ;

    /* Manually load the material file using the filename you defined at
     * the beginning of the current source file.
     */
    obj->load_mtl(MTL_FILE, true);

    /* Build the textures. */
    for (auto texture=obj->texture.begin();
         texture!=obj->texture.end(); ++texture) {
        (*texture)->build(obj->texture_path,
                          TEXTURE_MIPMAP | TEXTURE_CLAMP | TEXTURE_16_BITS,
                          TEXTURE_FILTER_3X,
                          0.0f);
    }

    /* Build the shader programs (in this case, there's only one). */
    for (auto program=obj->program.begin();
         program!=obj->program.end(); ++program) {
        (*program)->build(program_bind_attrib_location,
                          NULL,
                          true,
                          obj->program_path);
    }

    /* Build the materials ans associate the material_draw callback
     * function to each of them to be able to set the uniform variables
     * of the shader program.
     */
    for (auto objmaterial=obj->objmaterial.begin();
         objmaterial!=obj->objmaterial.end(); ++objmaterial) {
        objmaterial->build(NULL);
        /* Set a material callback so every time the material is about
         * to be used for drawing, the material_draw function will be
         * triggered by the execution pointer.
         */
        objmaterial->set_draw_callback(material_draw);
    }

    /* Load the MD5 mesh file from disk. */
    md5 = new MD5(MD5_MESH, true);

    /* Convert the triangles to triangle strips. */
    md5->optimize(128);

    /* Build the VBO and VAO and construct the normals and tangents for
     * each face of the meshes.
     */
    md5->build();

    /* Loop while there are some mesh parts. */
    for (auto md5mesh=md5->md5mesh.begin();
         md5mesh!=md5->md5mesh.end(); ++md5mesh) {
        /* Query the OBJ material database to get the objmaterial
         * pointer for the current mesh part.  Note that for the MD5
         * format, each part name is considered as a shader that
         * corresponds to the same material entry name in the OBJ
         * material file.
         */
        md5mesh->set_mesh_material(obj->get_material(md5mesh->shader,
                                                     false));
    }

    /* Free the mesh data that used to build the mesh, because this data
     * is no longer required for drawing.
     */
    md5->free_mesh_data();

    /* Disable the cull face to make sure that even backfaces will drawn
     * onscreen.
     */
    glDisable(GL_CULL_FACE);
}


void templateAppDraw(void)
{
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);


    gfx->set_matrix_mode(PROJECTION_MATRIX);
    gfx->load_identity();

    // Adjust "Field of View Y" angle for devices which has an aspect
    // ratio which is wider than the origin iPhone (3:2).  Devices which
    // have a narrower aspect ratio (such as iPad) work fine, as is.
    const float iPhoneOriginalWidth =320.0f;
    const float iPhoneOriginalHeight=480.0f;
    const float originalFovy=45.0f;
    float fovy(originalFovy);
    if (viewport_matrix[3]*iPhoneOriginalWidth > viewport_matrix[2]*iPhoneOriginalHeight) {
        float   h = (iPhoneOriginalHeight*0.5f) / tanf(originalFovy*0.5f*DEG_TO_RAD);
        fovy = 2.0f * atan2f(((float)viewport_matrix[3])*0.5, h) * RAD_TO_DEG;
    }
    gfx->set_perspective(fovy,
                         (float)viewport_matrix[2] / (float)viewport_matrix[3],
                           0.1f,
                         100.0f,
                           0.0f);


    gfx->set_matrix_mode(MODELVIEW_MATRIX);
    gfx->load_identity();

    // Rotate -90 degrees
    static const quaternion q(M_SQRT1_2, -M_SQRT1_2, 0, 0);
    gfx->rotate(q);

    gfx->translate(0.0f, 14.0f, -3.0f);

    gfx->push_matrix();

    /* If auto-rotate is ON, simply turn the geometry on the Z axis,
     * demo reel style.
     */
    if (auto_rotate) rot_angle->z += 1.0f;

    /* Rotate the X and Z axis based on the rotation specified by the
     * user.
     */
    float   alpha(rot_angle->x*DEG_TO_RAD_DIV_2);
    float   cosAlpha(cosf(alpha)), sinAlpha(sinf(alpha));
    float   beta (rot_angle->z*DEG_TO_RAD_DIV_2);
    float   cosBeta (cosf(beta)),  sinBeta (sinf(beta));
    gfx->rotate(quaternion( cosAlpha*cosBeta, sinAlpha*cosBeta,
                           -sinAlpha*sinBeta, cosAlpha*sinBeta));

    /* Draw the model onscreen. */
    md5->draw();

    gfx->pop_matrix();
}


void templateAppToucheBegan(float x, float y, unsigned int tap_count)
{
    if (tap_count == 2) auto_rotate = !auto_rotate;

    touche->x = x;
    touche->y = y;
}


void templateAppToucheMoved(float x, float y, unsigned int tap_count)
{
    auto_rotate = false;

    rot_angle->z += x - touche->x;
    rot_angle->x += y - touche->y;

    touche->x = x;
    touche->y = y;
}


void templateAppExit(void) {
    delete obj;
    obj = NULL;
    delete md5;
    md5 = NULL;

    delete light;
    light = NULL;
}
