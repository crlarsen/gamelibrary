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


static void *THREAD_run(void *ptr)
{
    THREAD *thread = (THREAD *)ptr;

    struct sched_param param;

    pthread_setschedparam(thread->thread, SCHED_RR, &param);

    param.sched_priority = thread->priority;

    while (thread->state) {
        if (thread->state == PLAY && thread->threadcallback)
            thread->threadcallback(thread);

        usleep(thread->timeout * 1000);
    }
    
    pthread_exit(NULL);
    
    return NULL;
}


THREAD::THREAD(THREADCALLBACK   *threadcallback,
               void		*userdata,
               int		priority,
               unsigned int	timeout) :
    state(STOP), priority(priority), timeout(timeout),
    threadcallback(threadcallback), userdata(userdata)
{
    this->pause();

    this->thread_hdl = pthread_create(&this->thread,
                                      NULL,
                                      THREAD_run,
                                      (void *)this);
}


THREAD::~THREAD()
{
    this->stop();

    pthread_join(this->thread, NULL);
}


void THREAD::set_callback(THREADCALLBACK *threadcallback)
{
    this->threadcallback = threadcallback;
}


void THREAD::play()
{
    this->state = PLAY;
}


void THREAD::pause()
{
    this->state = PAUSE;

    usleep(this->timeout * 1000);
}


void THREAD::stop()
{
    this->state = STOP;

    usleep(this->timeout * 1000);
}
