/*

GFX Lightweight OpenGLES 2.0 Game and Graphics Engine

Copyright (C) 2011 Romain Marucchi-Foino http://gfx.sio2interactive.com

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of
this software. Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it freely,
subject to the following restrictions:

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


#include "gfx.h"

DirectionalLight::DirectionalLight(const char *name,
                                   const vec4 &color,
                                   const float rotx,
                                   const float roty,
                                   const float rotz) : LIGHT(name, color, /* type = */ LIGHT_DIRECTIONAL)
{
    /* Declare the up axis vector to be static, because it won't change. */
    vec3 up_axis(0.0f, 0.0f, 1.0f);
    /* Use the following helper function (which can be found in utils.cpp)
     * to rotate the up axis by the XYZ rotation angle received as parameters.
     * I think it's a lot easier to deal with angles when it comes to direction
     * vectors.
     */
    create_direction_vector(&this->direction, &up_axis, rotx, roty, rotz);
}

void DirectionalLight::push_to_shader(PROGRAM *program) {
    this->LIGHT::push_to_shader(program);

    /* A temp string to dynamically create the LIGHT property names. */
    char tmp[MAX_CHAR] = {""};
    /* Temp variable to hold the direction in eye space. */
    vec3 direction;
    /* Create the lamp direction property name. */
    sprintf(tmp, "LIGHT_VS.direction");
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
    this->get_direction_in_eye_space(&gfx.modelview_matrix[gfx.modelview_matrix_index - 1],
                                     &direction);

    glUniform3fv(program->get_uniform_location(tmp),
                 1,
                 (float *)&direction);
}

PointLight::PointLight(const char *name, const vec4 &color, const vec3 &position) : LIGHT(name, color, LIGHT_POINT)
{
    /* Assign the position received in parameter to the current lamp
     * pointer.  In addition, make sure that you specify 1 as the W
     * component of the position, because you are going to need to
     * multiply it by the modelview matrix the same way as if you were
     * dealing with a vertex position in eye space.
     */
    this->position = vec4(position, 1.0f);
}

PointLight::PointLight(const char *name, const vec4 &color, const vec3 &position, const unsigned char t) : LIGHT(name, color, t)
{
    /* Assign the position received in parameter to the current lamp
     * pointer.  In addition, make sure that you specify 1 as the W
     * component of the position, because you are going to need to
     * multiply it by the modelview matrix the same way as if you were
     * dealing with a vertex position in eye space.
     */
    this->position = vec4(position, 1.0f);
}

void PointLight::push_to_shader(PROGRAM *program) {
    this->LIGHT::push_to_shader(program);

    char tmp[MAX_CHAR] = {""};

    vec4 position;

    sprintf(tmp, "LIGHT_VS.position");

    this->get_position_in_eye_space(&gfx.modelview_matrix[gfx.modelview_matrix_index - 1],
                                    &position);

    glUniform3fv(program->get_uniform_location(tmp),
                 1,
                 (float *)&position);
}

AttenuatedPointLight::AttenuatedPointLight(const char *name, const vec4 &color,
                                         const vec3 &position, const float d,
                                         const float la,
                                         const float qa) : distance(d*2.0),
                                         linear_attenuation(la),
                                         quadratic_attenuation(qa),
                                         PointLight(name, color, position, LIGHT_POINT_WITH_ATTENUATION)
{
}

void AttenuatedPointLight::push_to_shader(PROGRAM *program) {
    this->LIGHT::push_to_shader(program);

    char tmp[MAX_CHAR] = {""};

    vec4 position;

    sprintf(tmp, "LIGHT_VS.position");

    this->get_position_in_eye_space(&gfx.modelview_matrix[gfx.modelview_matrix_index - 1],
                                    &position);

    glUniform3fv(program->get_uniform_location(tmp),
                 1,
                 (float *)&position);

    sprintf(tmp, "LIGHT_FS.distance");
    glUniform1f(program->get_uniform_location(tmp),
                this->distance);

    sprintf(tmp, "LIGHT_FS.linear_attenuation");
    glUniform1f(program->get_uniform_location(tmp),
                this->linear_attenuation);

    sprintf(tmp, "LIGHT_FS.quadratic_attenuation");
    glUniform1f(program->get_uniform_location(tmp),
                this->quadratic_attenuation);

}

PointSphere::PointSphere(const char *name,
                         const vec4 &color,
                         const vec3 &position,
                         const float distance) : distance(distance),
                         PointLight(name, color, position, LIGHT_POINT_SPHERE)
{
}

void PointSphere::push_to_shader(PROGRAM *program) {
    this->PointLight::push_to_shader(program);

    char tmp[MAX_CHAR] = {""};

    sprintf(tmp, "LIGHT_FS.distance");
    glUniform1f(program->get_uniform_location(tmp), distance);
}

SpotLight::SpotLight(const char *name,
                     const vec4 &color,
                     const vec3 &position,
                     /* The XYZ rotation angle of the spot direction
                      * vector in degrees.
                      */
                     const float rotx,
                     const float roty,
                     const float rotz,
                     /* The field of view of the spot, also in degrees. */
                     const float fov,
                     /* The spot blend to smooth the edge of the spot.
                      * This value is between the range of 0 and 1, where
                      * 0 represents no smoothing.
                      */
                     const float spot_blend) : PointLight(name, color, position, LIGHT_SPOT) {
    static vec3 up_axis(0.0f, 0.0f, 1.0f);
    this->spot_fov = fov;
    /* Calculate the spot cosine cut off. */
    this->spot_cos_cutoff = cosf((fov * 0.5f) * DEG_TO_RAD);
    /* Clamp the spot blend to make sure that there won't be a division by 0
     * inside the shader program.
     */
    this->spot_blend = CLAMP(spot_blend, 0.001, 1.0f);
    /* Create the direction vector for the spot based on the XYZ rotation
     * angle that the function receives.
     */
    create_direction_vector(&this->spot_direction,
                            &up_axis,
                            rotx,
                            roty,
                            rotz);
}

void SpotLight::push_to_shader(PROGRAM *program) {
    this->PointLight::push_to_shader(program);

    char tmp[MAX_CHAR] = {""};

    /* Calculating the direction of a spot is slightly different than
     * for directional lamp, because the cone has to be projected in
     * the same space as the object that might receive the light.
     */
    vec3 direction;

    sprintf(tmp, "LIGHT_VS.spot_direction");
    this->get_direction_in_object_space(&gfx.modelview_matrix[gfx.modelview_matrix_index - 1],
                                        &direction);

    glUniform3fv(program->get_uniform_location(tmp),
                 1,
                 (float *)&direction);
    /* Send the spot cos cutoff to let the shader determine if a
     * specific fragment is inside or outside the cone of light.
     */
    sprintf(tmp, "LIGHT_FS.spot_cos_cutoff");
    glUniform1f(program->get_uniform_location(tmp), this->spot_cos_cutoff);

    sprintf(tmp, "LIGHT_FS.spot_blend");
    glUniform1f(program->get_uniform_location(tmp), this->spot_blend);
}


void DirectionalLight::get_direction_in_eye_space(mat4 *m, vec3 *direction)
{
    /* Multiply the current lamp direction by the view matrix received in
     * parameter to be able to calculate the lamp direction in eye space.
     */
    vec3_multiply_mat4(direction,
                       &this->direction,
                       m);
    /* Invert the vector, because in eye space, the direction is simply the
     * inverted vector.
     */
    *direction = -*direction;
}

vec3 DirectionalLight::get_direction_in_eye_space(mat4 *m)
{
    vec3    direction;
    /* Multiply the current lamp direction by the view matrix received in
     * parameter to be able to calculate the lamp direction in eye space.
     */
    vec3_multiply_mat4(&direction,
                       &this->direction,
                       m);
    /* Invert the vector, because in eye space, the direction is simply the
     * inverted vector.
     */
    direction = -direction;

    return direction;
}


/* This function is basically very easy.  In the same way that you
 * convert the position in your vertex shader, handle the conversion
 * to eye space here so you you do not have to pass the modelview
 * matrix of the camera to the shader, and offload a bit of work from
 * the CPU.
 */
void PointLight::get_position_in_eye_space(mat4 *m, vec4 *position)
{
    /* Multiply the position by the matrix received in parameters and
     * assign the result to the position vector.
     */
    vec4_multiply_mat4(position,
                       &this->position,
                       m);
}

void SpotLight::get_direction_in_object_space(mat4 *m, vec3 *direction)
{
    mat4 invert;

    mat4_copy_mat4(&invert, m);

    mat4_invert(&invert);

    vec3_multiply_mat4(direction,
                       &this->spot_direction,
                       m);

    direction->safeNormalize();

    *direction = -*direction;
}
