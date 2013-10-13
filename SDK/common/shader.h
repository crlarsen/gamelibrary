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

#ifndef SHADER_H
#define SHADER_H

typedef struct
{
	char			name[ MAX_CHAR ];
	
	unsigned int	type;

	unsigned int	sid;
	
} SHADER;


SHADER *SHADER_init( char *name, unsigned int type );

SHADER *SHADER_free( SHADER *shader );

unsigned char SHADER_compile( SHADER *shader, const char *code, unsigned char debug );

void SHADER_delete_id( SHADER *shader );

#endif
