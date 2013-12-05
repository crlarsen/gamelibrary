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


void TEXTURE::init(char *name)
{
    assert(name==NULL || strlen(name)<sizeof(this->name));
    if (name==NULL) {
        memset(this->name, 0, sizeof(this->name));
    } else {
        strcpy(this->name, name);
    }
}

TEXTURE::TEXTURE(char *name) : tid(0), width(0), height(0), byte(0),
                               size(0), target(GL_TEXTURE_2D),
                               internal_format(0), format(0),
                               texel_type(0), texel_array(NULL),
                               n_mipmap(0), compression(0)
{
    this->init(name);
}

TEXTURE::TEXTURE(char *name, char *filename, const bool relative_path,
                 unsigned int flags, unsigned char filter,
                 float anisotropic_filter) :
    tid(0), width(0), height(0), byte(0), size(0),
    target(GL_TEXTURE_2D), internal_format(0), format(0), texel_type(0),
    texel_array(NULL), n_mipmap(0), compression(0)
{
    this->init(name);

    MEMORY *m = new MEMORY(filename, relative_path);

    if (m) {
        this->load(m);

        this->generate_id(flags, filter, anisotropic_filter);

        this->free_texel_array();
        
        delete m;
    }
}

TEXTURE::~TEXTURE()
{
    this->free_texel_array();

    this->delete_id();
}

void TEXTURE::load(MEMORY *memory)
{
    char ext[MAX_CHAR] = {""};

    get_file_name(memory->filename, this->name);

    get_file_extension(memory->filename, ext, true);

    if (!strcmp(ext, "PNG"))
        this->load_png(memory);
    else if (!strcmp(ext, "PVR"))
        this->load_pvr(memory);
}


void png_memory_read(png_structp structp, png_bytep bytep, png_size_t size)
{
    MEMORY *m = (MEMORY *) png_get_io_ptr(structp);

    m->read(bytep, size);
}


void TEXTURE::load_png(MEMORY *memory)
{
    png_structp structp;

    png_infop infop;

    png_bytep *bytep = NULL;

    int n,
    png_bit_depth,
    png_color_type;

    structp = png_create_read_struct(PNG_LIBPNG_VER_STRING,
                                     NULL,
                                     NULL,
                                     NULL);

    infop = png_create_info_struct(structp);

    png_set_read_fn(structp, (png_voidp *)memory, png_memory_read);

    png_read_info(structp, infop);

    png_bit_depth = png_get_bit_depth(structp, infop);

    png_color_type = png_get_color_type(structp, infop);

    if (png_color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_expand(structp);

    if (png_color_type == PNG_COLOR_TYPE_GRAY && png_bit_depth < 8)
        png_set_expand(structp);

    if (png_get_valid(structp, infop, PNG_INFO_tRNS))
        png_set_expand(structp);

    if (png_bit_depth == 16)
        png_set_strip_16(structp);

    if (png_color_type == PNG_COLOR_TYPE_GRAY ||
        png_color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(structp);

    png_read_update_info(structp, infop);

    png_get_IHDR(structp,
                 infop,
                 (png_uint_32 *)(&this->width),
                 (png_uint_32 *)(&this->height),
                 &png_bit_depth,
                 &png_color_type,
                 NULL, NULL, NULL);

    switch (png_color_type) {
        case PNG_COLOR_TYPE_GRAY:
            this->byte            = 1;
            this->internal_format =
            this->format          = GL_LUMINANCE;

            break;

        case PNG_COLOR_TYPE_GRAY_ALPHA:
            this->byte            = 2;
            this->internal_format =
            this->format          = GL_LUMINANCE_ALPHA;

            break;

        case PNG_COLOR_TYPE_RGB:
            this->byte            = 3;
            this->internal_format =
            this->format          = GL_RGB;

            break;

        case PNG_COLOR_TYPE_RGB_ALPHA:
            this->byte            = 4;
            this->internal_format =
            this->format          = GL_RGBA;

            break;
    }

    this->texel_type = GL_UNSIGNED_BYTE;

    this->size = this->width * this->height * this->byte;

    this->texel_array = (unsigned char *) malloc(this->size);

    bytep = (png_bytep *) malloc(this->height * sizeof(png_bytep));
    
    
    for (int i=0; i!=this->height; ++i) {
        n = this->height - (i + 1);
        
        bytep[n] = this->texel_array + (n * this->width * this->byte);
    }
    
    
    png_read_image(structp, bytep);
    
    png_read_end(structp, NULL);
    
    png_destroy_read_struct(&structp,
                            &infop,
                            NULL);
    
    free(bytep);
}


void TEXTURE::load_pvr(MEMORY *memory)
{
    const char pvrtc_identifier[4] = { 'P', 'V', 'R', '!' };

    PVRHEADER *pvrheader = (PVRHEADER *)memory->buffer;
    char   *tag = (char *)&pvrheader->tag;

    if (strncmp(tag, pvrtc_identifier, sizeof(pvrtc_identifier)))
        return;


    if ((pvrheader->flags & 0xFF) == 24 || // PVRTC2
        (pvrheader->flags & 0xFF) == 25) { // PVRTC4
        this->width    = pvrheader->width;
        this->height   = pvrheader->height;
        this->byte     = pvrheader->bpp;
        this->n_mipmap = pvrheader->n_mipmap + 1;

        if (pvrheader->bitalpha) {
            this->compression = pvrheader->bpp == 4 ?
        GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG:
            GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
        } else {
            this->compression = pvrheader->bpp == 4 ?
        GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG:
            GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG;
        }

        this->texel_array = (unsigned char *) malloc(pvrheader->datasize);

        memcpy(this->texel_array,
               &memory->buffer[sizeof(PVRHEADER)],
               pvrheader->datasize);
    }
}


void TEXTURE::convert_16_bits(unsigned int use_5551)
{
    unsigned int s = this->width * this->height,
    *t = NULL;

    unsigned short *texel_array = NULL;

    switch (this->byte) {
        case 3:
        {
            unsigned char *tmp_array = NULL;

            this->byte		 = 4;
            this->size		 = s * this->byte;
            this->texel_type = GL_UNSIGNED_SHORT_5_6_5;

            tmp_array = (unsigned char *) malloc(this->size);

            for (int i=0, j=0; i!=this->size; i+=this->byte, j+=3) {
                tmp_array[i  ] = this->texel_array[j  ];
                tmp_array[i+1] = this->texel_array[j+1];
                tmp_array[i+2] = this->texel_array[j+2];
                tmp_array[i+3] = 255;
            }

            free(this->texel_array);
            this->texel_array = tmp_array;

            texel_array = (unsigned short *)this->texel_array;

            t = (unsigned int *)this->texel_array;

            for (int i=0; i!=s; ++i, ++t) {
                *texel_array++ = ((( *t        & 0xff) >> 3) << 11) |
                ((((*t >>  8) & 0xff) >> 2) <<  5) |
                ((*t >> 16) & 0xff) >> 3;
            }

            break;
        }

        case 4:
        {
            texel_array = (unsigned short *)this->texel_array;

            t = (unsigned int *)this->texel_array;

            if (use_5551) {
                this->texel_type = GL_UNSIGNED_SHORT_5_5_5_1;

                for (int i=0; i!=s; ++i, ++t) {
                    *texel_array++ = ((( *t        & 0xff) >> 3) << 11) |
                    ((((*t >>  8) & 0xff) >> 3) <<  6) |
                    ((((*t >> 16) & 0xff) >> 3) <<  1) |
                    ((*t >> 24) & 0xff) >> 7;
                }
            } else {
                this->texel_type = GL_UNSIGNED_SHORT_4_4_4_4;
		
                for (int i=0; i!=s; ++i, ++t) {
                    *texel_array++ = ((( *t        & 0xff) >> 4) << 12) |
                                     ((((*t >>  8) & 0xff) >> 4) <<  8) |
                                     ((((*t >> 16) & 0xff) >> 4) <<  4) |
                                       ((*t >> 24) & 0xff) >> 4;
                }
            }
            
            break;
        }
    }
}


void TEXTURE::generate_id(unsigned int flags,
                          unsigned char filter,
                          float anisotropic_filter)
{
    if (this->tid)
        this->delete_id();

    glGenTextures(1, &this->tid);

    glBindTexture(this->target, this->tid);


    if (!this->compression) {
        switch (this->byte) {
            case 1: glPixelStorei(GL_PACK_ALIGNMENT, 1); break;
            case 2: glPixelStorei(GL_PACK_ALIGNMENT, 2); break;
            case 3:
            case 4: glPixelStorei(GL_PACK_ALIGNMENT, 4); break;
        }

        if (flags & TEXTURE_16_BITS)
            this->convert_16_bits(flags & TEXTURE_16_BITS_5551);
    }


    if (flags & TEXTURE_CLAMP) {
        glTexParameteri(this->target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(this->target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    } else {
        glTexParameteri(this->target, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(this->target, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }


    if (anisotropic_filter) {
        static float texture_max_anisotropy = 0.0f;

        if (!texture_max_anisotropy)
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT,
                        &texture_max_anisotropy);

        anisotropic_filter = CLAMP(anisotropic_filter,
                                   0.0f,
                                   texture_max_anisotropy);

        glTexParameterf(this->target,
                        GL_TEXTURE_MAX_ANISOTROPY_EXT,
                        anisotropic_filter);
    }


    if (flags & TEXTURE_MIPMAP) {
        switch (filter) {
            case TEXTURE_FILTER_1X:
                glTexParameteri(this->target, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
                glTexParameteri(this->target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

                break;

            case TEXTURE_FILTER_2X:
                glTexParameteri(this->target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
                glTexParameteri(this->target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                break;

            case TEXTURE_FILTER_3X:
                glTexParameteri(this->target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glTexParameteri(this->target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                break;

            case TEXTURE_FILTER_0X:
            default:
                glTexParameteri(this->target, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
                glTexParameteri(this->target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

                break;
        }
    } else {
        switch (filter) {
            case TEXTURE_FILTER_0X:
                glTexParameteri(this->target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(this->target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

                break;

            default:
                glTexParameteri(this->target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(this->target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

                break;
        }
    }


    if (this->compression) {
        unsigned int width  = this->width,
        height = this->height,
        bsize  = this->byte == 4 ? 16 : 32,
        bwidth,
        bheight;

        for (int i=0, offset=0, size=0; i!=this->n_mipmap; ++i, offset += size) {
            if (width  < 1) width  = 1;
            if (height < 1) height = 1;

            bwidth = this->byte == 4 ?
            width >> 2:
            width >> 3;

            bheight = height >> 2;

            size = bwidth * bheight * ((bsize * this->byte) >> 3);

            if (size < 32) size = 32;

            glCompressedTexImage2D(this->target,
                                   i,
                                   this->compression,
                                   width,
                                   height,
                                   0,
                                   size,
                                   &this->texel_array[offset]);
            width  >>= 1;
            height >>= 1;
        }
    } else {
        glTexImage2D(this->target,
                     0,
                     this->internal_format,
                     this->width,
                     this->height,
                     0,
                     this->format,
                     this->texel_type,
                     this->texel_array);
    }
    
    
    if (flags & TEXTURE_MIPMAP && !this->n_mipmap) glGenerateMipmap(this->target);
}


void TEXTURE::delete_id()
{
    if (this->tid) {
        glDeleteTextures(1, &this->tid);
        this->tid = 0;
    }
}


void TEXTURE::free_texel_array()
{
    if (this->texel_array) {
        free(this->texel_array);
        this->texel_array = NULL;
    }
}


void TEXTURE::draw()
{
    glBindTexture(this->target, this->tid);
}

void TEXTURE::build(char            *texture_path,
                    unsigned int    flags,
                    unsigned char   filter,
                    float           anisotropic_filter)
{
    MEMORY *m = NULL;

    char filename[MAX_PATH] = {""};

    sprintf(filename, "%s%s", texture_path, this->name);

    m = new MEMORY(filename, false);

    if (m) {
        this->load(m);

        this->generate_id(flags,
                          filter,
                          anisotropic_filter);
        
        this->free_texel_array();
        
        delete m;
    }
}
