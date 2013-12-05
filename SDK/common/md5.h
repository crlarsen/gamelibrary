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


enum
{
    MD5_METHOD_FRAME = 0,
    MD5_METHOD_LERP  = 1,
    MD5_METHOD_SLERP = 2
};


typedef struct
{
    char			name[ MAX_CHAR ];

    int				parent;

    vec3			location;

    vec4			rotation;

} MD5JOINT;


typedef struct
{
    vec2			uv;

    vec3			normal;

    vec3			tangent;

    unsigned int	start;

    unsigned int	count;

} MD5VERTEX;


typedef struct
{
    unsigned short indice[ 3 ];
    
} MD5TRIANGLE;


typedef struct
{
    int		joint;

    float	bias;

    vec3	location;

    vec3	normal;

    vec3	tangent;
    
} MD5WEIGHT;


struct MD5MESH {
    char		shader[ MAX_CHAR ];

    std::vector<MD5VERTEX>  md5vertex;

    unsigned int	vbo;

    unsigned int	size;

    unsigned int	stride;

    unsigned int	offset[ 4 ];

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
    void set_mesh_attributes();
    void set_mesh_visibility(const bool visible);
    void set_mesh_material(OBJMATERIAL *objmaterial);
    void build_vbo();
};

struct MD5ACTION {
    char			name[ MAX_CHAR ];

    std::vector<MD5JOINT*>   frame;

    MD5JOINT		*pose;

    int			curr_frame;

    int			next_frame;

    unsigned char	state;

    unsigned char	method;

    bool                loop;

    float		frame_time;

    float		fps;
public:
    void action_play(const unsigned char frame_interpolation_method,
                     const bool loop);
    void action_pause();
    void action_stop();
    void set_action_fps(float fps);
};

struct MD5 {
    char		name[ MAX_CHAR ];

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
    ~MD5();
    int load_action(char *name, char *filename, const bool relative_path);
    void free_mesh_data();
    MD5ACTION *get_action(char *name, const bool exact_name);
    MD5MESH *get_mesh(char *name, const bool exact_name);
    void optimize(unsigned int vertex_cache_size);
    void build_vbo(unsigned int mesh_index);    // CRL
    void build_bind_pose_weighted_normals_tangents();
    void set_pose(MD5JOINT *pose);
    void blend_pose(MD5JOINT *final_pose, MD5JOINT *pose0, MD5JOINT *pose1,
                    unsigned char joint_interpolation_method, float blend);
    void add_pose(MD5JOINT *final_pose, MD5ACTION *action0,
                  MD5ACTION *action1,
                  unsigned char joint_interpolation_method,
                  float action_weight);
    void build();
    void build2();
    bool draw_action(float time_step);
    void draw();
};

#endif
