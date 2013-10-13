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

#ifndef SOUND_H
#define SOUND_H

#define MAX_BUFFER 4

#define MAX_CHUNK_SIZE 1024 << 3


typedef struct
{
	char			name[ MAX_CHAR ];
	
	OggVorbis_File	*file;

	vorbis_info		*info;
	
	MEMORY			*memory;
	
	unsigned int	bid[ MAX_BUFFER ];

} SOUNDBUFFER;


typedef struct
{
	char			name[ MAX_CHAR ];
	
	unsigned int	sid;

	int				loop;

	SOUNDBUFFER		*soundbuffer;

} SOUND;


SOUNDBUFFER *SOUNDBUFFER_load( char *name, MEMORY *memory );

SOUNDBUFFER *SOUNDBUFFER_load_stream( char *name, MEMORY *memory );

unsigned char SOUNDBUFFER_decompress_chunk( SOUNDBUFFER *soundbuffer, unsigned int buffer_index );

SOUNDBUFFER *SOUNDBUFFER_free( SOUNDBUFFER *soundbuffer );

SOUND *SOUND_add( char *name, SOUNDBUFFER *soundbuffer );

SOUND *SOUND_free( SOUND *sound );

void SOUND_play( SOUND *sound, int loop );

void SOUND_pause( SOUND *sound );

void SOUND_stop( SOUND *sound );

void SOUND_set_speed( SOUND *sound, float speed );

void SOUND_set_volume( SOUND *sound, float volume );

void SOUND_set_location( SOUND *sound, vec3 *location, float reference_distance );

void SOUND_rewind( SOUND *sound );

float SOUND_get_time( SOUND *sound );

int SOUND_get_state( SOUND *sound );

float SOUND_get_volume( SOUND *sound );

void SOUND_update_queue( SOUND *sound );

#endif
