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


#ifndef LIGHT_H
#define LIGHT_H

enum
{
	LIGHT_DIRECTIONAL			 = 0,
	LIGHT_POINT					 = 1,
	LIGHT_POINT_WITH_ATTENUATION = 2,
	LIGHT_POINT_SPHERE			 = 3,
	LIGHT_SPOT					 = 4
};


typedef struct
{
	char	name[ MAX_CHAR ];

	vec4	color;

	vec3	direction;
	
	vec4	position;

	float	linear_attenuation;
	
	float	quadratic_attenuation;
	
	float	distance;
	
	float	spot_fov;

	float	spot_cos_cutoff;

	float	spot_blend;
	
	vec3	spot_direction;	

	unsigned char type;
	
} LIGHT;


LIGHT *LIGHT_create_directional( char *name, vec4 *color, float rotx, float roty, float rotz );

LIGHT *LIGHT_create_point( char *name, vec4 *color, vec3 *position );

LIGHT *LIGHT_create_point_with_attenuation( char *name, vec4 *color, vec3 *position, float distance, float linear_attenuation, float quadratic_attenuation );

LIGHT *LIGHT_create_point_sphere( char *name, vec4 *color, vec3 *position, float distance );

LIGHT *LIGHT_create_spot( char *name, vec4 *color, vec3 *position, float rotx, float roty, float rotz, float fov, float spot_blend );

void LIGHT_get_direction_in_object_space( LIGHT *light, mat4 *m, vec3 *direction );

void LIGHT_get_direction_in_eye_space( LIGHT *light, mat4 *m, vec3 *direction );

void LIGHT_get_position_in_eye_space( LIGHT *light, mat4 *m, vec4 *position );

LIGHT *LIGHT_free( LIGHT *light );

#endif
