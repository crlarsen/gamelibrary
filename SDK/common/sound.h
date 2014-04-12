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

#ifndef SOUND_H
#define SOUND_H

#define MAX_BUFFER 4

#define MAX_CHUNK_SIZE  (1024 << 3)


struct SOUNDBUFFER {
    char		name[MAX_CHAR];

    OggVorbis_File	*file;

    vorbis_info		*info;

    MEMORY		*memory;

    unsigned int	bid[MAX_BUFFER] = {};
protected:
    SOUNDBUFFER() {}
    void init(const char *name, MEMORY *memory);
public:
    SOUNDBUFFER(const char *name, MEMORY *memory);
    ~SOUNDBUFFER();
    unsigned char decompress_chunk(unsigned int buffer_index);
};

struct SOUNDBUFFERSTREAM : SOUNDBUFFER {
public:
    SOUNDBUFFERSTREAM(const char *name, MEMORY *memory);
};

struct SOUND {
    char		name[ MAX_CHAR ];

    unsigned int	sid;

    int			loop;

    SOUNDBUFFER		*soundbuffer;
public:
    SOUND(char *name, SOUNDBUFFER *soundbuffer);
    ~SOUND();
    void play(int loop);
    void pause();
    void stop();
    void set_speed(float speed);
    void set_volume(float volume);
    void set_location(vec3 &location, float reference_distance);
    void rewind();
    float get_time();
    int get_state();
    float get_volume();
    void update_queue();
};

#endif
