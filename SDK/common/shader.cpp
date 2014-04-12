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
 * - AUDIO
 * - FONT
 * - GFX
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

SHADER::SHADER(char *name, GLenum type) : type(type), sid(0)
{
    assert(strlen(name)<sizeof(this->name));
    strcpy(this->name, name ? name : "");
}


SHADER::~SHADER()
{
    this->delete_id();
}


bool SHADER::compile(const char *code, bool debug)
{
    char type[ MAX_CHAR ] = {""};

    GLint   loglen,
    status;

    if (this->sid) return false;

    this->sid = glCreateShader(this->type);

    glShaderSource(this->sid, 1, &code, NULL);

    glCompileShader(this->sid);

    if (debug) {
        if (this->type == GL_VERTEX_SHADER) strcpy(type, "GL_VERTEX_SHADER");
        else strcpy(type, "GL_FRAGMENT_SHADER");

        glGetShaderiv(this->sid, GL_INFO_LOG_LENGTH, &loglen);

        if (loglen) {
            GLchar  *log = (GLchar *) malloc(static_cast<size_t>(loglen));

            glGetShaderInfoLog(this->sid,
                               static_cast<GLsizei>(loglen),
                               static_cast<GLsizei *>(&loglen),
                               log);

        #ifdef __IPHONE_4_0
            printf("[ %s:%s ]\n%s", this->name, type, log);
        #else
            __android_log_print(ANDROID_LOG_ERROR, "", "[ %s:%s ]\n%s", this->name, type, log);
        #endif

            free(log);
        }
    }

    glGetShaderiv(this->sid, GL_COMPILE_STATUS, &status);
    
    if (!status) {
        this->delete_id();
        return false;
    }
    
    return true;
}


void SHADER::delete_id()
{
    if (this->sid) {
        glDeleteShader(this->sid);
        this->sid = 0;
    }
}
