/*

Book:      	Game and Graphics Programming for iOS and Android with OpenGL(R) ES 2.0
Author:    	Romain Marucchi-Foino
ISBN-10: 	1119975913
ISBN-13: 	978-1119975915
Publisher: 	John Wiley & Sons	

Copyright (C) 2011 Romain Marucchi-Foino

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of
this software. Permission is granted to anyone who either own or purchase a copy of
the book specified above, to use this software for any purpose, including commercial
applications subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that
you wrote the original software. If you use this software in a product, an acknowledgment
in the product would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be misrepresented
as being the original software.

3. This notice may not be removed or altered from any source distribution.

*/

#include "templateApp.h"

TEMPLATEAPP templateApp = {
							templateAppInit,
							templateAppDraw
						  };


ALCdevice *al_device;

ALCcontext *al_context;

ALuint soundbuffer;

ALuint soundsource;


void templateAppInit( int width, int height )
{
	atexit( templateAppExit );
	
	GFX_start();
	
	glViewport( 0, 0, width, height );



	al_device = alcOpenDevice( NULL );

	al_context = alcCreateContext( al_device, NULL );

	alcMakeContextCurrent( al_context );


	alGenBuffers( 1, &soundbuffer );


	MEMORY *memory = mopen( ( char * )"test.raw", 1 );
	
	alBufferData( soundbuffer,
				  AL_FORMAT_MONO16,
				  memory->buffer,
				  memory->size,
				  22050 );
	
	memory = mclose( memory );
	
	
	alGenSources( 1, &soundsource );

	alSourcei( soundsource,
			   AL_BUFFER,
			   soundbuffer );

	alSourcePlay( soundsource );	
}


void templateAppDraw( void )
{
	glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );

	int state = 0;
	
	alGetSourcei( soundsource, AL_SOURCE_STATE, &state );	
	
	if( state == AL_PLAYING )
	{
		glClearColor( 0.0f, 1.0f, 0.0f, 1.0f );

		float playback_time = 0.0f;
		
		alGetSourcef( soundsource, AL_SEC_OFFSET, &playback_time );

		console_print( "%f\n", playback_time );
	}
	
	else
		glClearColor( 1.0f, 0.0f, 0.0f, 1.0 );	
}


void templateAppExit( void )
{
	alDeleteBuffers( 1, &soundbuffer );
	
	alDeleteSources( 1, &soundsource );


	alcMakeContextCurrent( NULL );

	alcDestroyContext( al_context );

	alcCloseDevice( al_device );
}
