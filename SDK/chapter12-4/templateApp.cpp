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

/* To store the idle MD5ACTION pointer. */
MD5ACTION *idle = NULL;

/* The walk action pointer. */
MD5ACTION *walk = NULL;

/* The joints array that will be used to hold the final pose of the
 * skeleton after the idle and walk actions are blended together.
 */
MD5JOINT *final_pose = NULL;

TEMPLATEAPP templateApp = { templateAppInit,
							templateAppDraw,
							templateAppToucheBegan,
							templateAppToucheMoved };

int viewport_matrix[4];


LIGHT *light = NULL;


vec2 touche = { 0.0f, 0.0f }; 

vec3 rot_angle = { 0.0f, 0.0f, 0.0f };

unsigned char auto_rotate = 0; 


void program_bind_attrib_location(void *ptr) {
	PROGRAM *program = (PROGRAM *)ptr;

	glBindAttribLocation(program->pid, 0, "POSITION");
	glBindAttribLocation(program->pid, 1, "NORMAL");
	glBindAttribLocation(program->pid, 2, "TEXCOORD0");
	glBindAttribLocation(program->pid, 3, "TANGENT0");
}


void material_draw(void *ptr)
{
	OBJMATERIAL *objmaterial = (OBJMATERIAL *)ptr;

	PROGRAM *program = objmaterial->program;

    for (auto it=program->uniform_map.begin(); it!=program->uniform_map.end(); ++it) {
        auto    &name = it->first;
        auto    &uniform = it->second;

		if (uniform.constant) {
			continue;
		} else if (name == "DIFFUSE") {
			glUniform1i(uniform.location, 1);

			uniform.constant = true;
		} else if (name == "BUMP") {
			glUniform1i(uniform.location, 4);

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
                         (float *)&objmaterial->ambient);
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
                         (float *)&objmaterial->diffuse);
        } else if (name == "MATERIAL.specular") {
			glUniform4fv(uniform.location,
                         1,
                         (float *)&objmaterial->specular);
        } else if (name == "MATERIAL.shininess") {
			glUniform1f(uniform.location,
                        objmaterial->specular_exponent * 0.128f);

			uniform.constant = true;
		} else if (name == "LIGHT_FS.color") {
            // Lamp Data
			glUniform4fv(uniform.location,
                         1,
                         (float *)&light->color);

			uniform.constant = true;
		} else if (name == "LIGHT_VS.direction") {
			vec3 direction;

			LIGHT_get_direction_in_eye_space(light,
                                             &gfx.modelview_matrix[gfx.modelview_matrix_index - 1],
                                             &direction);

			glUniform3fv(uniform.location,
                         1,
                         (float *)&direction);

			uniform.constant = true;
		}
	}
}


void templateAppInit(int width, int height)
{
	atexit(templateAppExit);

	GFX_start();

	glViewport(0.0f, 0.0f, width, height);

	glGetIntegerv(GL_VIEWPORT, viewport_matrix);

	vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };

	light = LIGHT_create_directional((char *)"point", &color, 45.0f, 0.0f, 0.0f);

    /* Manually initialize a blank OBJ structure.  You do not need to
     * use the OBJ_load function this time, because there's no geometry
     * to load, only a material file.
     */
//    obj = (OBJ *) calloc(1, sizeof(OBJ));
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

    /* Build the materials and associate the material_draw callback
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
    md5 = MD5_load_mesh(MD5_MESH, 1);

    /* Convert the triangles to triangle strips. */
    MD5_optimize(md5, 128);

    /* Build the VBO and VAO and construct the normals and tangents for
     * each face of the meshes.
     */
    MD5_build(md5);

    /* Loop while there are some mesh parts. */
    for (int i=0; i!=md5->n_mesh; ++i) {
        /* The current mesh pointer. */
        MD5MESH *md5mesh = &md5->md5mesh[i];

        /* Query the OBJ material database to get the objmaterial
         * pointer for the current mesh part.  Note that for the MD5
         * format, each part name is considered as a shader that
         * corresponds to the same material entry name in the OBJ
         * material file.
         */
        MD5_set_mesh_material(md5mesh,
                              obj->get_material(md5mesh->shader, false));
    }

    /* Load the actions from the disk. */
    MD5_load_action(md5,
                    (char *)"idle",  // Internal name for this action.
                    (char *)"bob_idle.md5anim",  // The action file name.
                    1);   // Use a relative path to load the action file.
    MD5_load_action(md5,
                    (char *)"walk",
                    (char *)"bob_walk.md5anim",
                    1);

    /* Retrieve the pointer of the idle action. */
    idle = MD5_get_action(md5, (char *)"idle", 0);

    /* Set the frame rate that want to use to play back the animation. */
    MD5_set_action_fps(idle, 24.0f);

    /* Start playing the animation using looping. */
//    MD5_action_play(idle,
//                    /* The method to use to interpolate between frames.
//                     * For this first example, simply use the
//                     * MD5_METHOD_FRAME method to represent that each
//                     * frame of the sequence will be played sequentially
//                     * one after the other (no interpolation between
//                     * each frame).
//                     */
//                     MD5_METHOD_FRAME,
//                    /* Specify whether or not the animation should loop
//                     * when the end frame is reached (either 1 or 0).
//                     */
//                     1);

//    MD5_action_play(idle,
//                     MD5_METHOD_LERP,
//                     1);

    MD5_action_play(idle,
                    MD5_METHOD_SLERP,
                    1);

    /* Get the walk animation pointer. */
    walk = MD5_get_action(md5, (char *)"walk", 0);

    /* Set the rate of the animation playback. */
    MD5_set_action_fps(walk, 24.0f);

    /* Play the walk action using spherical interpolation between frames
     * and loop when the end frame is reached.
     */
    MD5_action_play(walk,
                    MD5_METHOD_SLERP,
                    1);

    /* Initialize a temporary skeleton to be able to store the final
     * pose after blending the idle and walk animation.
     */
    final_pose = (MD5JOINT *) calloc(md5->n_joint, sizeof(MD5JOINT));

    /* Free the mesh data that used to build the mesh, because this data
     * is no longer required for drawing.
     */
    MD5_free_mesh_data(md5);

    /* Disable the cull face to make sure that even backfaces will drawn
     * onscreen.
     */
    glDisable(GL_CULL_FACE);
}


void templateAppDraw(void)
{
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);


	GFX_set_matrix_mode(PROJECTION_MATRIX);
	GFX_load_identity();

	GFX_set_perspective( 45.0f,
                        (float)viewport_matrix[2] / (float)viewport_matrix[3],
                          0.1f,
                        100.0f,
                          0.0f);


	GFX_set_matrix_mode(MODELVIEW_MATRIX);
	GFX_load_identity();

	GFX_translate(0.0f, -14.0f, 3.0f);

	GFX_rotate(90.0, 1.0f, 0.0f, 0.0f);

	mat4_invert(GFX_get_modelview_matrix());

    GFX_push_matrix();

    /* If auto-rotate is ON, simply turn the geometry on the Z axis,
     * demo reel style.
     */
    if (auto_rotate) rot_angle.z += 1.0f;

    /* Rotate the X and Z axis based on the rotation specified by the
     * user.
     */
    GFX_rotate(rot_angle.x, 1.0f, 0.0f, 0.0f);
    GFX_rotate(rot_angle.z, 0.0f, 0.0f, 1.0f);

    /* Control the direction of the blending, from idle to walk and from
     * walk to idle (for demonstration purposes only, of course).
     */
    static unsigned char blend_direction = 0;

    /* The blend factor to use to mix the two animation sequences
     * together.  This factor is basically a value from 0 to 1 that
     * determines how much the two actions will be blended together
     * (from the first to the second).
     */
    static float blend_factor = 0.0f;

    /* If the blend_direction is 0, increase the blend factor to
     * progressively push lazy Bob to start walking.
     */
    if (!blend_direction) blend_factor += 0.001f;
    /* If the blend_direction is a value other than 0, progressivly
     * decrease the blend factor (to let Bob have a rest) and
     * interpolate the walk animation back to the idle action.
     */
    else blend_factor -= 0.001f;

    /* If the blend factor is less than 0 or greater than 1, invert the
     * blend_direction.
     */
    if (blend_factor < 0.0f || blend_factor > 1.0f)
    { blend_direction = !blend_direction; }

    /* Increase the time step for all actions with statuses that are set
     * to PLAY.
     */
    MD5_draw_action(md5, 1.0f / 60.0f);

    /* This is the interesting part.  This function receives two
     * skeleton poses and blends them together based on the a blend
     * factor.  The resulting skeleton will then be stored in the
     * final_pose array of joints.
     */
    MD5_add_pose(md5,
                 /* The final skeleton. */
                 final_pose,
                 /* The first action to use for blending. */
                 idle,
                 /* The second action to use for blending. */
                 walk,
                 /* The interpolation method to use to interpolate
                  * the first and second action poses together.
                  */
                 MD5_METHOD_SLERP,
                 /* The weight of the walk action (the second action
                  * passed in parameter to the function) since you are
                  * using addition, the idle animation will always have
                  * the priority.  This factor will control how much
                  * percentage of the walk animation will be added to
                  * the idle animation.
                  */
                 CLAMP(blend_factor, 0.0f, 1.0f));
    
    /* Use the final pose skeleton to calculate and update the vertex
     * positions of the skeleton's skin and the VBO with the latest
     * interpolated skeleton.
     */
    MD5_set_pose(md5, final_pose);
    
    /* Draw the model onscreen. */
    MD5_draw(md5);
    
    GFX_pop_matrix();
}


void templateAppToucheBegan(float x, float y, unsigned int tap_count)
{
	if (tap_count == 2) auto_rotate = !auto_rotate;
    
	touche.x = x;
	touche.y = y;
}


void templateAppToucheMoved(float x, float y, unsigned int tap_count)
{
	auto_rotate = 0;
    
	rot_angle.z += x - touche.x;
	rot_angle.x += y - touche.y;
    
	touche.x = x;
	touche.y = y;
}


void templateAppExit(void)
{
    free(final_pose);
    delete obj;
    md5 = MD5_free(md5);
    
	light = LIGHT_free(light);
}
