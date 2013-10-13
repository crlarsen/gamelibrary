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


void templateAppInit( int width, int height )
{
	// Setup the exit callback function.
	atexit( templateAppExit );
	
	// Initialize GLES.
	GFX_start();
	
	// Setup a GLES viewport using the current width and height of the screen.
	glViewport( 0, 0, width, height );
	
	/* Insert your initialization code here */
}


void templateAppDraw( void )
{
	// Clear the depth, stencil and colorbuffer.
	glClear( GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT );

	/* Insert your drawing code here */
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
	/* Code to run when the application exit, perfect location to free everything. */
}
