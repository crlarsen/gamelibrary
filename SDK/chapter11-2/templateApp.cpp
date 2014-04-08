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
 * - LIGHT
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

#define OBJ_FILE (char *)"Scene.obj"

OBJ *obj = NULL;

TEMPLATEAPP templateApp = {
    templateAppInit,
    templateAppDraw
};

std::vector<OBJMESH>::iterator objmesh;

int viewport_matrix[4];

LIGHT *light = NULL;


vec3    center (0.0f, 0.0f, 0.0f),
        up_axis(0.0f, 0.0f, 1.0f);
/* For convenience, declare a pointer to the projector texture. */
TEXTURE *texture = NULL;
/* The modelview and projection matrix from the projector (the spot
 * point of view).
 */
mat4 projector_matrix;


void program_bind_attrib_location(void *ptr) {
    PROGRAM *program = (PROGRAM *)ptr;

    glBindAttribLocation(program->pid, VA_Position,  VA_Position_String);
    glBindAttribLocation(program->pid, VA_Normal,    VA_Normal_String);
    glBindAttribLocation(program->pid, VA_TexCoord0, VA_TexCoord0_String);
    glBindAttribLocation(program->pid, VA_Tangent0,  VA_Tangent0_String);
}


GFX *gfx = NULL;

void program_draw(void *ptr)
{
    PROGRAM *program = (PROGRAM *)ptr;

    for (auto it=program->uniform_map.begin(); it!=program->uniform_map.end(); ++it){
        auto    &name = it->first;
        auto    &uniform = it->second;

        if (uniform.constant) {
            continue;
        } else if (name == "MODELVIEWPROJECTIONMATRIX") {
            glUniformMatrix4fv(uniform.location,
                               1,
                               GL_FALSE,
                               gfx->get_modelview_projection_matrix().m());
        } else if (name == "PROJECTOR") {
            glUniform1i(uniform.location,
                        0);

            uniform.constant = true;
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
        } else if (name == "PROJECTORMATRIX") {
            glUniformMatrix4fv(uniform.location,
                               1,
                               GL_FALSE,
                               projector_matrix.m());
        } else if (name == "MATERIAL.ambient") {
            // Material Data
            glUniform4fv(uniform.location,
                         1,
                         objmesh->current_material->ambient.v());
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
                         objmesh->current_material->diffuse.v());

            uniform.constant = true;
        } else if (name == "MATERIAL.specular") {
            glUniform4fv(uniform.location,
                         1,
                         objmesh->current_material->specular.v());

            uniform.constant = true;
        } else if (name == "MATERIAL.shininess") {
            glUniform1f(uniform.location,
                        objmesh->current_material->specular_exponent * 0.128f);

            uniform.constant = true;
        }
    }

    // Light Data
    auto spotLight = dynamic_cast<SpotLight *>(light);

    static float rot_angle = 0.0f;

    spotLight->position->x = 7.5f * cosf(rot_angle * DEG_TO_RAD);
    spotLight->position->y = 7.5f * sinf(rot_angle * DEG_TO_RAD);

    rot_angle += 0.25f;

    spotLight->spot_direction = center - vec3(spotLight->position, true);

    spotLight->spot_direction.safeNormalize();
    light->push_to_shader(gfx, program);
}


void templateAppInit(int width, int height) {

    atexit(templateAppExit);

    glViewport(0.0f, 0.0f, width, height);

    glGetIntegerv(GL_VIEWPORT, viewport_matrix);

    gfx = new GFX;

    obj = new OBJ(OBJ_FILE, true);
    
    for (auto objmesh=obj->objmesh.begin();
         objmesh != obj->objmesh.end(); ++objmesh) {

        objmesh->optimize(128);

        objmesh->build();

        objmesh->free_vertex_data();
    }
    
    for (auto texture=obj->texture.begin();
         texture!=obj->texture.end(); ++texture) {
        (*texture)->build(obj->texture_path,
                          TEXTURE_MIPMAP | TEXTURE_16_BITS,
                          TEXTURE_FILTER_2X,
                          0.0f);
    }

    for (auto program=obj->program.begin();
         program != obj->program.end(); ++program) {
        (*program)->build(program_bind_attrib_location,
                          program_draw,
                          true,
                          obj->program_path);
    }

    for (auto objmaterial=obj->objmaterial.begin();
         objmaterial!=obj->objmaterial.end(); ++objmaterial) {
        objmaterial->build(NULL);
    }

    vec4 color(1.0f, 1.0f, 1.0f, 1.0f);

    vec3 position(7.5f, 0.0f, 6.0f);

    light = new SpotLight((char *)"spot", color, position, 0.0f, 0.0f, 0.0f, 75.0f, 0.05f);

    obj->get_mesh((char *)"projector", false)->visible = false;

    texture = obj->get_texture((char *)"projector", false);
    
    glBindTexture(GL_TEXTURE_2D, texture->tid);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}


void draw_scene_from_projector(void)
{
    /* Set up the projection matrix to use the same field of view as the
     * spot.  The idea behind this block is to pretend to render the scene
     * from the eye of the spot, and then, later on, reuse the projection
     * and modelview matrices to transform the vertex position inside the
     * shader program to be in that same coordinate system.  The result of
     * this operation will allow you to automatically generate the UV
     * coordinates from the projector (the spot) and project the texture
     * on the objects of the scene.
     */
    gfx->set_matrix_mode(PROJECTION_MATRIX);
    gfx->load_identity();

    auto spotLight = dynamic_cast<SpotLight *>(light);
    gfx->set_perspective(spotLight->spot_fov,
                         (float)viewport_matrix[2] / (float)viewport_matrix[3],
                           1.0f,
                          20.0f,
                         -90.0f);

    gfx->set_matrix_mode(MODELVIEW_MATRIX);
    gfx->load_identity();
    /* Execute a look_at to be able to gather the modelview matrix.  Note
     * that the position of the light and the camera as well as the
     * projection matrix are the same.  This will allow you to pretend that
     * the scene is rendered from the light point of view without really
     * drawing anything, just gather the necessary matrices to be able to
     * project the texture from the spot.
     */
    gfx->look_at(*(vec3 *)&spotLight->position, center, up_axis);

    projector_matrix[0][0] = 0.5f;
    projector_matrix[0][1] = 0.0f;
    projector_matrix[0][2] = 0.0f;
    projector_matrix[0][3] = 0.0f;

    projector_matrix[1][0] = 0.0f;
    projector_matrix[1][1] = 0.5f;
    projector_matrix[1][2] = 0.0f;
    projector_matrix[1][3] = 0.0f;

    projector_matrix[2][0] = 0.0f;
    projector_matrix[2][1] = 0.0f;
    projector_matrix[2][2] = 0.5f;
    projector_matrix[2][3] = 0.0f;

    projector_matrix[3][0] = 0.5f;
    projector_matrix[3][1] = 0.5f;
    projector_matrix[3][2] = 0.5f;
    projector_matrix[3][3] = 1.0f;

    /* Multiply the bias matrix with the current model view and
     * projection matrix and store the result as the projector_matrix.
     */
    projector_matrix = gfx->get_modelview_projection_matrix() * projector_matrix;
}


void draw_scene(void)
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    gfx->set_matrix_mode(PROJECTION_MATRIX);
    gfx->load_identity();

    gfx->set_perspective( 45.0f,
                         (float)viewport_matrix[2] / (float)viewport_matrix[3],
                           0.1f,
                         100.0f,
                         -90.0f);

    gfx->set_matrix_mode(MODELVIEW_MATRIX);
    gfx->load_identity();

    const float   alpha(-72.0f*DEG_TO_RAD_DIV_2);
    const float   cosAlpha(cosf(alpha)), sinAlpha(sinf(alpha));
    const float   beta(-48.5f*DEG_TO_RAD_DIV_2);
    const float   cosBeta(cosf(beta)), sinBeta(sinf(beta));
    gfx->rotate(quaternion( cosAlpha*cosBeta, sinAlpha*cosBeta,
                           -sinAlpha*sinBeta, cosAlpha*sinBeta));

    gfx->translate(-14.0f, 12.0f, -7.0f);

    /* Bind the projector texture ID to the texture channel 0. */
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture->tid);
    /* Create a local copy of the projector matrix, because it will have
     * to be updated independently for each object and sent over to the
     * shader program.
     */
    mat4 projector_matrix_copy;

    projector_matrix_copy = projector_matrix;

    for (objmesh=obj->objmesh.begin();
         objmesh!=obj->objmesh.end(); ++objmesh) {

        gfx->push_matrix();

        gfx->translate(objmesh->location);

        TStack  l;
        l.loadMatrix(projector_matrix_copy);

        l.translate(objmesh->location);

        projector_matrix = l.back();

        objmesh->draw();
        
        gfx->pop_matrix();
    }
}


void templateAppDraw(void) {
    draw_scene_from_projector();

    draw_scene();
}


void templateAppExit(void) {

    delete light;
    light = NULL;

    delete obj;
}
