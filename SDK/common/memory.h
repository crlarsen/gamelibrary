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

#ifndef MEMORY_H
#define MEMORY_H


struct MEMORY {
	char			filename[MAX_PATH];
	
	unsigned int	size;
	
	unsigned int	position;

	unsigned char	*buffer;
public:
    MEMORY(const char *filename, const bool relative_path);
    ~MEMORY();
    unsigned int read(void *dst, unsigned int size);
    void insert(const char *str, const unsigned int position);
private:
    // When I start compiling with C++ 11, use "= delete" instead of
    // declaring these methods to be private.  Note that since these
    // are never used I don't need to implement their bodies.  There
    // are two situations where errors will be protected:
    //   - If anything outside of the MEMORY classes, or any of its
    //     friends, then the compiler will complain that the methods
    //     are private.
    //   - If any MEMORY method, or one of its friends, tries to call
    //     either of these methods the link editor will generate an
    //     error since their bodies are never implemented.
    MEMORY(const MEMORY &src);
    MEMORY &operator=(const MEMORY &rhs);
};

#endif
