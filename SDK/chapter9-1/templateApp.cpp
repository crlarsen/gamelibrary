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
/*
 * Source code modified by Chris Larsen to make the following data types into
 * proper C++ classes:
 * - MEMORY
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

#include "templateApp.h"

/* The main structure of the template. This is a pure C struct, you initialize the structure
   as demonstrated below. Depending on the type of your type of app simply comment / uncomment
   which event callback you want to use. */

TEMPLATEAPP templateApp = {
    /* Will be called once when the program start. */
    templateAppInit,

    /* Will be called every frame. This is the best location to plug your drawing. */
    templateAppDraw,

    /* This function will be triggered when a new touche is recorded on screen. */
    //templateAppToucheBegan,

    /* This function will be triggered when an existing touche is moved on screen. */
    //templateAppToucheMoved,

    /* This function will be triggered when an existing touche is released from the the screen. */
    //templateAppToucheEnded,

    /* This function will be called everytime the accelerometer values are refreshed. Please take
     not that the accelerometer can only work on a real device, and not on the simulator. In addition
     you will have to turn ON the accelerometer functionality to be able to use it. This will be
     demonstrated in the book later on. */
    //templateAppAccelerometer // Turned off by default.
};

/* The hardware device to use with OpenAL. */
ALCdevice *al_device;
/* The OpenAL context.  This context contains the global OpenAL states and
 * variables.  This is also where the IDs for the sound buffers and sound
 * sources are maintained.
 */
ALCcontext *al_context;

ALuint soundbuffer;

ALuint soundsource;

void templateAppInit( int width, int height )
{
    // Setup the exit callback function.
    atexit( templateAppExit );

    // Initialize GLES.
    GFX_start();

    // Setup a GLES viewport using the current width and height of the screen.
    glViewport( 0, 0, width, height );

    /* Open the first valid device OpenAL finds and use it for playback. */
    al_device = alcOpenDevice(NULL);
    /* Create a vanilla OpenAL context (linking the device initialized above). */
    al_context = alcCreateContext(al_device, NULL);
    /* Activate the context you've just created and make it the current context. */
    alcMakeContextCurrent(al_context);
    
    /* Ask OpenAL to give you a valid ID for a new sound buffer. */
    alGenBuffers( 1, &soundbuffer );
    /* Load the raw sound file in memory. */
    MEMORY *memory = new MEMORY((char *)"test.raw", true);
    /* Send over the content of the file to the audio memory. */
    alBufferData( soundbuffer,
                 AL_FORMAT_MONO16,  // Tell OpenAL that the sound file is
                                    // mono and 16 PCM.
                 memory->buffer,    // The complete sound file memory buffer.
                 memory->size,      // The total size of the sound buffer in bytes.
                 22050 );           // The playback rate of the sound in Hz.
    /* Close and free the memory.  At this point, the sound buffer data is
     * maintained by OpenAL and is stored in audio memory, so there's no need to
     * keep the buffer alive in local memory.
     */
    delete memory;
    memory = NULL;

    /* Ask OpenAL to generate a new source ID. */
    alGenSources( 1, &soundsource );
    /* Attach the sound buffer ID to the sound source.  This operation will tell
     * OpenAL that when the source is played, the data associated with the sound
     * buffer ID will be used.
     */
    alSourcei( soundsource,
              AL_BUFFER,
              soundbuffer );
    /* Play the sound buffer for the current sound source. */
    alSourcePlay( soundsource );
}


void templateAppDraw( void )
{
    // Clear the depth, stencil and colorbuffer.
    glClear( GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT );
    
    /* Temp. variable to hold the state of the sound source. */
    int state = 0;
    /* Request OpenAL to give you the current state of the source.  The result
     * will indicate if the source is currently playing, paused, or simply
     * stopped.
     */
    alGetSourcei(soundsource, AL_SOURCE_STATE, &state);
    /* If the sound is playing... */
    if (state == AL_PLAYING) {
        /* Set the clear color of the screen to green. */
        glClearColor(0, 1, 0, 1);
        /* Declare a temporary variable to hold the curren playback time. */
        float playback_time = 0.0f;
        /* Request the current offset in seconds for the current sound source. */
        alGetSourcef(soundsource, AL_SEC_OFFSET, &playback_time);
        /* Print the current playback time on the console. */
        console_print("%f\n", playback_time);
    } else {
        /* The sound stops playing; set the clear color to red. */
        glClearColor(1, 0, 0, 1);
    }
}


void templateAppToucheBegan( float x, float y, unsigned int tap_count )
{
    /* Insert code to execute when a new touche is detected on screen. */
}


void templateAppToucheMoved( float x, float y, unsigned int tap_count )
{
    /* Insert code to execute when a touche move on screen. */
}


void templateAppToucheEnded( float x, float y, unsigned int tap_count )
{
    /* Insert code to execute when a touche is removed from the screen. */
}


void templateAppAccelerometer( float x, float y, float z )
{
    /* Insert code to execute with the accelerometer values ( when available on the system ). */
}


void templateAppExit( void )
{
    /* Delete the sound buffer and invalidate the ID. */
    alDeleteBuffers(1, &soundbuffer);
    /* Delete the sound source and invalidate the ID. */
    alDeleteSources(1, &soundsource);
    /* Set a null context so the one you created in the previous step can
     * be destroyed.
     */
    alcMakeContextCurrent(NULL);
    /* Destroy the OpenAL context. */
    alcDestroyContext(al_context);
    /* Close the device. */
    alcCloseDevice(al_device);
}
