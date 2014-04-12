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

// These functions need to be declared as functions which aren't members of AUDIO
// so that they will be callable by the Ogg Vorbis library.  They're also declared
// "static" so they aren't visible to most of the outside world.  AUDIO class
// member fuctions are provided which just wrap these routines; the member functions
// give public access to these "private" functions.
static size_t AUDIO_ogg_read(void *ptr, size_t size, size_t read, void *memory_ptr);

static int AUDIO_ogg_seek(void *memory_ptr, ogg_int64_t offset, int stride);

static long AUDIO_ogg_tell(void *memory_ptr);

static int AUDIO_ogg_close(void *memory_ptr);

AUDIO::AUDIO() :
    al_device(alcOpenDevice(NULL)),
    al_context(alcCreateContext(al_device, NULL))
{
    alcMakeContextCurrent(this->al_context);

    console_print( "\nAL_VENDOR:       %s\n", ( char * )alGetString ( AL_VENDOR     ) );
    console_print( "AL_RENDERER:     %s\n"  , ( char * )alGetString ( AL_RENDERER   ) );
    console_print( "AL_VERSION:      %s\n"  , ( char * )alGetString ( AL_VERSION    ) );
    console_print( "AL_EXTENSIONS:   %s\n"  , ( char * )alGetString ( AL_EXTENSIONS ) );

    this->callbacks.read_func  = AUDIO_ogg_read;
    this->callbacks.seek_func  = AUDIO_ogg_seek;
    this->callbacks.tell_func  = AUDIO_ogg_tell;
    this->callbacks.close_func = AUDIO_ogg_close;
}


AUDIO::~AUDIO( void )
{
    alcMakeContextCurrent(NULL);

    alcDestroyContext(al_context);

    alcCloseDevice(al_device);

    this->callbacks.read_func  = NULL;
    this->callbacks.seek_func  = NULL;
    this->callbacks.tell_func  = NULL;
    this->callbacks.close_func = NULL;
}


void AUDIO::error()
{
    unsigned int error;

    while( ( error = glGetError() ) != GL_NO_ERROR )
    {
        char str[ MAX_CHAR ] = {""};

        switch( error )
        {
            case AL_INVALID_NAME:
            {
                strcpy( str, "AL_INVALID_NAME" );
                break;
            }

            case AL_INVALID_ENUM:
            {
                strcpy( str, "AL_INVALID_ENUM" );
                break;
            }

            case AL_INVALID_VALUE:
            {
                strcpy( str, "AL_INVALID_VALUE" );
                break;
            }

            case AL_INVALID_OPERATION:
            {
                strcpy( str, "AL_INVALID_OPERATION" );
                break;
            }
                
            case AL_OUT_OF_MEMORY:
            {
                strcpy( str, "AL_OUT_OF_MEMORY" );
                break;
            }
        }
        
        console_print( "[ AL_ERROR ]\nERROR: %s\n", str );
    }
}


void AUDIO::set_listener(vec3 &location, vec3 &direction, vec3 &up)
{
    float orientation[6] = { direction->x, direction->y, direction->z,
        up->x, up->y, up->z };

    alListener3f(AL_POSITION,
                 location->x,
                 location->y,
                 location->z);

    alListenerfv(AL_ORIENTATION, &orientation[0]);
}


static size_t AUDIO_ogg_read( void *ptr, size_t size, size_t read, void *memory_ptr )
{
    unsigned int    seof,
                    pos;

    MEMORY *memory = ( MEMORY * )memory_ptr;

    seof = memory->size - memory->position;

    pos = ( ( read * size ) < seof ) ?
    pos = read * size :
    pos = seof;

    if (pos) {
        memcpy( ptr, memory->buffer + memory->position, pos );

        memory->position += pos;
    }
    
    return pos;
}

size_t AUDIO::ogg_read(void *ptr, size_t size, size_t read, void *memory_ptr)
{
    return AUDIO_ogg_read(ptr, size, read, memory_ptr);
}

static int AUDIO_ogg_seek( void *memory_ptr, ogg_int64_t offset, int stride )
{
    unsigned int pos;

    MEMORY *memory = ( MEMORY * )memory_ptr;

    switch( stride )
    {
        case SEEK_SET:
        {
            pos = ( memory->size >= offset ) ?
            pos = ( unsigned int )offset :
            pos = memory->size;

            memory->position = pos;

            break;
        }

        case SEEK_CUR:
        {
            unsigned int seof = memory->size - memory->position;

            pos = ( offset < seof ) ?
            pos = ( unsigned int )offset :
            pos = seof;
            
            memory->position += pos;
            
            break;
        }
            
        case SEEK_END:
        {
            memory->position = memory->size + 1;
            
            break;
        }
    };
    
    return 0;
}

int AUDIO::ogg_seek(void *memory_ptr, ogg_int64_t offset, int stride)
{
    return AUDIO_ogg_seek(memory_ptr, offset, stride);
}


static long AUDIO_ogg_tell( void *memory_ptr )
{
	MEMORY *memory = ( MEMORY * )memory_ptr;

	return memory->position;
}

long AUDIO::ogg_tell(void *memory_ptr)
{
    return AUDIO_ogg_tell(memory_ptr);
}

static int AUDIO_ogg_close( void *memory_ptr )
{ return memory_ptr ? 1 : 0; }

int AUDIO::ogg_close(void *memory_ptr) {
    return AUDIO_ogg_close(memory_ptr);
}

