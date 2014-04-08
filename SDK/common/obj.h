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

#ifndef OBJ_H
#define OBJ_H

typedef void( MATERIALDRAWCALLBACK( void * ) );

struct OBJ;

// Definitions for texture maps:
enum TextureMap {
    TM_map_Ka       = 0,
    TM_map_Kd       = 1,
    TM_map_Ks       = 2,
    TM_map_Disp     = 3,
    TM_map_Bump     = 4,
    TM_map_Tr       = 5,
    TM_Unused       = 7,

    // In case the programmer prefers the other naming convention:
    TM_Ambient      = TM_map_Ka,
    TM_Diffuse      = TM_map_Kd,
    TM_Specular     = TM_map_Ks,
    TM_Displacement = TM_map_Disp,
    TM_Bump         = TM_map_Bump,
    TM_Transparency = TM_map_Tr,
    };

#define TM_Ambient_String       ((char *)"AMBIENT")
#define TM_Diffuse_String       ((char *)"DIFFUSE")
#define TM_Specular_String      ((char *)"SPECULAR")
#define TM_Displacement_String  ((char *)"DISPLACEMENT")
#define TM_Bump_String          ((char *)"BUMP")
#define TM_Transparency_String  ((char *)"TRANSPARENCY")

// In case the programmer prefers the other naming convention:
#define TM_map_Ka_String    TM_Ambient_String
#define TM_map_Kd_String    TM_Diffuse_String
#define TM_map_Ks_String    TM_Specular_String
#define TM_map_Disp_String  TM_Displacement_String
#define TM_map_Bump_String  TM_Bump_String
#define TM_map_Tr_String    TM_Transparency_String

// Definitions for material property information:
#define MP_Dissolve     ((char *)"DISSOLVE")
#define MP_Ambient      ((char *)"AMBIENT_COLOR")
#define MP_Diffuse      ((char *)"DIFFUSE_COLOR")
#define MP_Specular     ((char *)"SPECULAR_COLOR")
#define MP_Shininess    ((char *)"SHININESS")

struct OBJMATERIAL {
    char                    name[MAX_CHAR] = "";        // newmtl

    vec4                    ambient;                    // Ka

    vec4                    diffuse;                    // Kd

    vec4                    specular;                   // Ks

    vec3                    transmission_filter;        // Tf

    int                     illumination_model;         // illum

    float                   dissolve;                   // d

    float                   specular_exponent;          // Ns

    float                   optical_density;            // Ni

    char                    map_ambient[MAX_CHAR];      // map_Ka

    char                    map_diffuse[MAX_CHAR];      // map_Kd

    char                    map_specular[MAX_CHAR];     // map_Ks

    char                    map_translucency[MAX_CHAR]; // map_Tr

    char                    map_disp[MAX_CHAR];         // disp or map_disp

    char                    map_bump[MAX_CHAR];         // bump or map_bump

    TEXTURE                 *texture_ambient;

    TEXTURE                 *texture_diffuse;

    TEXTURE                 *texture_specular;

    TEXTURE                 *texture_translucency;

    TEXTURE                 *texture_disp;

    TEXTURE                 *texture_bump;

    PROGRAM                 *program;

    MATERIALDRAWCALLBACK    *materialdrawcallback;

    const OBJ               *parent;

public:
    OBJMATERIAL(char *name=NULL, OBJ *parent=NULL);
    ~OBJMATERIAL();
    OBJMATERIAL(const OBJMATERIAL &src);
    OBJMATERIAL &operator=(const OBJMATERIAL &rhs);
    void draw();
    void build(PROGRAM *program);
    void set_draw_callback(MATERIALDRAWCALLBACK *materialdrawcallback);
};


struct OBJTRIANGLEINDEX
{
    int vertex_index[3];

    int uv_index[3];

public:
    OBJTRIANGLEINDEX(int vi[3], int uvi[3]) {
        vertex_index[0] = vi[0];
        vertex_index[1] = vi[1];
        vertex_index[2] = vi[2];

        uv_index[0] = uvi[0];
        uv_index[1] = uvi[1];
        uv_index[2] = uvi[2];
    }
};


struct OBJTRIANGLELIST
{
    std::vector<OBJTRIANGLEINDEX>   objtriangleindex;

    bool	 useuvs;

    // Nasty!  Nasty!  Nasty!
    // This is one case where the vector foobar can't just use its
    // size() method as a substitute for n_foobar.  The code releases
    // the indices (indice_array) to free up memory.  When that happens
    // indice_array.size() goes to 0.  So later in the code when we
    // call glDrawElements() we're passing in zero.  Retain
    // n_indice_array in the data structure to prevent bugs.
    unsigned short              n_indice_array;

    std::vector<unsigned short> indice_array;

    OBJMATERIAL                 *objmaterial;

    int                         mode;

    GLuint                      vbo;

public:
    OBJTRIANGLELIST();
    OBJTRIANGLELIST(int mode, bool useuvs, OBJMATERIAL *objmaterial);
    ~OBJTRIANGLELIST();
    OBJTRIANGLELIST(const OBJTRIANGLELIST &src);
    OBJTRIANGLELIST &operator=(const OBJTRIANGLELIST &rhs);
};


struct OBJVERTEXDATA {
    int vertex_index;

    int uv_index;

public:
    OBJVERTEXDATA(const int vi=0, const int uvi=0);
};

// Definitions for Vertex Attributes:
enum VertexAttribute {  // Use with glBindAttribLocation()
    VA_Position  = 0,
    VA_Normal    = 1,
    VA_TexCoord0 = 2,
    VA_Tangent0  = 3,
    VA_FNormal   = 4
    };

#define VA_Position_String  ((char *)"POSITION")
#define VA_Normal_String    ((char *)"NORMAL")
#define VA_TexCoord0_String ((char *)"TEXCOORD0")
#define VA_Tangent0_String  ((char *)"TANGENT0")
#define VA_FNormal_String   ((char *)"FNORMAL")

#define OFFSET_NO_TEXCOORD_NEEDED  (~0)

struct OBJMESH {
    char                            name[MAX_CHAR] = "";// o

    bool                            visible;

    char                            group[MAX_CHAR] = "";// g

    std::vector<OBJVERTEXDATA>      objvertexdata;

    std::vector<OBJTRIANGLELIST>    objtrianglelist;

    OBJMATERIAL                     *current_material;

    vec3                            location;

    vec3                            rotation;

    vec3                            scale;

    vec3                            min;

    vec3                            max;

    vec3                            dimension;

    float                           radius;

    float                           distance;

    GLuint                          vbo;

    unsigned int                    stride;

    unsigned int                    size;

    // offsets for vector attributes
    unsigned int                    offset[5] = { ~0, ~0, ~0, ~0, ~0 };

    GLuint                          vao;

    btRigidBody                     *btrigidbody;
    
    bool                            use_smooth_normals;
    
    const OBJ                       *parent;

public:
    OBJMESH(OBJ *parent=NULL);
    OBJMESH(char *name, bool visible, char *group, float scale_x,
            float scale_y, float scale_z, float distance,
            bool use_smooth_normals, OBJ *parent);
    ~OBJMESH();
    OBJMESH(const OBJMESH &src);
    OBJMESH &operator=(const OBJMESH &rhs);
    void add_vertex_data(const int vertex_index, const int uv_index);
    void update_bounds();
    void build_vbo();
    void set_attributes();
    void build();
    void build2();
    void optimize(unsigned int vertex_cache_size);
    void draw();
    void draw2();
    void draw3(GFX *gfx);
    void free_vertex_data();
};


struct OBJ {
    char                        texture_path[MAX_PATH] = "";

    char                        program_path[MAX_PATH] = "";

    std::vector<OBJMESH>        objmesh;

    std::vector<OBJMATERIAL>    objmaterial;

    std::vector<TEXTURE *>      texture;

    std::vector<PROGRAM *>      program;

    std::vector<vec3>           indexed_vertex;         // v

    std::vector<vec3>           indexed_normal;

    std::vector<vec3>           indexed_fnormal;

    std::vector<vec3>           indexed_tangent;
    
    std::vector<vec2>           indexed_uv;		// vt

public:
    OBJ(char *filename=NULL, const bool relative_path=true);
    ~OBJ();
    OBJMESH *get_mesh(const char *name, const bool exact_name);
    int get_mesh_index(const char *name, const bool exact_name);
    TEXTURE *get_texture(const char *name, const bool exact_name);
    OBJMATERIAL *get_material(const char *name, const bool exact_name);
    PROGRAM *get_program(const char *name, const bool exact_name);
    bool load_mtl(char *filename, const bool relative_path);
    void free_vertex_data();
    friend OBJMATERIAL;
private:
    int get_texture_index(char *filename) const;
    void add_texture(char *filename);
    int get_program_index(char *filename) const;
    void add_program(char *filename);
};

#endif
