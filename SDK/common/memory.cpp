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

#include "gfx.h"

MEMORY::MEMORY(const char *filename, const bool relative_path) :
    size(0), position(0), buffer(NULL)
{
    #ifdef __IPHONE_4_0

        FILE *f;
		
        char fname[MAX_PATH] = {""};
		
        assert(filename!=NULL);
        if (relative_path) {
            get_file_path(getenv("FILESYSTEM"), fname);
			
            strcat(fname, filename);
        } else {
            strcpy(fname, filename);
        }

        f = fopen(fname, "rb");
		
        if (!f) return;

        assert(strlen(fname)<sizeof(this->filename));
        strcpy(this->filename, fname);
		
		
        fseek(f, 0, SEEK_END);
        this->size = ftell(f);
        fseek(f, 0, SEEK_SET);
		
		
        this->buffer = (unsigned char *) calloc(1, this->size + 1);
        fread(this->buffer, this->size, 1, f);
        this->buffer[this->size] = 0;
		
		
        fclose(f);
		
        return;
	
	
    #else
        char fpath[MAX_PATH] = {""},
             fname[MAX_PATH] = {""};

        unzFile		    uf;
        unz_file_info   fi;
        unz_file_pos    fp;

        strcpy(fpath, getenv("FILESYSTEM"));

        uf = unzOpen(fpath);
		
        if (!uf) return;

        if (relative_path)
            sprintf(fname, "assets/%s", filename);
        else
            strcpy(fname, filename);
		
        unzGoToFirstFile(uf);

        unzGetFilePos(uf, &fp);
		
        if (unzLocateFile(uf, fname, 1) == UNZ_OK) {
            unzGetCurrentFileInfo(uf,
                                  &fi,
                                  this->filename,
                                  MAX_PATH,
                                  NULL, 0,
                                  NULL, 0);

            if (unzOpenCurrentFilePassword(uf, NULL) == UNZ_OK) {
                this->position = 0;
                this->size     = fi.uncompressed_size;
                this->buffer   = (unsigned char *) realloc(this->buffer, fi.uncompressed_size + 1);
                this->buffer[fi.uncompressed_size] = 0;

                while (unzReadCurrentFile(uf, this->buffer, fi.uncompressed_size) > 0){}

                unzCloseCurrentFile(uf);

                unzClose(uf);
					
                return;
            }
        }
		
        unzClose(uf);

        return;
		
    #endif
}


MEMORY::~MEMORY()
{
    if (this->buffer)
        free(this->buffer);
}


unsigned int MEMORY::read(void *dst, unsigned int size)
{
    if ((this->position + size) > this->size)
        size = this->size - this->position;

    memcpy(dst, &this->buffer[this->position], size);
	
    this->position += size;

    return size;
}


void MEMORY::insert(const char *str, const unsigned int position)
{
    unsigned int s1 = strlen(str),
                 s2 = this->size + s1 + 1;

    char    *buffer = (char *)this->buffer,
            *tmp    = (char *)calloc(1, s2);
	
    if (position)
        strncpy(&tmp[0], &buffer[0], position);

    strcat(&tmp[position], str);
	
    strcat(&tmp[position + s1], &buffer[position]);

    this->size = s2;
	
    free(this->buffer);
    this->buffer = (unsigned char *)tmp;
}
