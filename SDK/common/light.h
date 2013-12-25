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


#ifndef LIGHT_H
#define LIGHT_H

enum
{
    LIGHT_DIRECTIONAL		 = 0,
    LIGHT_POINT			 = 1,
    LIGHT_POINT_WITH_ATTENUATION = 2,
    LIGHT_POINT_SPHERE		 = 3,
    LIGHT_SPOT			 = 4
};

struct LIGHT {
    char    name[MAX_CHAR];
    vec4    color;
    unsigned char type;
    LIGHT(const char *n, const vec4 &c, const unsigned char t=~0) : color(c), type(t) {
        memset(name, 0, sizeof(name));
        strcpy(name, n);
    }
    virtual ~LIGHT() {}
    LIGHT(const LIGHT &src) : color(src.color), type(src.type) {
        memset(name, 0, sizeof(name));
        strcpy(name, src.name);
    }
    LIGHT &operator=(const LIGHT &rhs) {
        if (this != &rhs) {
            memset(name, 0, sizeof(name));
            strcpy(name, rhs.name);
            color = rhs.color;
            type  = rhs.type;
        }
        return *this;
    }
    virtual void push_to_shader(PROGRAM *program) {
        /* A temp string to dynamically create the LIGHT property names. */
        char tmp[MAX_CHAR] = {""};
        /* Create the uniform name for the color of the lamp. */
        sprintf(tmp, "LIGHT_FS.color");
        /* Get the uniform location and send over the current lamp color. */
        glUniform4fv(program->get_uniform_location(tmp),
                     1,
                     this->color.v());
    }
};

struct DirectionalLight : LIGHT {
    vec3    direction;
public:
    DirectionalLight(const char *name,
                     const vec4 &color,
                     const float rotx,
                     const float roty,
                     const float rotz);
    ~DirectionalLight() {}
    DirectionalLight(const DirectionalLight &src) :
        direction(src.direction), LIGHT(src) {
    }
    DirectionalLight &operator=(const DirectionalLight &rhs) {
        if (this != &rhs) {
            LIGHT::operator=(rhs);
            direction = rhs.direction;
        }
        return *this;
    }
    void get_direction_in_eye_space(mat4 *m, vec3 *direction);
    vec3 get_direction_in_eye_space(mat4 *m);
    void push_to_shader(PROGRAM *program);
};

struct PointLight : LIGHT {
    vec4    position;
protected:
    PointLight(const char *name, const vec4 &color, const vec3 &position, const unsigned char t);
public:
    PointLight(const char *name, const vec4 &color, const vec3 &position);
    ~PointLight() {}
    PointLight(const PointLight &src) :
        position(src.position), LIGHT(src) {
    }
    PointLight &operator=(const PointLight &rhs) {
        if (this != &rhs) {
            LIGHT::operator=(rhs);
            position = rhs.position;
        }
        return *this;
    }
    void get_position_in_eye_space(mat4 *m, vec4 *position);
    void push_to_shader(PROGRAM *program);
};

struct AttenuatedPointLight : PointLight {
    float   linear_attenuation;
    float   quadratic_attenuation;
    float   distance;
public:
    AttenuatedPointLight(const char *name, const vec4 &color,
                         const vec3 &position, const float distance,
                         const float linear_attenuation,
                         const float quadratic_attenuation);
    ~AttenuatedPointLight() {}
    AttenuatedPointLight(const AttenuatedPointLight &src) :
        linear_attenuation(src.linear_attenuation),
        quadratic_attenuation(src.quadratic_attenuation),
        distance(src.distance), PointLight(src) {
    }
    AttenuatedPointLight &operator=(const AttenuatedPointLight &rhs) {
        if (this != &rhs) {
            PointLight::operator=(rhs);
            linear_attenuation    = rhs.linear_attenuation;
            quadratic_attenuation = rhs.quadratic_attenuation;
            distance              = rhs.distance;
        }
        return *this;
    }
    void push_to_shader(PROGRAM *program);
};

struct PointSphere : PointLight {
    float   distance;
public:
    PointSphere(const char *name,
                const vec4 &color,
                const vec3 &position,
                const float distance);
    ~PointSphere() {}
    PointSphere(const PointSphere &src) :
        distance(src.distance), PointLight(src) {
    }
    PointSphere &operator=(const PointSphere &rhs) {
        if (this != &rhs) {
            PointLight::operator=(rhs);
            distance = rhs.distance;
        }
        return *this;
    }
    void push_to_shader(PROGRAM *program);
};

struct SpotLight : PointLight {
    float   spot_fov;
    float   spot_cos_cutoff;
    float   spot_blend;
    vec3    spot_direction;
public:
    SpotLight(const char *name,
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
    ~SpotLight() {}
    SpotLight(const SpotLight &src) :
        spot_fov(src.spot_fov), spot_cos_cutoff(src.spot_cos_cutoff),
        spot_blend(src.spot_blend),
        spot_direction(src.spot_direction), PointLight(src) {
    }
    SpotLight &operator=(const SpotLight &rhs) {
        if (this != &rhs) {
            PointLight::operator=(rhs);
            spot_fov        = rhs.spot_fov;
            spot_cos_cutoff = rhs.spot_cos_cutoff;
            spot_blend      = rhs.spot_blend;
            spot_direction  = rhs.spot_direction;
        }
        return *this;
    }
    void push_to_shader(PROGRAM *program);
    void get_direction_in_object_space(mat4 *m, vec3 *direction);
};

#endif
