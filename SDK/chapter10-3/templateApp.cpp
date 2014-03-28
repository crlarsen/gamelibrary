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

enum LampType {
    LampDirectional          = 0,
    LampPoint                = 1,
    LampPointWithAttenuation = 2,
    LampSphericalPoint       = 3,
    LampSpot                 = 4
};

struct LAMP {
    char    name[MAX_CHAR];
    vec4    color;
    unsigned char type;
    LAMP(const char *n, const vec4 &c, const unsigned char t=~0) : color(c), type(t) {
        assert(n==NULL || strlen(n)<sizeof(this->name));
        strcpy(this->name, n ? n : "");
    }
    virtual ~LAMP() {}
    LAMP(const LAMP &src) : color(src.color), type(src.type) {
        strcpy(name, src.name);
    }
    LAMP &operator=(const LAMP &rhs) {
        if (this != &rhs) {
            strcpy(name, rhs.name);
            color = rhs.color;
            type  = rhs.type;
        }
        return *this;
    }
    virtual void push_to_shader(PROGRAM *program) {
        /* A temp string to dynamically create the LAMP property names. */
        char tmp[MAX_CHAR] = {""};
        /* Create the uniform name for the color of the lamp. */
        sprintf(tmp, "LAMP_FS.color");
        /* Get the uniform location and send over the current lamp color. */
        glUniform4fv(program->get_uniform_location(tmp),
                     1,
                     this->color.v());
    }
};

LAMP *lamp = NULL;

struct DirectionalLamp : LAMP {
    vec3    direction;
public:
    DirectionalLamp(const char *name,
                    const vec4 &color,
                    const float rotx,
                    const float roty,
                    const float rotz);
    ~DirectionalLamp() {}
    DirectionalLamp(const DirectionalLamp &src) :
        direction(src.direction), LAMP(src) {
    }
    DirectionalLamp &operator=(const DirectionalLamp &rhs) {
        if (this != &rhs) {
            LAMP::operator=(rhs);
            direction = rhs.direction;
        }
        return *this;
    }
    vec3 get_direction_in_eye_space(const mat4 &m);
    void push_to_shader(PROGRAM *program) {
        this->LAMP::push_to_shader(program);

        /* A temp string to dynamically create the LAMP property names. */
        char tmp[MAX_CHAR] = {""};
        /* Temp variable to hold the direction in eye space. */
        vec3 direction_es;
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
        direction_es =
            get_direction_in_eye_space(GFX_get_modelview_matrix(-1));

        glUniform3fv(program->get_uniform_location(tmp),
                     1,
                     direction_es.v());
    }
};

DirectionalLamp::DirectionalLamp(const char *name,
                                 const vec4 &color,
                                 const float rotx,
                                 const float roty,
                                 const float rotz) : LAMP(name, color, LampDirectional)
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

vec3 DirectionalLamp::get_direction_in_eye_space(const mat4 &m)
{
    /* Multiply the current lamp direction by the view matrix received in
     * parameter to be able to calculate the lamp direction in eye space.
     * Negate the vector, because in eye space, the direction is simply
     * the negated vector.
     */
    return -vec3(vec4(direction, 0.0f) * m);
}

struct PointLamp : LAMP {
    vec4    position;
protected:
    PointLamp(const char *name, const vec4 &color, const vec3 &position, const unsigned char t);
public:
    PointLamp(const char *name, const vec4 &color, const vec3 &position);
    ~PointLamp() {}
    PointLamp(const PointLamp &src) :
        position(src.position), LAMP(src) {
    }
    PointLamp &operator=(const PointLamp &rhs) {
        if (this != &rhs) {
            LAMP::operator=(rhs);
            position = rhs.position;
        }
        return *this;
    }
    vec4 get_position_in_eye_space(const mat4 &m);
    void push_to_shader(PROGRAM *program) {
        this->LAMP::push_to_shader(program);

        char tmp[MAX_CHAR] = {""};

        vec4 position_es;

        sprintf(tmp, "LAMP_VS.position");

        position_es =
            get_position_in_eye_space(GFX_get_modelview_matrix(-1));

        glUniform3fv(program->get_uniform_location(tmp),
                     1,
                     position_es.v());
    }
};

PointLamp::PointLamp(const char *name, const vec4 &color, const vec3 &position) : LAMP(name, color, LampPoint)
{
    /* Assign the position received in parameter to the current lamp
     * pointer.  In addition, make sure that you specify 1 as the W
     * component of the position, because you are going to need to
     * multiply it by the modelview matrix the same way as if you were
     * dealing with a vertex position in eye space.
     */
    this->position = vec4(position, 1.0f);
}

PointLamp::PointLamp(const char *name, const vec4 &color, const vec3 &position, const unsigned char t) : LAMP(name, color, t)
{
    /* Assign the position received in parameter to the current lamp
     * pointer.  In addition, make sure that you specify 1 as the W
     * component of the position, because you are going to need to
     * multiply it by the modelview matrix the same way as if you were
     * dealing with a vertex position in eye space.
     */
    this->position = vec4(position, 1.0f);
}

/* This function is basically very easy.  In the same way that you
 * convert the position in your vertex shader, handle the conversion
 * to eye space here so you you do not have to pass the modelview
 * matrix of the camera to the shader, and offload a bit of work from
 * the CPU.
 */
vec4 PointLamp::get_position_in_eye_space(const mat4 &m)
{
    /* Multiply the position by the matrix received in parameters and
     * assign the result to the position vector.
     */
    return position * m;
}

struct AttenuatedPointLamp : PointLamp {
    float   linear_attenuation;
    float   quadratic_attenuation;
    float   distance;
public:
    AttenuatedPointLamp(const char *name, const vec4 &color,
                        const vec3 &position, const float distance,
                        const float linear_attenuation,
                        const float quadratic_attenuation);
    ~AttenuatedPointLamp() {}
    AttenuatedPointLamp(const AttenuatedPointLamp &src) :
        linear_attenuation(src.linear_attenuation),
        quadratic_attenuation(src.quadratic_attenuation),
        distance(src.distance), PointLamp(src) {
    }
    AttenuatedPointLamp &operator=(const AttenuatedPointLamp &rhs) {
        if (this != &rhs) {
            PointLamp::operator=(rhs);
            linear_attenuation    = rhs.linear_attenuation;
            quadratic_attenuation = rhs.quadratic_attenuation;
            distance              = rhs.distance;
        }
        return *this;
    }
    void push_to_shader(PROGRAM *program) {
        this->PointLamp::push_to_shader(program);

        char tmp[MAX_CHAR] = {""};

        sprintf(tmp, "LAMP_FS.distance");
        glUniform1f(program->get_uniform_location(tmp),
                    this->distance);

        sprintf(tmp, "LAMP_FS.linear_attenuation");
        glUniform1f(program->get_uniform_location(tmp),
                    this->linear_attenuation);

        sprintf(tmp, "LAMP_FS.quadratic_attenuation");
        glUniform1f(program->get_uniform_location(tmp),
                    this->quadratic_attenuation);

    }
};

AttenuatedPointLamp::AttenuatedPointLamp(const char *name, const vec4 &color,
                                         const vec3 &position, const float d,
                                         const float la,
                                         const float qa) : distance(d*2.0),
                                         linear_attenuation(la),
                                         quadratic_attenuation(qa),
                                         PointLamp(name, color, position, LampPointWithAttenuation)
{
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
                               GFX_get_modelview_projection_matrix().m());
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

    lamp->push_to_shader(program);
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
    
    vec4 color(1.0f, 1.0f, 1.0f, 1.0f);
    
//    lamp = LAMP_create_directional((char *)"sun",    // Internal name of lamp
//                                   &color, // The lamp color.
//                                   -25.0f,  // The XYZ rotation angle in degrees
//                                   0.0f,  // that will be used to create the
//                                   -45.0f);// direction vector.
    /* The 3D position in world space of the point light. */
    vec3 position(3.5f, 3.0f, 6.0f);
    /* Create a new LAMP pointer and declare it as a simple point light. */
//    lamp = new PointLamp((char *)"point", color, position);
    /* The linear and quadratic attenuation are values that range from 0
     * to 1, which will be directly affected by the falloff distance of
     * the lamp.  1 means fully attenuated, and 0 represents constant (same
     * as in the regular point light calculations in the previous section).
     */
    lamp = new AttenuatedPointLamp((char *)"point1",
                                   color,
                                   position,
                                   10.0f,
                                    0.5f,
                                    1.0f);
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

    const float   alpha(-72.0f*DEG_TO_RAD_DIV_2);
    const float   cosAlpha(cosf(alpha)), sinAlpha(sinf(alpha));
    const float   beta(-48.5f*DEG_TO_RAD_DIV_2);
    const float   cosBeta(cosf(beta)), sinBeta(sinf(beta));
    GFX_rotate(quaternion( cosAlpha*cosBeta, sinAlpha*cosBeta,
                          -sinAlpha*sinBeta, cosAlpha*sinBeta));

    GFX_translate(-14.0f, 12.0f, -7.0f);


    for (objmesh=obj->objmesh.begin();
         objmesh!=obj->objmesh.end(); ++objmesh) {

        GFX_push_matrix();

        GFX_translate(objmesh->location);

        objmesh->draw();
        
        GFX_pop_matrix();
    }
}


void templateAppExit(void) {
    delete lamp;
    lamp = NULL;

    delete obj;
}
