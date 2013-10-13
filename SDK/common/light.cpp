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


LIGHT *LIGHT_create_directional( char *name, vec4 *color, float rotx, float roty, float rotz )
{
	vec3 up_axis = { 0.0f, 0.0f, 1.0f };
	
	LIGHT *light = ( LIGHT * ) calloc( 1, sizeof( LIGHT ) );
	
	strcpy( light->name, name );

	memcpy( &light->color, color, sizeof( vec4 ) );

	light->type = 0;
	
	create_direction_vector( &light->direction, &up_axis, rotx, roty, rotz );	

	return light;
}


LIGHT *LIGHT_create_point( char *name, vec4 *color, vec3 *position )
{
	LIGHT *light = ( LIGHT * ) calloc( 1, sizeof( LIGHT ) );
	
	strcpy( light->name, name );

	memcpy( &light->color, color, sizeof( vec4 ) );

	memcpy( &light->position, position, sizeof( vec3 ) );
	light->position.w = 1.0f;

	light->type = 1;

	return light;
}


LIGHT *LIGHT_create_point_with_attenuation( char *name, vec4 *color, vec3 *position, float distance, float linear_attenuation, float quadratic_attenuation )
{
	LIGHT *light = LIGHT_create_point( name, color, position );
		
	light->distance = distance * 2.0f;

	light->linear_attenuation = linear_attenuation;
	
	light->quadratic_attenuation = quadratic_attenuation;

	light->type = 2;

	return light;
}


LIGHT *LIGHT_create_point_sphere( char *name, vec4 *color, vec3 *position, float distance )
{
	LIGHT *light = LIGHT_create_point( name, color, position );

	light->distance = distance;

	light->type = 3;
	
	return light;
}


LIGHT *LIGHT_create_spot( char *name, vec4 *color, vec3 *position, float rotx, float roty, float rotz, float fov, float spot_blend ) {
	
	static vec3 up_axis = { 0.0f, 0.0f, 1.0f };

	LIGHT *light = ( LIGHT * ) calloc( 1, sizeof( LIGHT ) );

	strcpy( light->name, name );

	memcpy( &light->color, color, sizeof( vec4 ) );

	light->spot_fov = fov;

	light->spot_cos_cutoff = cosf( ( fov * 0.5f ) * DEG_TO_RAD );

	light->spot_blend = CLAMP( spot_blend, 0.001, 1.0f );

	memcpy( &light->position, position, sizeof( vec3 ) );
	light->position.w = 1.0f;

	light->type = 4;

	create_direction_vector( &light->spot_direction,
							 &up_axis,
							 rotx,
							 roty,
							 rotz );
	return light;
}


void LIGHT_get_direction_in_object_space( LIGHT *light, mat4 *m, vec3 *direction )
{
	mat4 invert;
	
	mat4_copy_mat4( &invert, m );
	
	mat4_invert( &invert );
	
	vec3_multiply_mat4( direction,
						&light->spot_direction,
						m );
						
	vec3_normalize( direction,
					direction );
	
	vec3_invert( direction,
				 direction );
}


void LIGHT_get_direction_in_eye_space( LIGHT *light, mat4 *m, vec3 *direction )
{
	vec3_multiply_mat4( direction,
						&light->direction,
						m );
		
	vec3_invert( direction, direction );
}



void LIGHT_get_position_in_eye_space( LIGHT *light, mat4 *m, vec4 *position )
{
	vec4_multiply_mat4( position,
						&light->position,
						m );
}


LIGHT *LIGHT_free( LIGHT *light )
{
	free( light );
	return NULL;
}
