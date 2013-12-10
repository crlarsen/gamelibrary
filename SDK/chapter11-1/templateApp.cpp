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

/* The color buffer texture ID */
unsigned int colorbuffer_texture = 0,
/* The width and height of the texture.  When it comes to
 * texture in OpenGLES, a width and height using a power
 * of 2 will always give better performance compared to a
 * non-power of two texture (assuming that the non-power
 * of 2 texture extension is supported by your hardware).
 */
colorbuffer_width	 = 128,
colorbuffer_height	 = 256;
/* The number of pixels to use in order to blur the texture vertically
 * and horizontally.
 */
float blur_radius = 2.0f;
/* Pointer to the fullscreen object. */
OBJMESH *fullscreen = NULL;
/* Flag to be able to track the current rendering pass, because you will
 * have to determine if the scene should be rendered with only diffuse
 * or only specular colors.
 */
unsigned char pass = 0;

#define OBJ_FILE (char *)"Scene.obj"

OBJ *obj = NULL;

TEMPLATEAPP templateApp = {
    templateAppInit,
    templateAppDraw
};

std::vector<OBJMESH>::iterator objmesh;

int viewport_matrix[4];

LIGHT *light = NULL;


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
                               (float *)GFX_get_modelview_projection_matrix());
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
                               (float *)GFX_get_modelview_matrix());
        } else if (name == "PROJECTIONMATRIX") {
            glUniformMatrix4fv(uniform.location,
                               1,
                               GL_FALSE,
                               (float *)GFX_get_projection_matrix());

            uniform.constant = true;
        } else if (name == "NORMALMATRIX") {
            glUniformMatrix3fv(uniform.location,
                               1,
                               GL_FALSE,
                               (float *)GFX_get_normal_matrix());
        } else if (name == "MATERIAL.ambient") {
            // Material Data
            glUniform4fv(uniform.location,
                         1,
                         (float *)&objmesh->current_material->ambient);
            /* In this scene, all the materials (in this case, there are
             * only two) have the exact same properties, so simply tag the
             * uniforms for the current material to be constant.  This will
             * also allow you to get better performance at runtime, because
             * the data will not be sent over and over for nothing.
             */
            uniform.constant = true;
        } else if (name == "MATERIAL.diffuse") {
            if (pass == 1) {
                vec4 black(0.0f, 0.0f, 0.0f, 1.0f);

                glUniform4fv(uniform.location,
                             1,
                             (float *)&black);
            } else {
                glUniform4fv(uniform.location,
                             1,
                             (float *)&objmesh->current_material->diffuse);
            }
        } else if (name == "MATERIAL.specular") {
            if (pass == 2) {
                vec4 black(0.0f, 0.0f, 0.0f, 1.0f);
                /*
                 // Or your could use half of the original specular color like this (which also gives good results):
                (objmesh->current_material->specular.x * 0.5f,
                 objmesh->current_material->specular.y * 0.5f,
                 objmesh->current_material->specular.z * 0.5f,
                 1.0f };
                 */

                glUniform4fv(uniform.location,
                             1,
                             (float *)&black);
            } else {
                glUniform4fv(uniform.location,
                             1,
                             (float *)&objmesh->current_material->specular);
            }
        } else if (name == "MATERIAL.shininess") {
            glUniform1f(uniform.location,
                        objmesh->current_material->specular_exponent * 0.128f);

            uniform.constant = true;
        }
    }

    // Light Data
    static float rot_angle = 0.0f;

    auto pointLight = dynamic_cast<PointLight *>(light);
    if (pointLight) {
        pointLight->position.x = 7.5f * cosf(rot_angle * DEG_TO_RAD);
        pointLight->position.y = 7.5f * sinf(rot_angle * DEG_TO_RAD);
    }

    rot_angle += 0.25f;

    light->push_to_shader(program);
}


void templateAppInit(int width, int height) {

    atexit(templateAppExit);

    GFX_start();

    glViewport(0.0f, 0.0f, width, height);

    glGetIntegerv(GL_VIEWPORT, viewport_matrix);

    obj = new OBJ(OBJ_FILE, true);
    
    for (objmesh=obj->objmesh.begin();
         objmesh != obj->objmesh.end(); ++objmesh) {

        objmesh->optimize(128);

        objmesh->build();

        objmesh->free_vertex_data();
    }

    for (auto texture=obj->texture.begin();
         texture!=obj->texture.end(); ++texture) {
        (*texture)->build(obj->texture_path,
                          TEXTURE_MIPMAP | TEXTURE_16_BITS,
                          TEXTURE_FILTER_3X,
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

    light = new PointLight((char *)"point", color, position);


    /* Generate a new texture ID. */
    glGenTextures(1, &colorbuffer_texture);
    /* Bind the new texture ID. */
    glBindTexture(GL_TEXTURE_2D, colorbuffer_texture);
    /* Make sure that the texture coordinates will be clamped to the
     * range of 0 to 1.
     */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    /* Specify that the texture pixels will be linearly interpolated
     * when magnified or minified.
     */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    /* Create a new 2D image. */
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 /* Only RGB, no alpha necessary. */
                 GL_RGB,
                 /* Specify the width and height of the texture are
                  * based on teh values that you set above.  This is
                  * necessary to properly render the color buffer to
                  * the texture, because you are going to need to
                  * resize the glViewport to fit these values.
                  */
                 colorbuffer_width,
                 colorbuffer_height,
                 0,
                 GL_RGB,
                 GL_UNSIGNED_SHORT_5_6_5,
                 /* No pixel data is needed because you are going to
                  * dynamically fill the texture when requesting to
                  * render the current color buffer to it.
                  */
                 NULL);	
    
    fullscreen = obj->get_mesh("fullscreen", false);
    fullscreen->visible = false;
}


void draw_scene(void)
{
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    GFX_set_matrix_mode(MODELVIEW_MATRIX);
    GFX_load_identity();

    GFX_rotate(-72.0, 1.0f, 0.0f, 0.0f);

    GFX_rotate(-48.5f, 0.0f, 0.0f, 1.0f);

    GFX_translate(-14.0f, 12.0f, -7.0f);

    for (objmesh=obj->objmesh.begin();
         objmesh!=obj->objmesh.end(); ++objmesh) {

        GFX_push_matrix();

        GFX_translate(objmesh->location.x,
                      objmesh->location.y,
                      objmesh->location.z);
        
        objmesh->draw();
        
        GFX_pop_matrix();
    }
}

void first_pass(void)
{
    /* Tag that the first pass is about to be drawn onscreen. */
    pass = 1;
    /* This is really important!  Resize the glViewport to fit the
     * texture width and height, but do not recalculate the aspect
     * ratio of the perspective matrix.  The image will be scaled to
     * fit the screen resolution, so you have to make sure that the
     * perspective ration in which the scene is drawn fits the
     * original fullscreen viewport size.
     */
    glViewport(0, 0, colorbuffer_width, colorbuffer_height);
    /* Call the draw_scene function and render the scene at a lower
     * resolution with the same width and height of the texture that
     * you want to save the color buffer result to.
     */
    draw_scene();
    /* Bind the color buffer texture ID to be able to save the image. */
    glBindTexture(GL_TEXTURE_2D, colorbuffer_texture);
    /* The function to call in order to transfer the current result of
     * the color buffer to an arbitrary texture ID previously bound to
     * the current GL context.
     */
    glCopyTexSubImage2D(GL_TEXTURE_2D,
                        0, 0, 0, 0, 0,
                        colorbuffer_width,
                        colorbuffer_height);
}


void second_pass(void)
{
    pass = 2;

    glViewport(0, 0, viewport_matrix[2], viewport_matrix[3]);

    draw_scene();
}


void fullscreen_pass(void)
{
    GFX_set_matrix_mode(PROJECTION_MATRIX);
    GFX_load_identity();

    float   half_width  = (float)viewport_matrix[2] * 0.5f,
            half_height = (float)viewport_matrix[3] * 0.5f;

    GFX_load_identity();

    GFX_set_orthographic_2d(-half_width,
                            half_width,
                            -half_height,
                            half_height);

    GFX_set_matrix_mode(MODELVIEW_MATRIX);
    GFX_load_identity();


    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    glDisable(GL_CULL_FACE);

    glEnable(GL_BLEND);

    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);


    glActiveTexture(GL_TEXTURE1);
    
    glBindTexture(GL_TEXTURE_2D, colorbuffer_texture);
    
    
    fullscreen->visible = true;
    {
        PROGRAM *program = obj->get_program("blur", false);

        GFX_scale((float)viewport_matrix[2],
                  (float)viewport_matrix[3],
                  1.0f);

        GFX_rotate(180.0f, 1.0f, 0.0f, 0.0f);


        program->draw();


        vec2    radius(blur_radius / (float)colorbuffer_width, 0.0f);

        glUniform2fv(program->get_uniform_location((char *)"BLUR_RADIUS"),
                     1,
                     (float *)&radius);

        auto objmesh = obj->get_mesh("fullscreen", false);
        objmesh->draw();


        radius.x = 0.0f;
        radius.y = blur_radius / (float)colorbuffer_height;

        glUniform2fv(program->get_uniform_location((char *)"BLUR_RADIUS"),
                     1,
                     (float *)&radius);

        
        
        objmesh->draw();
    }
    fullscreen->visible = false;
    
    
    glDisable(GL_BLEND);
    
    glEnable(GL_CULL_FACE);
    
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
}


void templateAppDraw(void) {

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    GFX_set_matrix_mode(PROJECTION_MATRIX);
    GFX_load_identity();

    GFX_set_perspective( 45.0f,
                        (float)viewport_matrix[2] / (float)viewport_matrix[3],
                        0.1f,
                        100.0f,
                        -90.0f);

    first_pass();

    second_pass();

    //glClear(GL_COLOR_BUFFER_BIT);

    fullscreen_pass();
}


void templateAppExit(void) {

    glDeleteTextures(1, &colorbuffer_texture);
    
    delete light;

    delete obj;
}
