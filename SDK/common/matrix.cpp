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

void vec3_multiply_mat4(vec3 &dst, const vec3 &v, const mat4 &m)
{
	dst->x = ( v->x * m[0][0] ) +
                 ( v->y * m[1][0] ) +
                 ( v->z * m[2][0] );

	dst->y = ( v->x * m[0][1] ) +
                 ( v->y * m[1][1] ) +
                 ( v->z * m[2][1] );

	dst->z = ( v->x * m[0][2] ) +
                 ( v->y * m[1][2] ) +
                 ( v->z * m[2][2] );
}


void mat3_copy_mat4( mat3 &dst, mat4 &m )
{
	memcpy( &dst[0], &m[0], sizeof( vec3 ) );
	memcpy( &dst[1], &m[1], sizeof( vec3 ) );
	memcpy( &dst[2], &m[2], sizeof( vec3 ) );
}


void mat4_translate( mat4 &dst, const mat4 &m, const vec3 &v )
{
	dst[3][0] = m[0][0] * v->x + m[1][0] * v->y + m[2][0] * v->z + m[3][0];
	dst[3][1] = m[0][1] * v->x + m[1][1] * v->y + m[2][1] * v->z + m[3][1];
	dst[3][2] = m[0][2] * v->x + m[1][2] * v->y + m[2][2] * v->z + m[3][2];
	dst[3][3] = m[0][3] * v->x + m[1][3] * v->y + m[2][3] * v->z + m[3][3];
}


void mat4_rotate( mat4 &dst, mat4 &m, const vec4 &v )
{
    float s = sinf( v->w * DEG_TO_RAD ),
    c = cosf( v->w * DEG_TO_RAD ),
    xx,
    yy,
    zz,
    xy,
    yz,
    zx,
    xs,
    ys,
    zs,
    c1;

    mat4 mat;

    vec3 t(v, true);

    mat.loadIdentity();

    if( !v->w || !t.length() ) return;
    t = t.normalize();

    xx = t->x * t->x;
    yy = t->y * t->y;
    zz = t->z * t->z;
    xy = t->x * t->y;
    yz = t->y * t->z;
    zx = t->z * t->x;
    xs = t->x * s;
    ys = t->y * s;
    zs = t->z * s;
    c1 = 1.0f - c;

    mat[0][0] = ( c1 * xx ) + c;
    mat[1][0] = ( c1 * xy ) - zs;
    mat[2][0] = ( c1 * zx ) + ys;

    mat[0][1] = ( c1 * xy ) + zs;
    mat[1][1] = ( c1 * yy ) + c;
    mat[2][1] = ( c1 * yz ) - xs;

    mat[0][2] = ( c1 * zx ) - ys;
    mat[1][2] = ( c1 * yz ) + xs;
    mat[2][2] = ( c1 * zz ) + c;
    
    m = mat * m;
}


void mat4_scale( mat4 &dst, const mat4 &m, const vec3 &v )
{
    dst[0][0] = m[0][0] * v->x;
    dst[0][1] = m[0][1] * v->x;
    dst[0][2] = m[0][2] * v->x;
    dst[0][3] = m[0][3] * v->x;

    dst[1][0] = m[1][0] * v->y;
    dst[1][1] = m[1][1] * v->y;
    dst[1][2] = m[1][2] * v->y;
    dst[1][3] = m[1][3] * v->y;

    dst[2][0] = m[2][0] * v->z;
    dst[2][1] = m[2][1] * v->z;
    dst[2][2] = m[2][2] * v->z;
    dst[2][3] = m[2][3] * v->z;
}


void mat4_ortho(mat4 &dst, float left, float right, float bottom, float top, float clip_start, float clip_end)
{
    mat4 mat;

    mat[0][0] = 2.0f / ( right - left  );
    mat[1][0] = 0.0f;
    mat[2][0] = 0.0f;
    mat[3][0] = -( right + left ) / ( right - left );

    mat[0][1] = 0.0f;
    mat[1][1] = 2.0f / ( top - bottom );
    mat[2][1] = 0.0f;
    mat[3][1] = -( top + bottom ) / ( top - bottom );

    mat[0][2] = 0.0f;
    mat[1][2] = 0.0f;
    mat[2][2] = -2.0f / ( clip_end - clip_start );
    mat[3][2] = -( clip_end + clip_start ) / ( clip_end - clip_start );

    mat[0][3] =
    mat[1][3] =
    mat[2][3] = 0.0f;
    mat[3][3] = 1.0f;
    
    dst = mat * dst;
}


void mat4_copy_mat3( mat4 &dst, mat3 &m )
{
    memcpy( &dst[0], &m[0], sizeof( vec3 ) );
    memcpy( &dst[1], &m[1], sizeof( vec3 ) );
    memcpy( &dst[2], &m[2], sizeof( vec3 ) );
}


void mat4_multiply_mat3( mat4 &dst, const mat4 &m0, const mat3 &m1 )
{
    mat3 mat;

    mat[0][0] = m0[0][0] * m1[0][0] + m0[1][0] * m1[0][1] + m0[2][0] * m1[0][2];
    mat[0][1] = m0[0][1] * m1[0][0] + m0[1][1] * m1[0][1] + m0[2][1] * m1[0][2];
    mat[0][2] = m0[0][2] * m1[0][0] + m0[1][2] * m1[0][1] + m0[2][2] * m1[0][2];

    mat[1][0] = m0[0][0] * m1[1][0] + m0[1][0] * m1[1][1] + m0[2][0] * m1[1][2];
    mat[1][1] = m0[0][1] * m1[1][0] + m0[1][1] * m1[1][1] + m0[2][1] * m1[1][2];
    mat[1][2] = m0[0][2] * m1[1][0] + m0[1][2] * m1[1][1] + m0[2][2] * m1[1][2];

    mat[2][0] = m0[0][0] * m1[2][0] + m0[1][0] * m1[2][1] + m0[2][0] * m1[2][2];
    mat[2][1] = m0[0][1] * m1[2][0] + m0[1][1] * m1[2][1] + m0[2][1] * m1[2][2];
    mat[2][2] = m0[0][2] * m1[2][0] + m0[1][2] * m1[2][1] + m0[2][2] * m1[2][2];
    
    mat4_copy_mat3( dst, mat );
}
