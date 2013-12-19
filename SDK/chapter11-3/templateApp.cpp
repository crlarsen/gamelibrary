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

SpotLight *light = NULL;


vec3    center (0.0f, 0.0f, 0.0f),
        up_axis(0.0f, 0.0f, 1.0f);


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

void program_bind_attrib_location(void *ptr) {
    PROGRAM *program = (PROGRAM *)ptr;

    glBindAttribLocation(program->pid, VA_Position,  VA_Position_String);
    glBindAttribLocation(program->pid, VA_Normal,    VA_Normal_String);
    glBindAttribLocation(program->pid, VA_TexCoord0, VA_TexCoord0_String);
    glBindAttribLocation(program->pid, VA_Tangent0,  VA_Tangent0_String);
}


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
                               GFX_get_modelview_projection_matrix().m());
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
                               GFX_get_modelview_matrix().m());
        } else if (name == "PROJECTIONMATRIX") {
            glUniformMatrix4fv(uniform.location,
                               1,
                               GL_FALSE,
                               GFX_get_projection_matrix().m());

            uniform.constant = true;
        } else if (name == "NORMALMATRIX") {
            glUniformMatrix3fv(uniform.location,
                               1,
                               GL_FALSE,
                               GFX_get_normal_matrix().m());
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
    static float rot_angle = 0.0f;

    light->position->x = 7.5f * cosf(rot_angle * DEG_TO_RAD);
    light->position->y = 7.5f * sinf(rot_angle * DEG_TO_RAD);

    rot_angle += 0.25f;

    light->spot_direction = center - vec3(light->position, true);

    light->spot_direction.safeNormalize();
    light->push_to_shader(program);
}


void templateAppInit(int width, int height)
{
    atexit(templateAppExit);

    glViewport(0.0f, 0.0f, width, height);

    glGetIntegerv(GL_VIEWPORT, viewport_matrix);

    GFX_start();

    obj = new OBJ(OBJ_FILE, true);

    for (auto objmesh=obj->objmesh.begin();
         objmesh!=obj->objmesh.end(); ++objmesh) {
        objmesh->optimize(128);

        objmesh->build2();
        
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
    
    /* Get the current frame buffer ID that is bound to the current GL
     * context.  If there is none, the value returned will be less than
     * 0 (if you are using Android); in this case, clamp the main_buffer
     * variable to 0 to unbind the framebuffer.  If you are running the
     * app on iOS you will get the current frame buffer ID and will
     * reuse it to switch between the shadowmap_buffer ID and the
     * original ID created by the system.  Very convenient!
     */
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &main_buffer);

    if (main_buffer < 0) main_buffer = 0;

    /* Generate a new frame buffer ID. */
    glGenFramebuffers(1, &shadowmap_buffer);

    /* Bind the new framebuffer ID. */
    glBindFramebuffer(GL_FRAMEBUFFER, shadowmap_buffer);

    /* Create a new texture ID. */
    glGenTextures(1, &depth_texture);

    /* Bind the new texture ID. */
    glBindTexture(GL_TEXTURE_2D, depth_texture);

    /* Set the magnification and minification filters to not use
     * interpolation.
     */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    /* Set the texture wrap to be clamped to the edge of the texture.
     * Just like in the projector tutorial, this will force the UV
     * values to stay in the range of 0 to 1.
     */
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    /* Create a blank depth texture using the size of the shadow map
     * specified above.  Note that your GL implementation needs to
     * have the extension GL_OES_depth_texture available for this
     * tutorial to work and be able to create a texture using the
     * GL_DEPTH_COMPONENT pixel format.
     */
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_DEPTH_COMPONENT,
                 shadowmap_width,
                 shadowmap_height,
                 0,
                 GL_DEPTH_COMPONENT,
                 /* Request a 16-bit depth buffer. */
                 GL_UNSIGNED_SHORT,
                 NULL);

    /* Unbind the texture. */
    glBindTexture(GL_TEXTURE_2D, 0);

    /* Attach the depth texture to the frame buffer.  This will allow
     * you to use this texture as a depth buffer.
     */
    glFramebufferTexture2D(GL_FRAMEBUFFER,
                           GL_DEPTH_ATTACHMENT,
                           GL_TEXTURE_2D,
                           depth_texture,
                           0);
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
    GFX_set_matrix_mode(PROJECTION_MATRIX);
    GFX_load_identity();

    GFX_set_perspective(light->spot_fov,
                        (float)viewport_matrix[2] / (float)viewport_matrix[3],
                        1.0f,
                        20.0f,
                        -90.0f);

    GFX_set_matrix_mode(MODELVIEW_MATRIX);
    GFX_load_identity();
    /* Execute a look_at to be able to gather the modelview matrix.  Note
     * that the position of the light and the camera as well as the
     * projection matrix are the same.  This will allow you to pretend that
     * the scene is rendered from the light point of view without really
     * drawing anything, just gather the necessary matrices to be able to
     * project the texture from the spot.
     */
    GFX_look_at(*(vec3 *)&light->position, center, up_axis);

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

    projector_matrix =
        GFX_get_modelview_projection_matrix() * projector_matrix;

    /* Bind the shadowmap buffer to redirect the drawing to the shadowmap
     * frame buffer.
     */
    glBindFramebuffer(GL_FRAMEBUFFER, shadowmap_buffer);

    /* Resize the viewport to fit the shadow map width and height. */
    glViewport(0, 0, shadowmap_width, shadowmap_height);

    /* Clear the depth buffer, which will basically clear the content
     * of the depth_texture.
     */
    glClear(GL_DEPTH_BUFFER_BIT);

    /* Cull the front faces.  Because you are trying to render real-time
     * shadows, you are only interested in the back face of the object,
     * which basically is the surface that casts the shadow.  By culling
     * the front face, you will be able to cast shadows for the allow
     * objects to cast shadows on themselves.
     */
    glCullFace(GL_FRONT);

    /* Get the writedepth shader program. */
    PROGRAM *program = obj->get_program("writedepth", false);

    /* Assign the shader to all materials. */
    for (auto objmaterial=obj->objmaterial.begin();
         objmaterial!=obj->objmaterial.end(); ++objmaterial) {
        objmaterial->program = program;
    }

    /* Draw the scene as you normally do.  This will fill the
     * depth_texture values.
     */
    for (objmesh=obj->objmesh.begin();
         objmesh!=obj->objmesh.end(); ++objmesh) {

        GFX_push_matrix();

        GFX_translate(objmesh->location);

        objmesh->draw();

        GFX_pop_matrix();
    }
    
    /* Restore that back faces should be culled. */
    glCullFace(GL_BACK);
}


void draw_scene(void)
{
    glBindFramebuffer(GL_FRAMEBUFFER, main_buffer);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    glViewport(0, 0, viewport_matrix[2], viewport_matrix[3]);

    GFX_set_matrix_mode(PROJECTION_MATRIX);
    GFX_load_identity();

    GFX_set_perspective( 45.0f,
                        (float)viewport_matrix[2] / (float)viewport_matrix[3],
                        0.1f,
                        100.0f,
                        -90.0f);

    GFX_set_matrix_mode(MODELVIEW_MATRIX);
    GFX_load_identity();

    GFX_rotate(-72.0, 1.0f, 0.0f, 0.0f);

    GFX_rotate(-48.5f, 0.0f, 0.0f, 1.0f);

    GFX_translate(-14.0f, 12.0f, -7.0f);

    mat4 projector_matrix_copy;
    
    projector_matrix_copy = projector_matrix;
    
    /* Get the lighting shader program. */
    PROGRAM *program = obj->get_program("lighting", false);

    /* Attach the lighting program to all materials for the current OBJ
     * structure.
     */
    for (auto objmaterial=obj->objmaterial.begin();
         objmaterial!=obj->objmaterial.end(); ++objmaterial) {
        objmaterial->program = program;
    }
    
    /* Bind and make the depth_texture active on the texture channel 0. */
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depth_texture);

    /* Reset the counter to loop through the objects. */
    for (objmesh=obj->objmesh.begin();
         objmesh!=obj->objmesh.end(); ++objmesh) {

        GFX_push_matrix();

        GFX_translate(objmesh->location);

        projector_matrix = projector_matrix_copy;

        mat4_translate(projector_matrix, projector_matrix, objmesh->location);

        objmesh->draw();
        
        GFX_pop_matrix();
    }
}


void templateAppDraw(void) {
    draw_scene_from_projector();

    draw_scene();
}


void templateAppExit(void) {
    glDeleteFramebuffers(1, &shadowmap_buffer);
    glDeleteTextures(1, &depth_texture);

    delete light;
    light = NULL;

    delete obj;
}
