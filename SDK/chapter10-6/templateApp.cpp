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
    virtual void push_to_shader(GFX *gfx, PROGRAM *program, const int i) {
        /* A temp string to dynamically create the LAMP property names. */
        char tmp[MAX_CHAR] = {""};
        /* Create the uniform name for the color of the lamp. */
        sprintf(tmp, "LAMP_FS[%d].color", i);
        /* Get the uniform location and send over the current lamp color. */
        glUniform4fv(program->get_uniform_location(tmp),
                     1,
                     this->color.v());
    }
};

/* Define the maximum amount of lamps your implementation can handle
 * simultaneously.  For this example, you're defining 2.
 */
#define MAX_LAMP    2
/* Declare an array of lamp pointers. */
LAMP *lamp[MAX_LAMP];

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
    void push_to_shader(GFX *gfx, PROGRAM *program, const int i) {
        this->LAMP::push_to_shader(gfx, program, i);

        /* A temp string to dynamically create the LAMP property names. */
        char tmp[MAX_CHAR] = {""};
        /* Temp variable to hold the direction in eye space. */
        vec3 direction_es;
        /* Create the lamp direction property name. */
        sprintf(tmp, "LAMP_VS[%d].direction", i);
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
            get_direction_in_eye_space(gfx->get_modelview_matrix(-1));

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
    void push_to_shader(GFX *gfx, PROGRAM *program, const int i) {
        this->LAMP::push_to_shader(gfx, program, i);

        char tmp[MAX_CHAR] = {""};

        vec4 position_es;

        sprintf(tmp, "LAMP_VS[%d].position", i);

        position_es =
            get_position_in_eye_space(gfx->get_modelview_matrix(-1));

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
    void push_to_shader(GFX *gfx, PROGRAM *program, const int i) {
        this->PointLamp::push_to_shader(gfx, program, i);

        char tmp[MAX_CHAR] = {""};

        sprintf(tmp, "LAMP_FS[%d].distance", i);
        glUniform1f(program->get_uniform_location(tmp),
                    this->distance);

        sprintf(tmp, "LAMP_FS[%d].linear_attenuation", i);
        glUniform1f(program->get_uniform_location(tmp),
                    this->linear_attenuation);

        sprintf(tmp, "LAMP_FS[%d].quadratic_attenuation", i);
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

/* Basically create a point light, but with a distance parameter. */
struct PointSphereLamp : PointLamp {
    float   distance;
public:
    PointSphereLamp(const char *name,
                    const vec4 &color,
                    const vec3 &position,
                    const float distance);
    ~PointSphereLamp() {}
    PointSphereLamp(const PointSphereLamp &src) :
        distance(src.distance), PointLamp(src) {
    }
    PointSphereLamp &operator=(const PointSphereLamp &rhs) {
        if (this != &rhs) {
            PointLamp::operator=(rhs);
            distance = rhs.distance;
        }
        return *this;
    }
    void push_to_shader(GFX *gfx, PROGRAM *program, const int i) {
        this->PointLamp::push_to_shader(gfx, program, i);

        char tmp[MAX_CHAR] = {""};

        sprintf(tmp, "LAMP_FS[%d].distance", i);
        glUniform1f(program->get_uniform_location(tmp), distance);
    }
};

PointSphereLamp::PointSphereLamp(const char *name,
                                 const vec4 &color,
                                 const vec3 &position,
                                 const float distance) : distance(distance),
                                 PointLamp(name, color, position, LampSphericalPoint)
{
}

struct SpotLamp : PointLamp {
    float   spot_cos_cutoff;
    float   spot_blend;
    vec3    spot_direction;
public:
    SpotLamp(const char *name,
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
             const float spot_blend);
    ~SpotLamp() {}
    SpotLamp(const SpotLamp &src) :
        spot_cos_cutoff(src.spot_cos_cutoff),
        spot_blend(src.spot_blend),
        spot_direction(src.spot_direction),
        PointLamp(src) {
    }
    SpotLamp &operator=(const SpotLamp &rhs) {
        if (this != &rhs) {
            PointLamp::operator=(rhs);
            spot_cos_cutoff = rhs.spot_cos_cutoff;
            spot_blend      = rhs.spot_blend;
            spot_direction  = rhs.spot_direction;
        }
        return *this;
    }
    void push_to_shader(GFX *gfx, PROGRAM *program, const int i) {
        this->PointLamp::push_to_shader(gfx, program, i);

        char tmp[MAX_CHAR] = {""};

        /* Calculating the direction of a spot is slightly different than
         * for directional lamp, because the cone has to be projected in
         * the same space as the object that might receive the light.
         */
        vec3 direction_os;

        sprintf(tmp, "LAMP_VS[%d].spot_direction", i);
        direction_os =
            get_direction_in_object_space(gfx->get_modelview_matrix(-1));

        glUniform3fv(program->get_uniform_location(tmp),
                     1,
                     direction_os.v());
        /* Send the spot cos cutoff to let the shader determine if a
         * specific fragment is inside or outside the cone of light.
         */
        sprintf(tmp, "LAMP_FS[%d].spot_cos_cutoff", i);
        glUniform1f(program->get_uniform_location(tmp), this->spot_cos_cutoff);

        sprintf(tmp, "LAMP_FS[%d].spot_blend", i);
        glUniform1f(program->get_uniform_location(tmp), this->spot_blend);
    }
    vec3 get_direction_in_object_space(const mat4 &m);
};

SpotLamp::SpotLamp(const char *name,
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
                   const float spot_blend) : PointLamp(name, color, position, LampSpot) {
    static vec3 up_axis(0.0f, 0.0f, 1.0f);
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

vec3 SpotLamp::get_direction_in_object_space(const mat4 &m)
{
    return -vec3(vec4(spot_direction, 0.0f) * m).normalize();
}

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

    /* Since your lamps are now in an array, simply loop and dynamically
     * create the uniform name for the lamp index in the shader program,
     * and gather the necessary data for a specific lamp index as long as
     * the loop is rolling.
     */
    for (int i=0; i != MAX_LAMP; ++i)
        lamp[i]->push_to_shader(gfx, program, i);
}


void templateAppInit(int width, int height)
{
    atexit(templateAppExit);

    gfx = new GFX;

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
    
//    lamp = new DirectionalLamp((char *)"sun",   // Internal name of lamp
//                               color,   // The lamp color.
//                               -25.0f,  // The XYZ rotation angle in degrees
//                                 0.0f,  // that will be used to create the
//                               -45.0f);// direction vector.
    /* The 3D position in world space of the point light. */
    vec3 position(3.5f, 3.0f, 6.0f);
//    /* Create a new LAMP pointer and declare it as a simple point light. */
//    lamp = new PointLamp((char *)"point", color, position);
//    /* The linear and quadratic attenuation are values that range from 0
//     * to 1, which will be directly affected by the falloff distance of
//     * the lamp.  1 means fully attenuated, and 0 represents constant (same
//     * as in the regular point light calculations in the previous section).
//     */
//    lamp = new AttenuatedPointLamp((char *)"point1",
//                                   color,
//                                   position,
//                                   10.0f,
//                                    0.5f,
//                                    1.0f);
//    lamp = new PointSphereLamp((char *)"point2",
//                               color,
//                               position,
//                               10.0f);
//    lamp = new SpotLamp((char *)"spot",
//                        color,
//                        position,
//                        /* The spot XYZ rotation angles in degrees. */
//                        -25.0f, 0.0f, -45.0f,
//                        /* The field of view in degrees. */
//                        75.0f,
//                        /* The spot blend. */
//                        0.05f);

    /* Create the first lamp, basically the same as you did before, except
     * you are initializing it at index 0 of the lamp point array.
     */
    lamp[0] = new PointSphereLamp((char *)"point1", color, position, 10.0f);

    /* Invert the XY position. */
    position->x = -position->x;
    position->y = -position->y;

    /* Modify the color to be red. */
    color->y =
    color->z = 0.0f;

    /* Create the second lamp. */
    lamp[1] = new PointSphereLamp((char *)"point2", color, position, 10.0f);
}

void templateAppDraw(void)
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
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
        float   h = (iPhoneOriginalHeight*0.5f) / tanf(originalFovy*0.5f*DEG_TO_RAD)
        ;
        fovy = 2.0f * atan2f(((float)viewport_matrix[3])*0.5, h) * RAD_TO_DEG;
    }
    gfx->set_perspective(fovy,
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


    for (objmesh=obj->objmesh.begin();
         objmesh!=obj->objmesh.end(); ++objmesh) {

        gfx->push_matrix();

        gfx->translate(objmesh->location);

        objmesh->draw();
        
        gfx->pop_matrix();
    }
}


void templateAppExit(void) {
    for (int i=0; i!=MAX_LAMP; ++i) {
        delete lamp[i];
        lamp[i] = NULL;
    }

    delete obj;
}
