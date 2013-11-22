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

#define OBJ_FILE (char *)"Scene.obj"

OBJ *obj = NULL;

TEMPLATEAPP templateApp = { templateAppInit,
							templateAppDraw };

std::vector<OBJMESH>::iterator objmesh;

int viewport_matrix[4];

enum LampType {
    LampDirectional          = 0,
    LampPoint                = 1,
    LampPointWithAttenuation = 2,
    LampSphericalPoint       = 3,
    LampSpot                 = 4
};

typedef struct {
    char    name[MAX_CHAR];
    vec4    color;
    vec3    direction;
    vec4    position;   // Position of the lamp in world coordinates
    /* Affect the attenuation of the light based on its distance from
     * the fragment.
     */
    float   linear_attenuation;
    /* Affect the attenuation of the light based on the square of the
     * distance of the fragment from the light.
     */
    float   quadratic_attenuation;
    /* The falloff distance of the light.  The light will be at half of
     * its original intensity at this distance.
     */
    float   distance;
    unsigned char type;
} LAMP;

LAMP *lamp = NULL;

LAMP *LAMP_create_directional(char *name,
                              vec4 *color,
                              float rotx,
                              float roty,
                              float rotz)
{
    /* Declare the up axis vector to be static, because it won't change. */
    vec3 up_axis = { 0.0f, 0.0f, 1.0f };
    /* Allocate memory for a new LAMP. */
    LAMP *lamp = (LAMP *) calloc(1, sizeof(LAMP));
    /* Assign the name received in parameter to the structure, because it is
     * always nice to have an internal name for each structure.
     */
    strcpy(lamp->name, name);
    /* Assign the color to the lamp. */
    memcpy(&lamp->color, color, sizeof(vec4));
    /* Set the type of the lamp as 0 for directional. */
    lamp->type = LampDirectional;
    /* Use the following helper function (which can be found in utils.cpp)
     * to rotate the up axis by the XYZ rotation angle received as parameters.
     * I think it's a lot easier to deal with angles when it comes to direction
     * vectors.
     */
    create_direction_vector(&lamp->direction, &up_axis, rotx, roty, rotz);
    /* Return the new lamp pointer. */
    return lamp;
}

LAMP *LAMP_create_point(char *name, vec4 *color, vec3 *position)
{
    LAMP *lamp = (LAMP *) calloc(1, sizeof(LAMP));
    strcpy(lamp->name, name);
    memcpy(&lamp->color, color, sizeof(vec4));
    /* Assign the position received in parameter to the current lamp
     * pointer.  In addition, make sure thatyou specify 1 as the W
     * component of the position, because you are going to need to
     * multiply it by the modelview matrix the same way as if you were
     * dealing with a vertex position in eye space.
     */
    memcpy(&lamp->position, position, sizeof(vec3));
    lamp->position.w = 1.0f;
    /* Specify that 1 represents a basic point light that emits constant
     * omnidirectional light.
     */
    lamp->type = LampPoint;
    return lamp;
}

LAMP *LAMP_create_point_with_attenuation(char *name, vec4 *color, vec3 *position, float distance, float linear_attenuation, float quadratic_attenuation)
{
    LAMP *lamp = (LAMP *) calloc(1, sizeof(LAMP));
    strcpy(lamp->name, name);
    memcpy(&lamp->color, color, sizeof(vec4));
    memcpy(&lamp->position, position, sizeof(vec3));
    lamp->position.w = 1.0f;
    /* Store the double distance, beacuse the falloff distance parameter
     * represents the half distance where the light starts to be
     * attenuated.
     */
    lamp->distance = distance * 2.0f;
    /* Store the linear attentuation. */
    lamp->linear_attenuation = linear_attenuation;
    /* Strore the quadratic attenuation. */
    lamp->quadratic_attenuation = quadratic_attenuation;
    /* Create a new lamp type. */
    lamp->type = LampPointWithAttenuation;

    return lamp;
}

/* Basically create a point light, but with a distance parameter. */
LAMP *LAMP_create_point_sphere(char *name,
                               vec4 *color,
                               vec3 *position,
                               float distance)
{
    /* Redirect the execution pointer to create a simple point light, and
     * then adjust and tweak the other parameters to fit a new lamp type.
     */
    LAMP *lamp = LAMP_create_point(name, color, position);

    lamp->distance = distance;

    lamp->type = LampSphericalPoint;

    return lamp;
}

void LAMP_get_direction_in_eye_space(LAMP *lamp, mat4 *m, vec3 *direction)
{
    /* Multiply the current lamp direction by the view matrix received in
     * parameter to be able to calculate the lamp direction in eye space.
     */
    vec3_multiply_mat4(direction,
                       &lamp->direction,
                       m);
    /* Invert the vector, because in eye space, the direction is simply the
     * inverted vector.
     */
    vec3_invert(direction, direction);
}

/* This function is basically very easy.  In the same way that you
 * convert the position in your vertex shader, handle the conversion
 * to eye space here so you you do not have to pass the modelview
 * matrix of the camera to the shader, and offload a bit of work from
 * the CPU.
 */
void LAMP_get_position_in_eye_space(LAMP *lamp, mat4 *m, vec4 *position)
{
    /* Multiply the position by the matrix received in parameters and
     * assign the result to the position vector.
     */
    vec4_multiply_mat4(position,
                       &lamp->position,
                       m);
}

LAMP *LAMP_free(LAMP *lamp)
{
    free(lamp);
    return NULL;
}

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
            glUniform4fv(uniform.location,
                         1,
                         (float *)&objmesh->current_material->diffuse);

			uniform.constant = true;
        } else if (name == "MATERIAL.specular") {
            glUniform4fv(uniform.location,
                         1,
                         (float *)&objmesh->current_material->specular);

			uniform.constant = true;
        } else if (name == "MATERIAL.shininess") {
            glUniform1f(uniform.location,
                        objmesh->current_material->specular_exponent * 0.128f);
            
			uniform.constant = true;
        }
	}

    /* A temp string to dynamically create the LAMP property names. */
    char tmp[MAX_CHAR] = {""};
    /* Create the uniform name for the color of the lamp. */
    sprintf(tmp, "LAMP_FS.color");
    /* Get the uniform location and send over the current lamp color. */
    glUniform4fv(program->get_uniform_location(tmp),
                 1,
                 (float *)&lamp->color);

    /* Check if the lamp type is directional.  If yes, you need to send
     * over the normalized light direction vector in eye space.
     */
    if (lamp->type == LampDirectional) {
        /* Temp variable to hold the direction in eye space. */
        vec3 direction;
        /* Create the lamp direction property name. */
        sprintf(tmp, "LAMP_VS.direction");
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
        LAMP_get_direction_in_eye_space(lamp,
                                        &gfx.modelview_matrix[gfx.modelview_matrix_index - 1],
                                        &direction);

        glUniform3fv(program->get_uniform_location(tmp),
                     1,
                     (float *)&direction);
    } else if (lamp->type == LampPoint) {
        vec4 position;

        sprintf(tmp, "LAMP_VS.position");

        LAMP_get_position_in_eye_space(lamp,
                                       &gfx.modelview_matrix[gfx.modelview_matrix_index - 1],
                                       &position);

        glUniform3fv(program->get_uniform_location(tmp),
                     1,
                     (float *)&position);
    } else if (lamp->type == LampPointWithAttenuation) {
        vec4 position;

        sprintf(tmp, "LAMP_VS.position");

        LAMP_get_position_in_eye_space(lamp,
                                       &gfx.modelview_matrix[gfx.modelview_matrix_index - 1],
                                       &position);

        glUniform3fv(program->get_uniform_location(tmp),
                     1,
                     (float *)&position);

        sprintf(tmp, "LAMP_FS.distance");
        glUniform1f(program->get_uniform_location(tmp),
                    lamp->distance);

        sprintf(tmp, "LAMP_FS.linear_attenuation");
        glUniform1f(program->get_uniform_location(tmp),
                    lamp->linear_attenuation);

        sprintf(tmp, "LAMP_FS.quadratic_attenuation");
        glUniform1f(program->get_uniform_location(tmp),
                    lamp->quadratic_attenuation);
    } else if (lamp->type == LampSphericalPoint) {
        /* This is basically the same as for type #1 (basic point light), except
         * that the distance is sent over to the shader.
         */
        vec4 position;

        sprintf(tmp, "LAMP_VS.position");

        LAMP_get_position_in_eye_space(lamp,
                                       &gfx.modelview_matrix[gfx.modelview_matrix_index - 1],
                                       &position);

        glUniform3fv(program->get_uniform_location(tmp),
                     1,
                     (float *)&position);

        sprintf(tmp, "LAMP_FS.distance");
        glUniform1f(program->get_uniform_location(tmp),
                    lamp->distance);
    }
}

void templateAppInit(int width, int height)
{
	atexit(templateAppExit);

	GFX_start();

	glViewport(0.0f, 0.0f, width, height);

	glGetIntegerv(GL_VIEWPORT, viewport_matrix);

	obj = new OBJ(OBJ_FILE, true);

	for (auto objmesh=obj->objmesh.begin();
         objmesh!=obj->objmesh.end(); ++objmesh) {
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
         program!=obj->program.end(); ++program) {
		(*program)->build(program_bind_attrib_location,
                          program_draw,
                          true,
                          obj->program_path);
    }

	for (auto objmaterial=obj->objmaterial.begin();
         objmaterial!=obj->objmaterial.end(); ++objmaterial) {
		objmaterial->build(NULL);
    }
    
    vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
    
//    lamp = LAMP_create_directional((char *)"sun",    // Internal name of lamp
//                                   &color, // The lamp color.
//                                   -25.0f,  // The XYZ rotation angle in degrees
//                                   0.0f,  // that will be used to create the
//                                   -45.0f);// direction vector.
    /* The 3D position in world space of the point light. */
    vec3 position = { 3.5f, 3.0f, 6.0f };
//    /* Create a new LAMP pointer and declare it as a simple point light. */
//    lamp = LAMP_create_point((char *)"point", &color, &position);
//    /* The linear and quadratic attenuation are values that range from 0
//     * to 1, which will be directly affected by the falloff distance of
//     * the lamp.  1 means fully attenuated, and 0 represents constant (same
//     * as in the regular point light calculations in the previous section).
//     */
//    lamp = LAMP_create_point_with_attenuation((char *)"point1",
//                                              &color,
//                                              &position,
//                                              10.0f,
//                                               0.5f,
//                                               1.0f);
    lamp = LAMP_create_point_sphere((char *)"point2",
                                    &color,
                                    &position,
                                    10.0f);
}


void templateAppDraw(void)
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	GFX_set_matrix_mode(PROJECTION_MATRIX);
	GFX_load_identity();
	
	GFX_set_perspective(45.0f,
                        (float)viewport_matrix[2] / (float)viewport_matrix[3],
                          0.1f,
                        100.0f,
                        -90.0f);

	GFX_set_matrix_mode(MODELVIEW_MATRIX);
	GFX_load_identity();

	GFX_translate(14.0f, -12.0f, 7.0f);

	GFX_rotate(48.5f, 0.0f, 0.0f, 1.0f);

	GFX_rotate(72.0, 1.0f, 0.0f, 0.0f);
	
	mat4_invert(GFX_get_modelview_matrix());


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


void templateAppExit(void) {
    lamp = LAMP_free(lamp);

    delete obj;
}
