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

#ifndef TYPES_H
#define TYPES_H

#define MAX_CHAR	64

#define MAX_PATH	256

#define DEG_TO_RAD	M_PI / 180.0f

#define RAD_TO_DEG	90.0f / M_PI

#define BUFFER_OFFSET( x ) ( ( char * )NULL + x )

#define CLAMP( x, low, high ) ( ( x > high ) ? high : ( ( x < low ) ? low : x ) )


enum
{
	STOP  = 0,
	PLAY  = 1,
	PAUSE = 2
};


typedef struct
{
	float x;
	float y;

} vec2;


typedef struct
{
	float x;
	float y;
	float z;

} vec3;


typedef struct
{
	float x;
	float y;
	float z;
	float w;
	
} vec4;


typedef struct
{
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;	

} ucol4;


typedef struct
{
	float x;
	float y;
	float z;
	float w;
	
} quat;


typedef struct
{
	vec3 m[ 3 ];

} mat3;


typedef struct
{
	vec4 m[ 4 ];

} mat4;


typedef struct
{
	unsigned short vertex_index;
	
	unsigned short uv_index;

} vecind;


#endif
