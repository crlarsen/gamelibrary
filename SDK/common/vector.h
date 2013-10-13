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


#ifndef VECTOR_H
#define VECTOR_H

void vec2_add( vec2 *dst, vec2 *v0, vec2 *v1 );

void vec2_diff( vec2 *dst, vec2 *v0, vec2 *v1 );


void vec3_add( vec3 *dst, vec3 *v0, vec3 *v1 );

void vec3_diff( vec3 *dst, vec3 *v0, vec3 *v1 );

void vec3_mul( vec3 *dst, vec3 *v0, vec3 *v1 );

float vec3_dot( vec3 *v );

float vec3_dot_vec3( vec3 *v0, vec3 *v1 );

float vec3_length( vec3 *v );

float vec3_normalize( vec3 *dst, vec3 *v );

void vec3_cross( vec3 *dst, vec3 *v0, vec3 *v1 );

float vec3_dist( vec3 *v0, vec3 *v1 );

void vec3_mid( vec3 *dst, vec3 *v0, vec3 *v1 );

void vec3_invert( vec3 *dst, vec3 *v );

void vec3_lerp( vec3 *dst, vec3 *v0, vec3 *v1, float t );

void vec3_lerp2( vec3 *dst, vec3 *v0, vec3 *v1, float t );

void vec3_rotate_vec4( vec3 *dst, vec3 *v0, vec4 *v1 );


void vec3_to_recast( vec3 *v );

void recast_to_vec3( vec3 *v );


void vec4_add( vec4 *dst, vec4 *v0, vec4 *v1 );

void vec4_diff( vec4 *dst, vec4 *v0, vec4 *v1 );

void vec4_build_w( vec4 *v );

float vec4_dot( vec4 *v );

float vec4_dot_vec4( vec4 *v0, vec4 *v1 );

float vec4_length( vec4 *v );

float vec4_normalize( vec4 *dst, vec4 *v );

void vec4_multiply_vec3( vec4 *dst, vec4 *v0, vec3 *v1 );

void vec4_multiply_vec4( vec4 *dst, vec4 *v0, vec4 *v1 );

void vec4_conjugate( vec4 *dst, vec4 *v );

void vec4_invert( vec4 *dst, vec4 *v );

void vec4_lerp( vec4 *dst, vec4 *v0, vec4 *v1, float t );

void vec4_slerp( vec4 *dst, vec4 *v0, vec4 *v1, float t );

#endif
