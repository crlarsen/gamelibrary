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

#ifndef UTILS_H
#define UTILS_H

unsigned int get_micro_time( void );

unsigned int get_milli_time( void );

void adjust_file_path( char *filepath );

void get_file_path( char *filepath, char *path );

void get_file_name( char *filepath, char *name );

void get_file_extension( char *filepath, char *ext, unsigned char uppercase );

void generate_color_from_index( unsigned int index, vec4 *color );

void console_print( const char *str, ... );

void build_frustum( vec4 frustum[ 6 ], mat4 *modelview_matrix, mat4 *projection_matrix );

float sphere_distance_in_frustum( vec4 *frustum, vec3  *location, float radius );

unsigned char point_in_frustum( vec4 *frustum, vec3 *location );

unsigned char box_in_frustum( vec4 *frustum, vec3 *location, vec3 *dimension );

unsigned char sphere_intersect_frustum( vec4 *frustum, vec3 *location, float radius );

unsigned char box_intersect_frustum( vec4 *frustum, vec3 *location, vec3 *dimension );

unsigned int get_next_pow2( unsigned int size );

unsigned int get_nearest_pow2( unsigned int size );

void create_direction_vector( vec3 *dst, vec3 *up_axis, float rotx, float roty, float rotz );

#endif
