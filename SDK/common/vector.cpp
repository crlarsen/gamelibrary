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


void vec3_lerp(vec3 &dst, const vec3 &v0, const vec3 &v1, const float t)
{
    if( t == 1.0f )
    {
        dst[0] = v1[0];
        dst[1] = v1[1];
        dst[2] = v1[2];

        return;
    }
    else if( t == 0.0f )
    {
        dst[0] = v0[0];
        dst[1] = v0[1];
        dst[2] = v0[2];

        return;
    }

    dst[0] = v0[0] + t * ( v1[0] - v0[0] );
    dst[1] = v0[1] + t * ( v1[1] - v0[1] );
    dst[2] = v0[2] + t * ( v1[2] - v0[2] );
}


void vec3_lerp2( vec3 *dst, vec3 *v0, vec3 *v1, float t )
{
	(*dst)[0] = (*v0)[0] * t + (*v1)[0] * ( 1.0f - t );
	(*dst)[1] = (*v0)[1] * t + (*v1)[1] * ( 1.0f - t );
	(*dst)[2] = (*v0)[2] * t + (*v1)[2] * ( 1.0f - t );
}


void vec3_rotate_quat(vec3 &dst, const vec3 &v0, const quaternion &v1)
{
    quaternion i, t, f;

    i = v1.conjugate();

    i.safeNormalize();

    quat_multiply_vec3(t, v1, v0);

    quat_multiply_quat(f, t, i);

    dst = f.v;
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


void quat_build_r(quaternion &v)
{
    float l = 1.0f - ( v->i * v->i ) -
                     ( v->j * v->j ) -
                     ( v->k * v->k );

    v->r = ( l < 0.0f ) ? 0.0f : -sqrtf( l );
}


void quat_multiply_vec3(quaternion &dst, const quaternion &v0, const vec3 &v1)
{
    quaternion v;

    v->i =  ( v0->r * v1[0] ) + ( v0->j * v1[2] ) - ( v0->k * v1[1] );
    v->j =  ( v0->r * v1[1] ) + ( v0->k * v1[0] ) - ( v0->i * v1[2] );
    v->k =  ( v0->r * v1[2] ) + ( v0->i * v1[1] ) - ( v0->j * v1[0] );
    v->r = -( v0->i * v1[0] ) - ( v0->j * v1[1] ) - ( v0->k * v1[2] );

    dst = v;
}



void quat_multiply_quat(quaternion &dst, const quaternion &v0, const quaternion &v1)
{
    quaternion v;

    v->i = (v0->i * v1->r) + (v0->r * v1->i) + (v0->j * v1->k) - (v0->k * v1->j);
    v->j = (v0->j * v1->r) + (v0->r * v1->j) + (v0->k * v1->i) - (v0->i * v1->k);
    v->k = (v0->k * v1->r) + (v0->r * v1->k) + (v0->i * v1->j) - (v0->j * v1->i);
    v->r = (v0->r * v1->r) - (v0->i * v1->i) - (v0->j * v1->j) - (v0->k * v1->k);

    dst = v;
}


void quat_lerp(quaternion &dst, const quaternion &v0, const quaternion &v1, const float t)
{
	float dot = v0.dotProduct(v1),
		  k0,
		  k1;

	quaternion tmp(v1);
	
	if( t == 1.0f )
	{
            dst = v1;
		
		return;
	}
	else if( t == 0.0f )
	{
            dst = v0;
	
		return;
	}
	
	
	if( dot < 0.0f )
	{
            tmp = -tmp;
		
		dot = -dot;
	}


	if( dot > 0.999999f )
	{
		k0 = 1.0f - t;
		k1 = t;
	}
	else
	{
		float s  = sqrtf( 1.0f - ( dot * dot ) ),
			  o  = atan2f( s, dot ),
			  o1 = 1.0f / s;

		k0 = sinf( ( 1.0f - t ) * o ) * o1;
		k1 = sinf( t * o ) * o1;
	}
	
	dst->i = ( k0 * v0->i ) + ( k1 * tmp->i );
	dst->j = ( k0 * v0->j ) + ( k1 * tmp->j );
	dst->k = ( k0 * v0->k ) + ( k1 * tmp->k );		
	dst->r = ( k0 * v0->r ) + ( k1 * tmp->r );
}


void quat_slerp(quaternion &dst, const quaternion &v0, const quaternion &v1, const float t)
{
    float c = v0.dotProduct(v1),
    k0,
    k1;

    quaternion tmp(v1);

    if( t == 1.0f ) {
        dst = v1;

        return;
    } else if( t == 0.0f ) {
        dst = v0;

        return;
    }


    if( c < 0.0f ) {
        tmp = -tmp;

        c = -c;
    }


    if( c > 0.999999f ) {
        k0 = 1.0f - t;
        k1 = t;
    } else {
        float   s  = sqrtf( 1.0f - ( c * c ) ),
                o  = atan2f( s, c ),
                o1 = 1.0f / s;

        k0 = sinf( ( 1.0f - t ) * o ) * o1;
        k1 = sinf( t * o ) * o1;
    }

    dst->i = ( k0 * v0->i ) + ( k1 * tmp->i );
    dst->j = ( k0 * v0->j ) + ( k1 * tmp->j );
    dst->k = ( k0 * v0->k ) + ( k1 * tmp->k );		
    dst->r = ( k0 * v0->r ) + ( k1 * tmp->r );
}
