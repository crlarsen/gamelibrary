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

#ifndef MEMORY_H
#define MEMORY_H


typedef struct
{
	char			filename[ MAX_PATH ];
	
	unsigned int	size;
	
	unsigned int	position;

	unsigned char	*buffer;

} MEMORY;


MEMORY *mopen( char *filename, unsigned char relative_path );

MEMORY *mclose( MEMORY *memory );

unsigned int mread( MEMORY *memory, void *dst, unsigned int size );

void minsert( MEMORY *memory, char *str, unsigned int position );

#endif
