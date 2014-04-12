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

// Extract code common to the constructors SOUNDBUFFER() and
// SOUNDBUFFERSTREAM().
void SOUNDBUFFER::init(const char *name, MEMORY *memory)
{
    assert(name==NULL || strlen(name)<sizeof(this->name));
    strcpy(this->name, name ? name : "");

    this->file = (OggVorbis_File *) calloc(1, sizeof(OggVorbis_File));

    ov_open_callbacks(memory,
                      this->file,
                      NULL,
                      0,
                      audio->callbacks);

    this->info = ov_info(this->file, -1);
}

SOUNDBUFFER::SOUNDBUFFER(const char *name, MEMORY *memory, AUDIO *audio) :
    file(NULL), memory(NULL), audio(audio)
{
    char ext[MAX_CHAR] = {""};

    get_file_extension(memory->filename, ext, true);

    if (!strcmp(ext, "OGG")) {
        this->init(name, memory);

        // Always NULL with LLVM GCC? compiler bug?
        if (this->info) {
            unsigned int size = ((unsigned int)ov_pcm_total(this->file, -1) *
                    this->info->channels << 1);

            int count,
                bit;

            char    *data  = (char *) malloc(size),
                    *start = data;

            while ((count = ov_read(this->file,
                                    start,
                                    MAX_CHUNK_SIZE,
                                    0,
                                    2,
                                    1,
                                    &bit)) > 0) start += count;

            alGenBuffers(1, &this->bid[0]);

            alBufferData(this->bid[0],
                         (this->info->channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16,
                         data,
                         size,
                         this->info->rate);

            free(data);
        }

        audio->ogg_close(memory);

        ov_clear(this->file);

        free(this->file);

        this->file = NULL;

        this->info = NULL;
    }
}

SOUNDBUFFERSTREAM::SOUNDBUFFERSTREAM(const char *name, MEMORY *memory, AUDIO *audio) : SOUNDBUFFER(audio)
{
    char ext[MAX_CHAR] = {""};

    get_file_extension(memory->filename, ext, true);

    if (!strcmp(ext, "OGG")) {
        this->init(name, memory);

        this->memory = memory;

        alGenBuffers(MAX_BUFFER, &this->bid[0]);

        for (int i=0; i!=MAX_BUFFER; ++i)
            this->decompress_chunk(i);
    }
}

unsigned char SOUNDBUFFER::decompress_chunk(unsigned int buffer_index)
{
    char buffer[MAX_CHUNK_SIZE] = {""};

    int size = 0,
    bit;

    while (size < MAX_CHUNK_SIZE) {
        int count = ov_read(this->file,
                            buffer + size,
                            MAX_CHUNK_SIZE - size,
                            0,
                            2,
                            1,
                            &bit);

        if (count > 0) size += count;

        else break;
    }

    if (!size) return 0;

    alBufferData(this->bid[buffer_index],
                 (this->info->channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16,
                 buffer,
                 size,
                 this->info->rate);

    return 1;
}


SOUNDBUFFER::~SOUNDBUFFER()
{
    unsigned int i = 0;

    while (i != MAX_BUFFER) {
        if (this->bid[i]) alDeleteBuffers(1, &this->bid[i]);
        ++i;
    }

    if (this->file) {
        ov_clear(this->file);
        free(this->file);
    }


    if (this->memory) audio->ogg_close(this->memory);
}


SOUND::SOUND(char *name, SOUNDBUFFER *soundbuffer) :
    loop(false), soundbuffer(soundbuffer)
{
    vec3 tmp(0.0f, 0.0f, 0.0f);

    assert(name==NULL || strlen(name)<sizeof(this->name));
    strcpy(this->name, name ? name : "");

    alGenSources(1, &this->sid);

    alSource3f(this->sid, AL_POSITION, tmp->x, tmp->y, tmp->z);

    alSource3f(this->sid, AL_VELOCITY, tmp->x, tmp->y, tmp->z);

    alSource3f(this->sid, AL_DIRECTION, tmp->x, tmp->y, tmp->z);

    alSourcef(this->sid, AL_ROLLOFF_FACTOR, 1.0f);

    alSourcei(this->sid, AL_SOURCE_RELATIVE, AL_TRUE);

    alSourcef(this->sid, AL_GAIN, 0.0f);

    alSourcef(this->sid, AL_PITCH, 1.0f);
}


SOUND::~SOUND()
{
    if (this->sid) {
        this->stop();

        alDeleteSources(1, &this->sid);
    }
}


void SOUND::play(int loop)
{
    this->loop = loop;

    if (!this->soundbuffer->bid[1]) {
        alSourcei(this->sid, AL_LOOPING, loop);

        alSourcei(this->sid,
                  AL_BUFFER,
                  this->soundbuffer->bid[0]);
    } else {
        alSourceQueueBuffers(this->sid,
                             MAX_BUFFER,
                             &this->soundbuffer->bid[0]);
    }

    alSourcePlay(this->sid);
}


void SOUND::pause()
{
    alSourcePause(this->sid);
}


void SOUND::stop()
{
    alSourceStop(this->sid);
}


void SOUND::set_speed(float speed)
{
    alSourcef(this->sid, AL_PITCH, speed);
}


void SOUND::set_volume(float volume)
{
    alSourcef(this->sid, AL_GAIN, volume);
}


void SOUND::set_location(vec3 &location, float reference_distance)
{
    alSourcei(this->sid, AL_SOURCE_RELATIVE, AL_FALSE);

    alSourcef(this->sid, AL_REFERENCE_DISTANCE, reference_distance);

    alSource3f(this->sid,
               AL_POSITION,
               location->x,
               location->y,
               location->z);
}


void SOUND::rewind()
{
    alSourceRewind(this->sid);
}


float SOUND::get_time()
{
    float playback_time = 0.0f;

    alGetSourcef(this->sid, AL_SEC_OFFSET, &playback_time);

    return playback_time;
}


int SOUND::get_state()
{
    int state = 0;

    alGetSourcei(this->sid, AL_SOURCE_STATE, &state);

    return state;
}


float SOUND::get_volume()
{
    float volume = 0.0f;

    alGetSourcef(this->sid, AL_GAIN, &volume);

    return volume;
}


void SOUND::update_queue()
{
    unsigned int i = 0;
    
    int p,
    q;
    
    alGetSourcei(this->sid, AL_BUFFERS_PROCESSED, &p);
    
    alGetSourcei(this->sid, AL_BUFFERS_QUEUED, &q);
    
    while (p--) {
        unsigned int bid;
        
        alSourceUnqueueBuffers(this->sid,
                               1,
                               &bid);
        
        while (i != MAX_BUFFER) {
            if (bid == this->soundbuffer->bid[i]) break;
            
            ++i;
        }
        
        if (this->soundbuffer->decompress_chunk(i)) {
            alSourceQueueBuffers(this->sid,
                                 1,
                                 &bid);
        }
    }
    
    
    if (!q && this->loop) {
        soundbuffer->audio->ogg_seek(this->soundbuffer->file->datasource,
                                     0,
                                     SEEK_SET);

        while (i != MAX_BUFFER) {
            this->soundbuffer->decompress_chunk(i);
            ++i;
        }
        
        this->play(this->loop);
    }
}
