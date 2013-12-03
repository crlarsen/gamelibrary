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
 * - MEMORY
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

#include "gfx.h"

void PROGRAM::init(char *name) {
    assert(name==NULL || strlen(name)<sizeof(this->name));
    if (name==NULL) {
        memset(this->name, 0, sizeof(this->name));
    } else {
        strcpy(this->name, name);
    }
}

PROGRAM::PROGRAM(char *name) : vertex_shader(NULL),
                               fragment_shader(NULL),
                               pid(0),
                               programdrawcallback(NULL),
                               programbindattribcallback(NULL)
{
    this->init(name);
}

PROGRAM::PROGRAM(char                       *name,
                 char                       *vertex_shader_filename,
                 char                       *fragment_shader_filename,
                 const bool                 relative_path,
                 const bool                 debug_shader,
                 PROGRAMBINDATTRIBCALLBACK  *programbindattribcallback,
                 PROGRAMDRAWCALLBACK        *programdrawcallback) :
    pid(0),
    programdrawcallback(programdrawcallback),
    programbindattribcallback(programbindattribcallback)
{
    this->init(name);

    MEMORY *m = new MEMORY(vertex_shader_filename, relative_path);

    if (m) {
        this->vertex_shader = new SHADER(vertex_shader_filename, GL_VERTEX_SHADER);

        this->vertex_shader->compile((char *)m->buffer, debug_shader);

        delete m;
    }

    m = new MEMORY(fragment_shader_filename, relative_path);

    if (m) {
        this->fragment_shader = new SHADER(fragment_shader_filename, GL_FRAGMENT_SHADER);

        this->fragment_shader->compile((char *)m->buffer, debug_shader);

        delete m;
    }

    this->link(debug_shader);
}

PROGRAM::~PROGRAM()
{
    if (vertex_shader) delete vertex_shader;

    if (fragment_shader) delete fragment_shader;

    this->uniform_map.clear();

    this->vertex_attrib_map.clear();

    this->delete_id();
}

void PROGRAM::add_uniform(char *name, GLenum type)
{
    this->uniform_map[name].type     = type;
    this->uniform_map[name].location =
        glGetUniformLocation(this->pid, name);
    this->uniform_map[name].constant = false;
}


void PROGRAM::add_vertex_attrib(char *name, GLenum type)
{
    this->vertex_attrib_map[name].type     = type;
    this->vertex_attrib_map[name].location =
        glGetAttribLocation(this->pid, name);
}


bool PROGRAM::link(bool debug)
{
    GLenum  type;

    char *log,
    name[MAX_CHAR];

    int status,
    len,
    total,
    size;

    if (this->pid) return false;

    this->pid = glCreateProgram();

    glAttachShader(this->pid, this->vertex_shader->sid);

    glAttachShader(this->pid, this->fragment_shader->sid);

    if (this->programbindattribcallback) this->programbindattribcallback(this);

    glLinkProgram(this->pid);


    if (debug) {
        glGetProgramiv(this->pid, GL_INFO_LOG_LENGTH, &len);

        if (len) {
            log = (char *) malloc(len);

            glGetProgramInfoLog(this->pid, len, &len, log);

#ifdef __IPHONE_4_0

            printf("[ %s ]\n%s", this->name, log);
#else
            __android_log_print(ANDROID_LOG_ERROR, "", "[ %s ]\n%s", program->name, log);
#endif
            
            free(log);
        }
    }
    
    glGetProgramiv(this->pid, GL_LINK_STATUS, &status);

    if (!status) goto delete_program;


    if (debug) {
        glValidateProgram(this->pid);

        glGetProgramiv(this->pid, GL_INFO_LOG_LENGTH, &len);

        if (len) {
            log = (char *) malloc(len);

            glGetProgramInfoLog(this->pid, len, &len, log);

#ifdef __IPHONE_4_0

            printf("[ %s ]\n%s", this->name, log);
#else
            __android_log_print(ANDROID_LOG_ERROR, "", "[ %s ]\n%s", program->name, log);
#endif

            free(log);
        }


        glGetProgramiv(this->pid, GL_VALIDATE_STATUS, &status);

        if (!status) goto delete_program;
    }


    glGetProgramiv(this->pid, GL_ACTIVE_ATTRIBUTES, &total);

    for (int i=0; i != total; ++i) {
        glGetActiveAttrib(this->pid,
                          i,
                          MAX_CHAR,
                          &len,
                          &size,
                          &type,
                          name);

        this->add_vertex_attrib(name, type);
    }

    glGetProgramiv(this->pid, GL_ACTIVE_UNIFORMS, &total);

    for (int i=0; i != total; ++i) {
        glGetActiveUniform(this->pid,
                           i,
                           MAX_CHAR,
                           &len,
                           &size,
                           &type,
                           name);
        
        this->add_uniform(name, type);
    }
    
    return true;
    
    
delete_program:
    
    this->delete_id();
    
    return false;
}


void PROGRAM::set_draw_callback(PROGRAMDRAWCALLBACK *programdrawcallback)
{
    this->programdrawcallback = programdrawcallback;
}


void PROGRAM::set_bind_attrib_location_callback(PROGRAMBINDATTRIBCALLBACK *programbindattribcallback)
{
    this->programbindattribcallback = programbindattribcallback;
}


GLint PROGRAM::get_vertex_attrib_location(char *name)
{
    auto it = vertex_attrib_map.find(name);

    return it==vertex_attrib_map.end() ?
           static_cast<GLint>(-1) :
           it->second.location;
}


GLint PROGRAM::get_uniform_location(char *name)
{
    auto it = uniform_map.find(name);

    return it==uniform_map.end() ?
           static_cast<GLint>(-1) :
           it->second.location;
}


void PROGRAM::delete_id()
{
    if (this->pid) {
        glDeleteProgram(this->pid);

        this->pid = 0;
    }
}


void PROGRAM::draw()
{
    glUseProgram(this->pid);

    if (this->programdrawcallback) this->programdrawcallback(this);
}


bool PROGRAM::load_gfx(PROGRAMBINDATTRIBCALLBACK    *programbindattribcallback,
                       PROGRAMDRAWCALLBACK          *programdrawcallback,
                       char                         *filename,
                       const bool                   debug_shader,
                       const bool                   relative_path)
{
    MEMORY *m = new MEMORY(filename, relative_path);

    if (m) {
        char    vertex_token[MAX_CHAR]   = { "GL_VERTEX_SHADER"   },
        fragment_token[MAX_CHAR] = { "GL_FRAGMENT_SHADER" },
        *vertex_shader			 = strstr((char *)m->buffer, vertex_token),
        *fragment_shader		 = strstr((char *)m->buffer, fragment_token);


        get_file_name(filename, this->name);


        if ((vertex_shader && fragment_shader) && (fragment_shader > vertex_shader)) {
            this->vertex_shader = new SHADER(this->name, GL_VERTEX_SHADER);

            vertex_shader += strlen(vertex_token);

            *fragment_shader = 0;

            this->vertex_shader->compile(vertex_shader, debug_shader);


            this->fragment_shader = new SHADER(this->name, GL_FRAGMENT_SHADER);

            fragment_shader += strlen(fragment_token);

            this->fragment_shader->compile(fragment_shader, debug_shader);


            this->programbindattribcallback = programbindattribcallback;
            
            this->programdrawcallback = programdrawcallback;
            
            this->link(debug_shader);
        }
        
        delete m;
        
        return true;
    }
    
    return false;
}


void PROGRAM::build(PROGRAMBINDATTRIBCALLBACK   *programbindattribcallback,
                    PROGRAMDRAWCALLBACK         *programdrawcallback,
                    const bool                  debug_shader,
                    char                        *program_path)
{
    char filename[MAX_PATH] = {""};

    sprintf(filename, "%s%s", program_path, this->name);

    this->load_gfx(programbindattribcallback,
                   programdrawcallback,
                   filename,
                   debug_shader,
                   false);
}
