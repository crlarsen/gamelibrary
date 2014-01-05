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


void vec3_rotate_quat(vec3 &dst, const vec3 &v0, const quaternion &v1)
{
    float   w(-v1.v.dotProduct(v0));
    vec3    v(v1.w*v0 + v1.v.crossProduct(v0));

    dst = v*v1.w - w*v1.v + v1.v.crossProduct(v);
}


void vec3_to_recast( vec3 *v )
{
    vec3 tmp((*v)[0], (*v)[2], -(*v)[1]);

    *v = tmp;
}


void recast_to_vec3( vec3 *v )
{
    vec3    tmp((*v)[0], -(*v)[2], (*v)[1]);

    *v = tmp;
}


void quat_build_r(quaternion &q)
{
    float l = 1.0f - (q->i * q->i) -
                     (q->j * q->j) -
                     (q->k * q->k);

    q->r = ( l < 0.0f ) ? 0.0f : -sqrtf( l );
}


void quat_build_w(quaternion &v)
{
    quat_build_r(v);
}
