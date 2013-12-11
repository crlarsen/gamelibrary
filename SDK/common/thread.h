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

#ifndef THREAD_H
#define THREAD_H


enum
{
    THREAD_PRIORITY_VERY_LOW  = 15,
    THREAD_PRIORITY_LOW       = 23,
    THREAD_PRIORITY_NORMAL    = 31,
    THREAD_PRIORITY_HIGH      = 39,
    THREAD_PRIORITY_VERY_HIGH = 47
};


typedef void(THREADCALLBACK(void *));


struct THREAD {
    AnimState           state;

    int			priority;

    unsigned int	timeout;

    pthread_t		thread;

    unsigned int	thread_hdl;

    THREADCALLBACK	*threadcallback;
    
    void		*userdata;
public:
    THREAD(THREADCALLBACK *threadcallback, void	*userdata, int priority, unsigned int timeout);
    ~THREAD();
    void set_callback(THREADCALLBACK *threadcallback);
    void play();
    void pause();
    void stop();
};

#endif
