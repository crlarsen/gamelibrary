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

#ifndef AUDIO_H
#define AUDIO_H


struct AUDIO {
	ALCdevice		*al_device;

	ALCcontext		*al_context;
	
	ov_callbacks	callbacks;
public:
    AUDIO();
    ~AUDIO();
    void error();
    void set_listener(vec3 &location, vec3 &direction, vec3 &up);
    size_t ogg_read(void *ptr, size_t size, size_t read, void *memory_ptr);
    int ogg_seek(void *memory_ptr, ogg_int64_t offset, int stride);
    long ogg_tell(void *memory_ptr);
    int ogg_close(void *memory_ptr);
};

#endif
