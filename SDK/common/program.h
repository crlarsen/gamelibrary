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
 * - TEXTURE
 */

#ifndef PROGRAM_H
#define PROGRAM_H


typedef struct
{
    GLenum  type;

    GLint   location;

    bool    constant;

} UNIFORM;


typedef struct
{
    GLenum  type;

    GLint   location;

} VERTEX_ATTRIB;


typedef void(PROGRAMDRAWCALLBACK(void *));

typedef void(PROGRAMBINDATTRIBCALLBACK(void *));


struct PROGRAM {
    char                                name[MAX_CHAR];

    SHADER                              *vertex_shader;

    SHADER                              *fragment_shader;

    GLuint                              pid;

    std::map<std::string,UNIFORM>       uniform_map;

    std::map<std::string,VERTEX_ATTRIB> vertex_attrib_map;

    PROGRAMDRAWCALLBACK                 *programdrawcallback;

    PROGRAMBINDATTRIBCALLBACK           *programbindattribcallback;
private:
    void init(char *name);
    void add_vertex_attrib(char *name, GLenum type);
    void add_uniform(char *name, GLenum type);
public:
    PROGRAM(char *name);
    PROGRAM(char *name, char *vertex_shader_filename,
            char *fragment_shader_filename, const bool relative_path,
            const bool debug_shader,
            PROGRAMBINDATTRIBCALLBACK *programbindattribcallback,
            PROGRAMDRAWCALLBACK *programdrawcallback);
    ~PROGRAM();
    bool link(bool debug);
    void set_draw_callback(PROGRAMDRAWCALLBACK *programdrawcallback);
    void set_bind_attrib_location_callback(PROGRAMBINDATTRIBCALLBACK *programbindattribcallback);
    GLint get_vertex_attrib_location(char *name);
    GLint get_uniform_location(char *name);
    void delete_id();
    void draw();
    void reset();
    bool load_gfx(PROGRAMBINDATTRIBCALLBACK	*programbindattribcallback, PROGRAMDRAWCALLBACK	*programdrawcallback, char *filename, const bool debug_shader, const bool relative_path);
    void build(PROGRAMBINDATTRIBCALLBACK *programbindattribcallback,
               PROGRAMDRAWCALLBACK *programdrawcallback,
               bool debug_shader, char *program_path);
};

#endif
