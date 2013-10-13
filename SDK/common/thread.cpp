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


void *THREAD_run( void *ptr )
{
	THREAD *thread = ( THREAD * )ptr;

	struct sched_param param;

	pthread_setschedparam( thread->thread, SCHED_RR, &param );

	param.sched_priority = thread->priority;

	while( thread->state )
	{
		if( thread->state == PLAY && thread->threadcallback )
		{ thread->threadcallback( thread ); }
		
		usleep( thread->timeout * 1000 );
	}
	
	pthread_exit( NULL );
	
	return NULL;
}


THREAD *THREAD_create( THREADCALLBACK *threadcallback,
					   void			  *userdata,
					   int			   priority,
					   unsigned int	   timeout )
{
	THREAD *thread = ( THREAD * ) calloc( 1, sizeof( THREAD ) );

	thread->threadcallback = threadcallback;
	
	thread->priority = priority;
	thread->userdata = userdata;
	thread->timeout  = timeout;

	THREAD_pause( thread );

	thread->thread_hdl = pthread_create( &thread->thread,
										 NULL,
										 THREAD_run,
										 ( void * )thread );
	return thread;
}


THREAD *THREAD_free( THREAD *thread )
{
	THREAD_stop( thread );
	
	pthread_join( thread->thread, NULL );
	
	free( thread );
	return NULL;
}


void THREAD_set_callback( THREAD *thread, THREADCALLBACK *threadcallback )
{ thread->threadcallback = threadcallback; }


void THREAD_play( THREAD *thread )
{ thread->state = PLAY; }


void THREAD_pause( THREAD *thread )
{
	thread->state = PAUSE;
	
	usleep( thread->timeout * 1000 );
}


void THREAD_stop( THREAD *thread )
{
	thread->state = STOP;

	usleep( thread->timeout * 1000 );
}
