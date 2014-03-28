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

#include "gfx.h"


FONT::FONT(char *name) : character_data(NULL), font_size(0), texture_width(0),
                         texture_height(0), first_character(0),
                         count_character(0), tid(0)
{
    assert(name==NULL || strlen(name)<sizeof(this->name));
    strcpy(this->name, name ? name : "");

    this->program = new PROGRAM(name);

    this->program->vertex_shader = new SHADER(name, GL_VERTEX_SHADER);

    this->program->vertex_shader->compile("uniform mediump mat4 MODELVIEWPROJECTIONMATRIX;"
                                          "attribute mediump vec2 POSITION;"
                                          "attribute lowp vec2 TEXCOORD0;"
                                          "varying lowp vec2 texcoord0;"
                                          "void main( void ) {"
                                          "texcoord0 = TEXCOORD0;"
                                          "gl_Position = MODELVIEWPROJECTIONMATRIX * vec4( POSITION.x, POSITION.y, 0.0, 1.0 ); }",
                                          false);

    this->program->fragment_shader = new SHADER(name, GL_FRAGMENT_SHADER);

    this->program->fragment_shader->compile("uniform sampler2D DIFFUSE;"
                                            "uniform lowp vec4 COLOR;"
                                            "varying lowp vec2 texcoord0;"
                                            "void main( void ) {"
                                            "lowp vec4 color = texture2D( DIFFUSE, texcoord0 );"
                                            "color.x = COLOR.x;"
                                            "color.y = COLOR.y;"
                                            "color.z = COLOR.z;"
                                            "color.w *= COLOR.w;"
                                            "gl_FragColor = color; }",
                                            false);
    
    this->program->link(false);
}


FONT::~FONT()
{
    if (this->program)
        delete this->program;

    if (this->character_data)
        free(this->character_data);

    if (this->tid)
        glDeleteTextures(1, &this->tid);
}


bool FONT::load(char            *filename,
                const bool      relative_path,
                float           font_size,
                unsigned int    texture_width,
                unsigned int    texture_height,
                int             first_character,
                int             count_character)
{
    MEMORY *m = new MEMORY(filename, relative_path);

    if (m) {
        unsigned char *texel_array = (unsigned char *) malloc(texture_width * texture_height);

        this->character_data = (stbtt_bakedchar *) malloc(count_character * sizeof(stbtt_bakedchar));

        this->font_size = font_size;

        this->texture_width = texture_width;

        this->texture_height = texture_height;

        this->first_character = first_character;

        this->count_character = count_character;

        stbtt_BakeFontBitmap(m->buffer,
                             0,
                             font_size,
                             texel_array,
                             texture_width,
                             texture_height,
                             first_character,
                             count_character,
                             this->character_data);

        delete m;

        glGenTextures(1, &this->tid);

        glBindTexture(GL_TEXTURE_2D, this->tid);

        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     GL_ALPHA,
                     texture_width,
                     texture_height,
                     0,
                     GL_ALPHA,
                     GL_UNSIGNED_BYTE,
                     texel_array);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        free(texel_array);

        return true;
    }
    
    return false;
}


void FONT::print(float x, float y, char *text, vec4 *color)
{
    char    vertex_attribute   = this->program->get_vertex_attrib_location(VA_Position_String),
            texcoord_attribute = this->program->get_vertex_attrib_location(VA_TexCoord0_String);

    glBindVertexArrayOES(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glDisable(GL_CULL_FACE);

    glDisable(GL_DEPTH_TEST);

    glDepthMask(GL_FALSE);

    glEnable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    this->program->draw();

    glUniformMatrix4fv(this->program->get_uniform_location((char *)"MODELVIEWPROJECTIONMATRIX"),
                       1,
                       GL_FALSE,
                       GFX_get_modelview_projection_matrix().m());

    // In this situation it's not an error to use TM_Diffuse_String, and
    // TM_Ambient together.  This is exactly what the original code does.
    // At some point I should figure out why the author implemented the
    // code this way.
    glUniform1i(this->program->get_uniform_location(TM_Diffuse_String),
                TM_Ambient);

    if (color) {
        glUniform4fv(this->program->get_uniform_location((char *)"COLOR"),
                     1,
                     (float *)color);
    }

    glActiveTexture(GL_TEXTURE0);

    glBindTexture(GL_TEXTURE_2D, this->tid);

    glEnableVertexAttribArray(vertex_attribute);

    glEnableVertexAttribArray(texcoord_attribute);

    while (*text) {
        if (*text >= this->first_character &&
            *text <= (this->first_character + this->count_character)) {
            vec2 vert[4];

            vec2 uv[4];

            stbtt_aligned_quad quad;

            stbtt_bakedchar *bakedchar = this->character_data + (*text - this->first_character);

            int round_x = STBTT_ifloor(x + bakedchar->xoff);
            int round_y = STBTT_ifloor(y - bakedchar->yoff);

            quad.x0 = (float)round_x;
            quad.y0 = (float)round_y;
            quad.x1 = (float)round_x + bakedchar->x1 - bakedchar->x0;
            quad.y1 = (float)round_y - bakedchar->y1 + bakedchar->y0;

            quad.s0 = bakedchar->x0 / (float)this->texture_width;
            quad.t0 = bakedchar->y0 / (float)this->texture_width;
            quad.s1 = bakedchar->x1 / (float)this->texture_height;
            quad.t1 = bakedchar->y1 / (float)this->texture_height;

            x += bakedchar->xadvance;

            vert[0]->x = quad.x1; vert[0]->y = quad.y0;
            uv  [0]->x = quad.s1; uv  [0]->y = quad.t0;

            vert[1]->x = quad.x0; vert[1]->y = quad.y0;
            uv  [1]->x = quad.s0; uv  [1]->y = quad.t0;

            vert[2]->x = quad.x1; vert[2]->y = quad.y1;
            uv  [2]->x = quad.s1; uv  [2]->y = quad.t1;

            vert[3]->x = quad.x0; vert[3]->y = quad.y1;
            uv  [3]->x = quad.s0; uv  [3]->y = quad.t1;

            glVertexAttribPointer(vertex_attribute,
                                  2,
                                  GL_FLOAT,
                                  GL_FALSE,
                                  0,
                                  vert[0].v());
            
            glVertexAttribPointer(texcoord_attribute,
                                  2,
                                  GL_FLOAT,
                                  GL_FALSE,
                                  0,
                                  uv[0].v());
            
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }
	
        ++text;
    }
    
    glEnable(GL_CULL_FACE);
    
    glEnable(GL_DEPTH_TEST);
    
    glDepthMask(GL_TRUE);
    
    glDisable(GL_BLEND);
}


float FONT::length(char *text)
{
    float length = 0;

    while (*text) {
        if (*text >= this->first_character &&
            *text <= (this->first_character + this->count_character)) {
            stbtt_bakedchar *bakedchar = this->character_data + (*text - this->first_character);

            length += bakedchar->xadvance;
        }
	
        ++text;
    }
    
    return length;
}

