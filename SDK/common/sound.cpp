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

#include "gfx.h"


SOUNDBUFFER *SOUNDBUFFER_load( char *name, MEMORY *memory )
{
	char ext[ MAX_CHAR ] = {""};
	
	get_file_extension( memory->filename, ext, 1 );
	
	if( !strcmp( ext, "OGG" ) )
	{
		SOUNDBUFFER *soundbuffer = ( SOUNDBUFFER * ) calloc( 1, sizeof( SOUNDBUFFER ) );
		
		unsigned int size;
		
		strcpy( soundbuffer->name, name );
		
		soundbuffer->file = ( OggVorbis_File * ) calloc( 1, sizeof( OggVorbis_File ) );
		
		ov_open_callbacks( memory,
						   soundbuffer->file,
						   NULL,
						   0,
						   audio.callbacks );

		soundbuffer->info = ov_info( soundbuffer->file, -1 );

		// Always NULL with LLVM GCC? compiler bug?
		if( soundbuffer->info )
		{
			size = ( ( unsigned int )ov_pcm_total( soundbuffer->file, -1 ) *
												   soundbuffer->info->channels << 1 );

			int count,
				bit;

			char *data  = ( char * ) malloc( size ),
				 *start = data;

			while( ( count = ov_read( soundbuffer->file,
									  start,
									  MAX_CHUNK_SIZE,
									  0,
									  2,
									  1,
									  &bit ) ) > 0 ) start += count;

			alGenBuffers( 1, &soundbuffer->bid[ 0 ] );

			alBufferData( soundbuffer->bid[ 0 ],
						  ( soundbuffer->info->channels == 1 ) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16,
						  data,
						  size,
						  soundbuffer->info->rate );
			
			free( data );
		}

		AUDIO_ogg_close( memory );

		ov_clear( soundbuffer->file );
		
		free( soundbuffer->file );
		
		soundbuffer->file = NULL;
		
		soundbuffer->info = NULL;
		
		return soundbuffer;
	}
	
	return NULL;
}


SOUNDBUFFER *SOUNDBUFFER_load_stream( char *name, MEMORY *memory )
{
	char ext[ MAX_CHAR ] = {""};
	
	get_file_extension( memory->filename, ext, 1 );
	
	if( !strcmp( ext, "OGG" ) )
	{
		unsigned int i = 0;
		
		SOUNDBUFFER *soundbuffer = ( SOUNDBUFFER * ) calloc( 1, sizeof( SOUNDBUFFER ) );
		
		strcpy( soundbuffer->name, name );
		
		soundbuffer->memory = memory;
		
		soundbuffer->file = ( OggVorbis_File * ) calloc( 1, sizeof( OggVorbis_File ) );
		
		ov_open_callbacks( memory,
						   soundbuffer->file,
						   NULL,
						   0,
						   audio.callbacks );

		soundbuffer->info = ov_info( soundbuffer->file, -1 );
		
		alGenBuffers( MAX_BUFFER, &soundbuffer->bid[ 0 ] );
		
		while( i != MAX_BUFFER )
		{
			SOUNDBUFFER_decompress_chunk( soundbuffer, i );
			++i;
		}
		
		return soundbuffer;
	}

	return NULL;
}


unsigned char SOUNDBUFFER_decompress_chunk( SOUNDBUFFER *soundbuffer, unsigned int buffer_index )
{
	char buffer[ MAX_CHUNK_SIZE ] = {""};

	int size = 0,
		bit;

	while( size < MAX_CHUNK_SIZE )
	{
		int count = ov_read( soundbuffer->file,
							 buffer + size,
							 MAX_CHUNK_SIZE - size,
							 0,
							 2,
							 1,
							 &bit );
							 
		if( count > 0 ) size += count;
		
		else break;
	}

	if( !size ) return 0;

	alBufferData( soundbuffer->bid[ buffer_index ],
				  ( soundbuffer->info->channels == 1 ) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16,
				  buffer,
				  size,
				  soundbuffer->info->rate );

	return 1;
}


SOUNDBUFFER *SOUNDBUFFER_free( SOUNDBUFFER *soundbuffer )
{
	unsigned int i = 0;
	
	while( i != MAX_BUFFER )
	{
		if( soundbuffer->bid[ i ] ) alDeleteBuffers( 1, &soundbuffer->bid[ i ] );
		++i;
	}
	
	if( soundbuffer->file )
	{
		ov_clear( soundbuffer->file );
		free( soundbuffer->file );
	}


	if( soundbuffer->memory ) AUDIO_ogg_close( soundbuffer->memory );

	free( soundbuffer );
	return NULL;
}


SOUND *SOUND_add( char *name, SOUNDBUFFER *soundbuffer )
{
	vec3 tmp = { 0.0f, 0.0f, 0.0f };
	
	SOUND *sound = ( SOUND * ) calloc( 1, sizeof( SOUND ) );

	strcpy( sound->name, name );
	
	sound->soundbuffer = soundbuffer;

	alGenSources( 1, &sound->sid );
	
    alSource3f( sound->sid, AL_POSITION, tmp.x, tmp.y, tmp.z );

    alSource3f( sound->sid, AL_VELOCITY, tmp.x, tmp.y, tmp.z );
	
    alSource3f( sound->sid, AL_DIRECTION, tmp.x, tmp.y, tmp.z );
	
    alSourcef( sound->sid, AL_ROLLOFF_FACTOR, 1.0f );
	
    alSourcei( sound->sid, AL_SOURCE_RELATIVE, AL_TRUE );
	
	alSourcef( sound->sid, AL_GAIN, 0.0f );
	
	alSourcef( sound->sid, AL_PITCH, 1.0f );
	
	return sound;
}


SOUND *SOUND_free( SOUND *sound )
{
	if( sound->sid )
	{
		SOUND_stop( sound );
		
		alDeleteSources( 1, &sound->sid );
	}

	free( sound );
	return NULL;
}


void SOUND_play( SOUND *sound, int loop )
{
	sound->loop = loop;
	
	if( !sound->soundbuffer->bid[ 1 ] )
	{
		alSourcei( sound->sid, AL_LOOPING, loop );
		
		alSourcei( sound->sid,
				   AL_BUFFER,
				   sound->soundbuffer->bid[ 0 ] );
	}
	else
	{
		alSourceQueueBuffers( sound->sid,
							  MAX_BUFFER,
							  &sound->soundbuffer->bid[ 0 ] );
	}

	alSourcePlay( sound->sid );
}


void SOUND_pause( SOUND *sound )
{ alSourcePause( sound->sid ); }


void SOUND_stop( SOUND *sound )
{ alSourceStop( sound->sid ); }


void SOUND_set_speed( SOUND *sound, float speed )
{ alSourcef( sound->sid, AL_PITCH, speed ); }

						
void SOUND_set_volume( SOUND *sound, float volume )
{ alSourcef( sound->sid, AL_GAIN, volume ); }


void SOUND_set_location( SOUND *sound, vec3 *location, float reference_distance )
{
    alSourcei( sound->sid, AL_SOURCE_RELATIVE, AL_FALSE );
	
	alSourcef( sound->sid, AL_REFERENCE_DISTANCE, reference_distance );
	
	alSource3f( sound->sid,
				AL_POSITION,
				location->x,
				location->y,
				location->z );
}


void SOUND_rewind( SOUND *sound )
{ alSourceRewind( sound->sid ); }


float SOUND_get_time( SOUND *sound )
{
	float playback_time = 0.0f;
	
	alGetSourcef( sound->sid, AL_SEC_OFFSET, &playback_time );
	
	return playback_time;
}


int SOUND_get_state( SOUND *sound )
{
	int state = 0;
	
	alGetSourcei( sound->sid, AL_SOURCE_STATE, &state );
	
	return state;
}


float SOUND_get_volume( SOUND *sound )
{
	float volume = 0.0f;
	
	alGetSourcef( sound->sid, AL_GAIN, &volume );
	
	return volume;
}


void SOUND_update_queue( SOUND *sound )
{
	unsigned int i = 0;

	int p,
		q;

    alGetSourcei( sound->sid, AL_BUFFERS_PROCESSED, &p );

	alGetSourcei( sound->sid, AL_BUFFERS_QUEUED, &q );
	
	while( p-- )
    {
		unsigned int bid;
		
        alSourceUnqueueBuffers( sound->sid,
								1,
								&bid );

        while( i != MAX_BUFFER )
        {
        	if( bid == sound->soundbuffer->bid[ i ] ) break;

        	++i;
        }

		if( SOUNDBUFFER_decompress_chunk( sound->soundbuffer, i ) )
		{
			alSourceQueueBuffers( sound->sid,
								  1,
								  &bid );
		}
    }
	

	if( !q && sound->loop )
	{
		AUDIO_ogg_seek( sound->soundbuffer->file->datasource,
						0,
						SEEK_SET );

		while( i != MAX_BUFFER )
		{
			SOUNDBUFFER_decompress_chunk( sound->soundbuffer, i );
			++i;
		}

		SOUND_play( sound, sound->loop );
	}
}
