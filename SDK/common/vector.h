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

void vec3_lerp(vec3 &dst, const vec3 &v0, const vec3 &v1, const float t);

void vec3_lerp2(vec3 &dst, const vec3 &v0, const vec3 &v1, const float t);

void vec3_rotate_quat(vec3 &dst, const vec3 &v0, const quaternion &v1);


void vec3_to_recast( vec3 *v );

void recast_to_vec3( vec3 *v );


void quat_build_r(quaternion &v);

void quat_build_w(quaternion &v);

void quat_multiply_vec3(quaternion &dst, const quaternion &v0, const vec3 &v1);

void quat_multiply_quat(quaternion &dst, const quaternion &v0, const quaternion &v1);

void quat_lerp(quaternion &dst, const quaternion &v0, const quaternion &v1, const float t);

void quat_slerp(quaternion &dst, const quaternion &v0, const quaternion &v1, const float t);

#endif
