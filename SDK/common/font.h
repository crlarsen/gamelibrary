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


#ifndef FONT_H
#define FONT_H


struct FONT {
    char            name[MAX_CHAR] = "";

    stbtt_bakedchar *character_data;

    float           font_size;

    int             texture_width;

    int             texture_height;

    int             first_character;

    int             count_character;

    PROGRAM         *program;
    
    unsigned int    tid;
public:
    FONT(char *name);
    ~FONT();
    bool load(char *filename, const bool relative_path, float font_size,
              unsigned int texture_width, unsigned int texture_height,
              int first_character, int count_character);
    void print(GFX *gfx, float x, float y, char *text, vec4 *color);
    float length(char *text);
};

#endif
