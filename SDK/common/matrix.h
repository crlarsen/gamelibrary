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

#ifndef MATRIX_H
#define MATRIX_H

void vec3_multiply_mat3( vec3 *dst, vec3 *v, mat3 *m );

void vec3_multiply_mat4( vec3 *dst, vec3 *v, mat4 *m );

void vec4_multiply_mat4( vec4 *dst, vec4 *v, mat4 *m );

void mat3_identity( mat3 *m );

void mat3_copy_mat4( mat3 *dst, mat4 *m );

void mat4_identity( mat4 *m );

void mat4_copy_mat4( mat4 *dst, mat4 *m );

void mat4_translate( mat4 *dst, mat4 *m, vec3 *v );

void mat4_rotate_fast( mat4 *m, vec4 *v );

void mat4_rotate( mat4 *dst, mat4 *m, vec4 *v );

void mat4_scale( mat4 *dst, mat4 *m, vec3 *v );

unsigned char mat4_invert( mat4 *m );

unsigned char mat4_invert_full( mat4 *m );

void mat4_transpose( mat4 *m );

void mat4_ortho( mat4 *dst, float left, float right, float bottom, float top, float clip_start, float clip_end );

void mat4_copy_mat3( mat4 *dst, mat3 *m );

void mat4_multiply_mat3( mat4 *dst, mat4 *m0, mat3 *m1 );

void mat4_multiply_mat4( mat4 *dst, mat4 *m0, mat4 *m1 );

#endif
