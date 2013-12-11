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

#ifndef MD5_H
#define MD5_H


enum MD5Method {
    MD5_METHOD_FRAME = 0,
    MD5_METHOD_LERP  = 1,
    MD5_METHOD_SLERP = 2
};


struct MD5JOINT
{
    char			name[ MAX_CHAR ] = "";

    int				parent;

    vec3			location;

    quaternion			rotation;

    MD5JOINT() : parent(-1), location(0,0,0), rotation(1,0,0,0) {}
};


struct MD5VERTEX {
    vec2		uv;

    vec3		normal;

    vec3		tangent;

    unsigned int	start;

    unsigned int	count;
    MD5VERTEX() :
        start(0), count(0), uv(0,0), normal(0,0,0), tangent(0,0,0) {}
    ~MD5VERTEX() {}
    MD5VERTEX(const MD5VERTEX &src) :
        uv(src.uv), normal(src.normal), tangent(src.tangent),
        start(src.start), count(src.count) {}
    MD5VERTEX &operator=(const MD5VERTEX &rhs) {
        if (this != &rhs) {
            uv      = rhs.uv;
            normal  = rhs.normal;
            tangent = rhs.tangent;
            start   = rhs.start;
            count   = rhs.count;
        }
        return *this;
    }
};


struct MD5TRIANGLE {
    // CRL -- When this code is converted to C++ 11 consider replacing
    // this
    unsigned short indice[ 3 ] = { 0, 0, 0 };
    // with
    // std::array<unsigned short, 3>   indice = { 0, 0, 0 };
    MD5TRIANGLE() {}
    ~MD5TRIANGLE() {}
    MD5TRIANGLE(const MD5TRIANGLE &src) {
        memcpy(indice, src.indice, sizeof(indice));
    }
    MD5TRIANGLE &operator=(const MD5TRIANGLE &rhs) {
        if (this != &rhs)
            memcpy(indice, rhs.indice, sizeof(indice));
        return *this;
    }
};


struct MD5WEIGHT {
    int		joint;

    float	bias;

    vec3	location;

    vec3	normal;

    vec3	tangent;

    MD5WEIGHT() : joint(0), bias(0) {
        memset(&location, 0, sizeof(location));
        memset(&normal,   0, sizeof(normal));
        memset(&tangent,  0, sizeof(tangent));
    }
    ~MD5WEIGHT() {}
    MD5WEIGHT(const MD5WEIGHT &src) :
        joint(src.joint), bias(src.bias), location(src.location),
        normal(src.normal), tangent(src.tangent) {}
    MD5WEIGHT &operator=(const MD5WEIGHT &rhs) {
        if (this != &rhs) {
            joint    = rhs.joint;
            bias     = rhs.bias;
            location = rhs.location;
            normal   = rhs.normal;
            tangent  = rhs.tangent;
        }
        return *this;
    }
};


struct MD5MESH {
    char		shader[ MAX_CHAR ] = "";

    std::vector<MD5VERTEX>  md5vertex;

    unsigned int	vbo;

    unsigned int	size;

    unsigned int	stride;

    unsigned int	offset[ 4 ] = { 0, 0, 0, 0 };

    unsigned char	*vertex_data;

    std::vector<MD5TRIANGLE>    md5triangle;

    unsigned int	mode;

    unsigned short	n_indice;

    std::vector<unsigned short> indice;

    unsigned int	vbo_indice;

    std::vector<MD5WEIGHT>  md5weight;

    unsigned int	vao;

    bool                visible;

    OBJMATERIAL		*objmaterial;
public:
    MD5MESH(const char *name=NULL);
    ~MD5MESH() {
        if (this->vertex_data)
            free(this->vertex_data);

        if (this->vbo)
            glDeleteBuffers(1, &this->vbo);

        if (this->vbo_indice)
            glDeleteBuffers(1, &this->vbo_indice);

        if (this->vao)
            glDeleteVertexArraysOES(1, &this->vao);
    }
    MD5MESH(const MD5MESH &src);
    MD5MESH &operator=(const MD5MESH &rhs) {
        if (this != &rhs) {
            strcpy(shader, rhs.shader);
            md5vertex   = rhs.md5vertex;
            vbo         = rhs.vbo;
            size        = rhs.size;
            stride      = rhs.stride;
            memcpy(offset, rhs.offset, sizeof(offset));
            if (vertex_data) free(vertex_data);
            vertex_data = (unsigned char *) calloc(1, size);
            vertex_data = rhs.vertex_data;
            md5triangle = rhs.md5triangle;
            mode        = rhs.mode;
            n_indice    = rhs.n_indice;
            indice      = rhs.indice;
            vbo_indice  = rhs.vbo_indice;
            md5weight   = rhs.md5weight;
            vao         = rhs.vao;
            visible     = rhs.visible;
            objmaterial = rhs.objmaterial;
        }
        return *this;
    }
    void set_mesh_attributes();
    void set_mesh_visibility(const bool visible);
    void set_mesh_material(OBJMATERIAL *objmaterial);
    void build_vbo();
    void build_vao();
};

struct MD5ACTION {
    char			name[ MAX_CHAR ] = "";

    std::vector<std::vector<MD5JOINT> >   frame;

    std::vector<MD5JOINT>   pose;

    int			curr_frame;

    int			next_frame;

    AnimState           state;

    MD5Method           method;

    bool                loop;

    float		frame_time;

    float		fps;
public:
    MD5ACTION(const char *name=NULL);
    void action_play(const MD5Method frame_interpolation_method,
                     const bool loop);
    void action_pause();
    void action_stop();
    void set_action_fps(float fps);
};

struct MD5 {
    char		name[ MAX_CHAR ] = "";

    bool                visible;

    std::vector<MD5JOINT>    bind_pose;

    std::vector<MD5MESH>     md5mesh;

    std::vector<MD5ACTION>   md5action;

    vec3		location;

    vec3		rotation;

    vec3		scale;

    vec3		min;

    vec3		max;

    vec3		dimension;

    float		radius;

    float		distance;

    btRigidBody		*btrigidbody;
protected:
    void update_bound_mesh();
public:
    MD5(char *filename, const bool relative_path);
    ~MD5() {}
    int load_action(char *name, char *filename, const bool relative_path);
    void free_mesh_data();
    MD5ACTION *get_action(char *name, const bool exact_name);
    MD5MESH *get_mesh(char *name, const bool exact_name);
    void optimize(unsigned int vertex_cache_size);
    void build_bind_pose_weighted_normals_tangents();
    void set_pose(std::vector<MD5JOINT> &pose);
    void blend_pose(std::vector<MD5JOINT> &final_pose,
                    const std::vector<MD5JOINT> &pose0,
                    const std::vector<MD5JOINT> &pose1,
                    const MD5Method joint_interpolation_method,
                    const float blend);
    void add_pose(std::vector<MD5JOINT> &final_pose,
                  const MD5ACTION &action0,
                  const MD5ACTION &action1,
                  const MD5Method joint_interpolation_method,
                  const float action_weight);
    void build();
    void build2();
    bool draw_action(float time_step);
    void draw();
};

#endif
