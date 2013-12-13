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


void vec4_multiply_mat4(vec4 &dst, const vec4 &v, const mat4 &m)
{
	dst->x = ( v->x * m[0][0] ) +
                 ( v->y * m[1][0] ) +
                 ( v->z * m[2][0] ) +
                 ( v->w * m[3][0] );

	dst->y = ( v->x * m[0][1] ) +
                 ( v->y * m[1][1] ) +
                 ( v->z * m[2][1] ) +
                 ( v->w * m[3][1] );

	dst->z = ( v->x * m[0][2] ) +
                 ( v->y * m[1][2] ) +
                 ( v->z * m[2][2] ) +
                 ( v->w * m[3][2] );

	dst->w = ( v->x * m[0][3] ) +
                 ( v->y * m[1][3] ) +
                 ( v->z * m[2][3] ) +
                 ( v->w * m[3][3] );
}



void mat3_identity( mat3 *m )
{
	memset( &m[0], 0, sizeof( mat3 ) );
	
	(*m)[0][0] =
	(*m)[1][1] =
	(*m)[2][2] = 1.0f;

}


void mat3_copy_mat4( mat3 &dst, mat4 &m )
{
	memcpy( &dst[0], &m[0], sizeof( vec3 ) );
	memcpy( &dst[1], &m[1], sizeof( vec3 ) );
	memcpy( &dst[2], &m[2], sizeof( vec3 ) );
}


void mat4_identity( mat4 &m )
{
	memset( &m[0], 0, sizeof( mat4 ) );
	
	m[0][0] =
	m[1][1] =
	m[2][2] =
	m[3][3] = 1.0f;
}


void mat4_copy_mat4( mat4 &dst, const mat4 &m )
{ memcpy( &dst, &m, sizeof( mat4 ) ); }


void mat4_translate( mat4 &dst, const mat4 &m, const vec3 &v )
{
	dst[3][0] = m[0][0] * v[0] + m[1][0] * v[1] + m[2][0] * v[2] + m[3][0];
	dst[3][1] = m[0][1] * v[0] + m[1][1] * v[1] + m[2][1] * v[2] + m[3][1];
	dst[3][2] = m[0][2] * v[0] + m[1][2] * v[1] + m[2][2] * v[2] + m[3][2];
	dst[3][3] = m[0][3] * v[0] + m[1][3] * v[1] + m[2][3] * v[2] + m[3][3];
}


void mat4_rotate_fast( mat4 &m, const vec4 &v )
{
    float s = sinf( v[3] * DEG_TO_RAD ),
    c = cosf( v[3] * DEG_TO_RAD );

    mat4 mat;

    mat4_identity( mat );

    if( !v[0] )
    {
        if( !v[1] )
        {
            if( v[2] )
            {
                mat[0][0] = c;
                mat[1][1] = c;

                if( v[2] < 0.0f )
                {
                    mat[1][0] =  s;
                    mat[0][1] = -s;
                }
                else
                {
                    mat[1][0] = -s;
                    mat[0][1] =  s;
                }
            }
        }
        else if( !v[2] )
        {
            mat[0][0] = c;
            mat[2][2] = c;

            if( v[1] < 0.0f )
            {
                mat[2][0] = -s;
                mat[0][2] =  s;
            }
            else
            {
                mat[2][0] =  s;
                mat[0][2] = -s;
            }
        }
    }
    else if( !v[1] )
    {
        if( !v[2] )
        {
            mat[1][1] = c;
            mat[2][2] = c;
            
            if( v[0] < 0.0f )
            {
                mat[2][1] =  s;
                mat[1][2] = -s;
            }
            else
            {
                mat[2][1] = -s;
                mat[1][2] =  s;
            }
        }
    }
    
    mat4_multiply_mat4( m, m, mat );
}


void mat4_rotate( mat4 &dst, mat4 &m, const vec4 &v )
{
    float s = sinf( v[3] * DEG_TO_RAD ),
    c = cosf( v[3] * DEG_TO_RAD ),
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

    vec3 t(v[0], v[1], v[2]);

    mat4_identity( mat );

    if( !v[3] || !t.length() ) return;
    t = t.normalize();

    xx = t[0] * t[0];
    yy = t[1] * t[1];
    zz = t[2] * t[2];
    xy = t[0] * t[1];
    yz = t[1] * t[2];
    zx = t[2] * t[0];
    xs = t[0] * s;
    ys = t[1] * s;
    zs = t[2] * s;
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
    
    mat4_multiply_mat4( m, m, mat );	
}


void mat4_scale( mat4 &dst, const mat4 &m, const vec3 &v )
{
    dst[0][0] = m[0][0] * v[0];
    dst[0][1] = m[0][1] * v[0];
    dst[0][2] = m[0][2] * v[0];
    dst[0][3] = m[0][3] * v[0];

    dst[1][0] = m[1][0] * v[1];
    dst[1][1] = m[1][1] * v[1];
    dst[1][2] = m[1][2] * v[1];
    dst[1][3] = m[1][3] * v[1];

    dst[2][0] = m[2][0] * v[2];
    dst[2][1] = m[2][1] * v[2];
    dst[2][2] = m[2][2] * v[2];
    dst[2][3] = m[2][3] * v[2];
}


unsigned char mat4_invert( mat4 &m )
{
    mat4 mat;

    float d = ( m[0][0] * m[0][0] +
               m[1][0] * m[1][0] +
               m[2][0] * m[2][0] );

    if( !d ) return 0;

    d = 1.0f / d;

    mat[0][0] = d * m[0][0];
    mat[0][1] = d * m[1][0];
    mat[0][2] = d * m[2][0];

    mat[1][0] = d * m[0][1];
    mat[1][1] = d * m[1][1];
    mat[1][2] = d * m[2][1];

    mat[2][0] = d * m[0][2];
    mat[2][1] = d * m[1][2];
    mat[2][2] = d * m[2][2];

    mat[3][0] = -( mat[0][0] * m[3][0] + mat[1][0] * m[3][1] + mat[2][0] * m[3][2] );
    mat[3][1] = -( mat[0][1] * m[3][0] + mat[1][1] * m[3][1] + mat[2][1] * m[3][2] );
    mat[3][2] = -( mat[0][2] * m[3][0] + mat[1][2] * m[3][1] + mat[2][2] * m[3][2] );

    mat[0][3] =
    mat[1][3] = 
    mat[2][3] = 0.0f;
    mat[3][3] = 1.0f;
    
    mat4_copy_mat4( m, mat );
    
    return 1;
}


unsigned char mat4_invert_full( mat4 &m )
{
    mat4 inv;

    float d;

    inv[0][0] = m[1][1] * m[2][2] * m[3][3] -
    m[1][1] * m[2][3] * m[3][2] -
    m[2][1] * m[1][2] * m[3][3] +
    m[2][1] * m[1][3] * m[3][2] +
    m[3][1] * m[1][2] * m[2][3] -
    m[3][1] * m[1][3] * m[2][2];

    inv[1][0] = -m[1][0] * m[2][2] * m[3][3] +
    m[1][0] * m[2][3] * m[3][2] +
    m[2][0] * m[1][2] * m[3][3] -
    m[2][0] * m[1][3] * m[3][2] -
    m[3][0] * m[1][2] * m[2][3] +
    m[3][0] * m[1][3] * m[2][2];

    inv[2][0] = m[1][0] * m[2][1] * m[3][3] -
    m[1][0] * m[2][3] * m[3][1] -
    m[2][0] * m[1][1] * m[3][3] +
    m[2][0] * m[1][3] * m[3][1] +
    m[3][0] * m[1][1] * m[2][3] -
    m[3][0] * m[1][3] * m[2][1];

    inv[3][0] = -m[1][0] * m[2][1] * m[3][2] +
    m[1][0] * m[2][2] * m[3][1] +
    m[2][0] * m[1][1] * m[3][2] -
    m[2][0] * m[1][2] * m[3][1] -
    m[3][0] * m[1][1] * m[2][2] +
    m[3][0] * m[1][2] * m[2][1];

    inv[0][1] = -m[0][1] * m[2][2] * m[3][3] +
    m[0][1] * m[2][3] * m[3][2] +
    m[2][1] * m[0][2] * m[3][3] -
    m[2][1] * m[0][3] * m[3][2] -
    m[3][1] * m[0][2] * m[2][3] +
    m[3][1] * m[0][3] * m[2][2];

    inv[1][1] = m[0][0] * m[2][2] * m[3][3] -
    m[0][0] * m[2][3] * m[3][2] -
    m[2][0] * m[0][2] * m[3][3] +
    m[2][0] * m[0][3] * m[3][2] +
    m[3][0] * m[0][2] * m[2][3] -
    m[3][0] * m[0][3] * m[2][2];

    inv[2][1] = -m[0][0] * m[2][1] * m[3][3] +
    m[0][0] * m[2][3] * m[3][1] +
    m[2][0] * m[0][1] * m[3][3] -
    m[2][0] * m[0][3] * m[3][1] -
    m[3][0] * m[0][1] * m[2][3] +
    m[3][0] * m[0][3] * m[2][1];

    inv[3][1] = m[0][0] * m[2][1] * m[3][2] -
    m[0][0] * m[2][2] * m[3][1] -
    m[2][0] * m[0][1] * m[3][2] +
    m[2][0] * m[0][2] * m[3][1] +
    m[3][0] * m[0][1] * m[2][2] -
    m[3][0] * m[0][2] * m[2][1];

    inv[0][2] = m[0][1] * m[1][2] * m[3][3] -
    m[0][1] * m[1][3] * m[3][2] -
    m[1][1] * m[0][2] * m[3][3] +
    m[1][1] * m[0][3] * m[3][2] +
    m[3][1] * m[0][2] * m[1][3] -
    m[3][1] * m[0][3] * m[1][2];

    inv[1][2] = -m[0][0] * m[1][2] * m[3][3] +
    m[0][0] * m[1][3] * m[3][2] +
    m[1][0] * m[0][2] * m[3][3] -
    m[1][0] * m[0][3] * m[3][2] -
    m[3][0] * m[0][2] * m[1][3] +
    m[3][0] * m[0][3] * m[1][2];

    inv[2][2] = m[0][0] * m[1][1] * m[3][3] -
    m[0][0] * m[1][3] * m[3][1] -
    m[1][0] * m[0][1] * m[3][3] +
    m[1][0] * m[0][3] * m[3][1] +
    m[3][0] * m[0][1] * m[1][3] -
    m[3][0] * m[0][3] * m[1][1];

    inv[3][2] = -m[0][0] * m[1][1] * m[3][2] +
    m[0][0] * m[1][2] * m[3][1] +
    m[1][0] * m[0][1] * m[3][2] -
    m[1][0] * m[0][2] * m[3][1] -
    m[3][0] * m[0][1] * m[1][2] +
    m[3][0] * m[0][2] * m[1][1];

    inv[0][3] = -m[0][1] * m[1][2] * m[2][3] +
    m[0][1] * m[1][3] * m[2][2] +
    m[1][1] * m[0][2] * m[2][3] -
    m[1][1] * m[0][3] * m[2][2] -
    m[2][1] * m[0][2] * m[1][3] +
    m[2][1] * m[0][3] * m[1][2];

    inv[1][3] = m[0][0] * m[1][2] * m[2][3] -
    m[0][0] * m[1][3] * m[2][2] -
    m[1][0] * m[0][2] * m[2][3] +
    m[1][0] * m[0][3] * m[2][2] +
    m[2][0] * m[0][2] * m[1][3] -
    m[2][0] * m[0][3] * m[1][2];

    inv[2][3] = -m[0][0] * m[1][1] * m[2][3] +
    m[0][0] * m[1][3] * m[2][1] +
    m[1][0] * m[0][1] * m[2][3] -
    m[1][0] * m[0][3] * m[2][1] -
    m[2][0] * m[0][1] * m[1][3] +
    m[2][0] * m[0][3] * m[1][1];

    inv[3][3] = m[0][0] * m[1][1] * m[2][2] -
    m[0][0] * m[1][2] * m[2][1] -
    m[1][0] * m[0][1] * m[2][2] +
    m[1][0] * m[0][2] * m[2][1] +
    m[2][0] * m[0][1] * m[1][2] -
    m[2][0] * m[0][2] * m[1][1];

    d = m[0][0] * inv[0][0] +
    m[0][1] * inv[1][0] +
    m[0][2] * inv[2][0] +
    m[0][3] * inv[3][0];

    if( !d ) return 0;

    d = 1.0f / d;

    inv[0][0] *= d;
    inv[0][1] *= d;
    inv[0][2] *= d;
    inv[0][3] *= d;

    inv[1][0] *= d;
    inv[1][1] *= d;
    inv[1][2] *= d;
    inv[1][3] *= d;

    inv[2][0] *= d;
    inv[2][1] *= d;
    inv[2][2] *= d;
    inv[2][3] *= d;

    inv[3][0] *= d;
    inv[3][1] *= d;
    inv[3][2] *= d;
    inv[3][3] *= d;

    mat4_copy_mat4( m, inv );

    return 1;
}


void mat4_transpose( mat4 &m )
{
    float t;

    t			= m[0][1];
    m[0][1] = m[1][0];
    m[1][0] = t;

    t			= m[0][2];
    m[0][2] = m[2][0];
    m[2][0] = t;

    t			= m[0][3];
    m[0][3] = m[3][0];
    m[3][0] = t;

    t			= m[1][2];
    m[1][2] = m[2][1];
    m[2][1] = t;

    t			= m[1][3] ;
    m[1][3] = m[3][1] ;
    m[3][1] = t;
    
    t			= m[2][3] ; 
    m[2][3] = m[3][2] ; 
    m[3][2] = t;
}


void mat4_ortho( mat4 *dst, float left, float right, float bottom, float top, float clip_start, float clip_end )
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
    
    mat4_multiply_mat4( *dst, *dst, mat );
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


void mat4_multiply_mat4( mat4 &dst, const mat4 &m0, const mat4 &m1 )
{
    mat4 mat;

    mat[0][0] = m0[0][0] * m1[0][0] + m0[1][0] * m1[0][1] + m0[2][0] * m1[0][2] + m0[3][0] * m1[0][3];
    mat[0][1] = m0[0][1] * m1[0][0] + m0[1][1] * m1[0][1] + m0[2][1] * m1[0][2] + m0[3][1] * m1[0][3];
    mat[0][2] = m0[0][2] * m1[0][0] + m0[1][2] * m1[0][1] + m0[2][2] * m1[0][2] + m0[3][2] * m1[0][3];
    mat[0][3] = m0[0][3] * m1[0][0] + m0[1][3] * m1[0][1] + m0[2][3] * m1[0][2] + m0[3][3] * m1[0][3];

    mat[1][0] = m0[0][0] * m1[1][0] + m0[1][0] * m1[1][1] + m0[2][0] * m1[1][2] + m0[3][0] * m1[1][3];
    mat[1][1] = m0[0][1] * m1[1][0] + m0[1][1] * m1[1][1] + m0[2][1] * m1[1][2] + m0[3][1] * m1[1][3];
    mat[1][2] = m0[0][2] * m1[1][0] + m0[1][2] * m1[1][1] + m0[2][2] * m1[1][2] + m0[3][2] * m1[1][3];
    mat[1][3] = m0[0][3] * m1[1][0] + m0[1][3] * m1[1][1] + m0[2][3] * m1[1][2] + m0[3][3] * m1[1][3];

    mat[2][0] = m0[0][0] * m1[2][0] + m0[1][0] * m1[2][1] + m0[2][0] * m1[2][2] + m0[3][0] * m1[2][3];
    mat[2][1] = m0[0][1] * m1[2][0] + m0[1][1] * m1[2][1] + m0[2][1] * m1[2][2] + m0[3][1] * m1[2][3];
    mat[2][2] = m0[0][2] * m1[2][0] + m0[1][2] * m1[2][1] + m0[2][2] * m1[2][2] + m0[3][2] * m1[2][3];
    mat[2][3] = m0[0][3] * m1[2][0] + m0[1][3] * m1[2][1] + m0[2][3] * m1[2][2] + m0[3][3] * m1[2][3];

    mat[3][0] = m0[0][0] * m1[3][0] + m0[1][0] * m1[3][1] + m0[2][0] * m1[3][2] + m0[3][0] * m1[3][3];
    mat[3][1] = m0[0][1] * m1[3][0] + m0[1][1] * m1[3][1] + m0[2][1] * m1[3][2] + m0[3][1] * m1[3][3];
    mat[3][2] = m0[0][2] * m1[3][0] + m0[1][2] * m1[3][1] + m0[2][2] * m1[3][2] + m0[3][2] * m1[3][3];
    mat[3][3] = m0[0][3] * m1[3][0] + m0[1][3] * m1[3][1] + m0[2][3] * m1[3][2] + m0[3][3] * m1[3][3];
    
    mat4_copy_mat4( dst, mat );
}
