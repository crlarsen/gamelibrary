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
/*
 * Source code modified by Chris Larsen to make the following data types into
 * proper C++ classes:
 * - AUDIO
 * - FONT
 * - GFX
 * - LIGHT
 * - MD5
 * - MEMORY
 * - NAVIGATION
 * - OBJ
 * - OBJMATERIAL
 * - OBJMESH
 * - OBJTRIANGLEINDEX
 * - OBJTRIANGLELIST
 * - OBJVERTEXDATA
 * - PROGRAM
 * - SHADER
 * - SOUND
 * - TEXTURE
 * - THREAD
 */

#include "gfx.h"

unsigned int get_micro_time( void )
{
    struct timeval tv;

    gettimeofday( &tv, NULL );

    return ( tv.tv_sec * 1000000 ) + tv.tv_usec;
}


unsigned int get_milli_time( void )
{
    struct timeval tv;

    gettimeofday( &tv, NULL );

    return tv.tv_usec / 1000 + tv.tv_sec * 1000;
}


void adjust_file_path( char *filepath )
{
    for (int i=0, l=strlen(filepath); i!=l; ++i)
        if (filepath[i] == '\\')
            filepath[i] = '/';
}


void get_file_path( char *filepath, char *path )
{
    char *t = NULL;

    unsigned int p;

    adjust_file_path( filepath );

    t = strrchr( filepath, '/' );

    if (t) {
        p = ( t - filepath ) + 1;
        strncpy( path, filepath, p );
        path[p] = 0;
    }
}


void get_file_name( char *filepath, char *name )
{
    char *t = NULL;

    adjust_file_path( filepath );

    t = strrchr( filepath, '/' );

    if ( t )
        strcpy( name, t + 1 );
    else
        strcpy( name, filepath );
}


void get_file_extension(char *filepath, char *ext, const bool uppercase)
{
    char *t = NULL;

    adjust_file_path(filepath);

    t = strrchr(filepath, '.');

    if (t) strcpy( ext, t + 1 );

    if (uppercase) {
        for (int i=0, l=strlen(ext); i!=l; ++i)
            ext[i] = toupper(ext[i]);
    }
}


void generate_color_from_index( unsigned int index, vec4 *color )
{
    ucol4 tmp_color = { 0, 0, 0, 255 };

    unsigned int i = index >> 16;

    memcpy( &tmp_color.r, &i, 1 );

    i = index >> 8;

    memcpy( &tmp_color.g, &i, 1 );

    memcpy( &tmp_color.b, &index, 1 );

    (*color)->x = tmp_color.r / 255.0f;
    (*color)->y = tmp_color.g / 255.0f;
    (*color)->z = tmp_color.b / 255.0f;
    (*color)->w = tmp_color.a / 255.0f;
}


void console_print( const char *str, ... )
{
    char tmp[1024] = {""};

    va_list	ap;

    va_start( ap, str );
    {
        vsprintf( tmp, str, ap );
    }
    va_end( ap );


#ifdef __IPHONE_4_0

    printf( "%s", tmp );
#else
    __android_log_print( ANDROID_LOG_INFO, "", "%s", tmp );
#endif
}


void build_frustum(vec4 frustum[6], const mat4 &modelview_matrix, const mat4 &projection_matrix)
{
    mat4 c;

    float t;

    // Calculate matrix "c" as the transpose of the product
    // modelview_matrix * projection_matrix.
    for (int i=0; i!=modelview_matrix.nRows(); ++i) {
        for (int j=0; j!=projection_matrix.nCols(); ++j) {
            c[j][i] = modelview_matrix[i][0] * projection_matrix[0][j];
            for (int k=1; k!=modelview_matrix.nCols(); ++k) {
                c[j][i] += modelview_matrix[i][k] * projection_matrix[k][j];
            }
        }
    }


    frustum[0] = c[3] - c[0];

    t = 1.0f / sqrtf(frustum[0]->x * frustum[0]->x +
                     frustum[0]->y * frustum[0]->y +
                     frustum[0]->z * frustum[0]->z);

    frustum[0] *= t;


    frustum[1] = c[3] + c[0];

    t = 1.0f / sqrtf(frustum[1]->x * frustum[1]->x +
                     frustum[1]->y * frustum[1]->y +
                     frustum[1]->z * frustum[1]->z);

    frustum[1] *= t;


    frustum[2] = c[3] + c[1];

    t = 1.0f / sqrtf(frustum[2]->x * frustum[2]->x +
                     frustum[2]->y * frustum[2]->y +
                     frustum[2]->z * frustum[2]->z);

    frustum[2] *= t;


    frustum[3] = c[3] - c[1];

    t = 1.0f / sqrtf(frustum[3]->x * frustum[3]->x +
                     frustum[3]->y * frustum[3]->y +
                     frustum[3]->z * frustum[3]->z);

    frustum[3] *= t;


    frustum[4] = c[3] - c[2];

    t = 1.0f / sqrtf(frustum[4]->x * frustum[4]->x +
                     frustum[4]->y * frustum[4]->y +
                     frustum[4]->z * frustum[4]->z);
    
    frustum[4] *= t;

    
    frustum[5] = c[3] + c[2];

    
    t = 1.0f / sqrtf(frustum[5]->x * frustum[5]->x +
                     frustum[5]->y * frustum[5]->y +
                     frustum[5]->z * frustum[5]->z);
    
    frustum[5] *= t;
}


// CRL
// Note that if the sphere is in the frustum the return value is always
// dependent on the last iteration through the loop.  What is stored in
// the last position of frustum which makes it especially meaningful?
// Is this by design or is a max()/min() evaluation missing from the
// function?  That is, is this an oversight?
float sphere_distance_in_frustum( vec4  *frustum, vec3  *location, float radius )
{
    float d;

    // CRL -- We can do the following in more places in this file, i.e.,
    // expand the location to (one of) its equivalent value(s) in
    // homogeneous space, and use the dotProduct() method to make the
    // code (IMHO) more readable.
    vec4 loc(*location, 1.0f);
	
    for (int i=0; i!=6; ++i) {
        d = frustum[i].dotProduct(loc);

        if (d < -radius)
            return 0.0f;
    }
    
    return d + radius;	
}


bool point_in_frustum( vec4 *frustum, vec3 *location )
{
    vec4 loc(*location, 1.0f);

    for (int i=0; i!=6; ++i) {
        if (frustum[i].dotProduct(loc) < 0.0f )
            return false;
    }
    
    return true;
}


bool box_in_frustum( vec4 *frustum, vec3 *location, vec3 *dimension )
{
    unsigned int i = 0;

    while( i != 6 )
    {
        if (frustum[i]->x * ( (*location)->x - (*dimension)->x ) +
            frustum[i]->y * ( (*location)->y - (*dimension)->y ) +
            frustum[i]->z * ( (*location)->z - (*dimension)->z ) +
            frustum[i]->w > 0.0f )
        {
            ++i;
            continue;
        }


        if (frustum[i]->x * ( (*location)->x + (*dimension)->x ) +
            frustum[i]->y * ( (*location)->y - (*dimension)->y ) +
            frustum[i]->z * ( (*location)->z - (*dimension)->z ) +
            frustum[i]->w > 0.0f )
        {
            ++i;
            continue;
        }


        if (frustum[i]->x * ( (*location)->x - (*dimension)->x ) +
            frustum[i]->y * ( (*location)->y + (*dimension)->y ) +
            frustum[i]->z * ( (*location)->z - (*dimension)->z ) +
            frustum[i]->w > 0.0f )
        {
            ++i;
            continue;
        }


        if (frustum[i]->x * ( (*location)->x + (*dimension)->x ) +
            frustum[i]->y * ( (*location)->y + (*dimension)->y ) +
            frustum[i]->z * ( (*location)->z - (*dimension)->z ) +
            frustum[i]->w > 0.0f )
        {
            ++i;
            continue;
        }


        if (frustum[i]->x * ( (*location)->x - (*dimension)->x ) +
            frustum[i]->y * ( (*location)->y - (*dimension)->y ) +
            frustum[i]->z * ( (*location)->z + (*dimension)->z ) +
            frustum[i]->w > 0.0f )
        {
            ++i;
            continue;
        }


        if (frustum[i]->x * ( (*location)->x + (*dimension)->x ) +
            frustum[i]->y * ( (*location)->y - (*dimension)->y ) +
            frustum[i]->z * ( (*location)->z + (*dimension)->z ) +
            frustum[i]->w > 0.0f )
        {
            ++i;
            continue;
        }


        if (frustum[i]->x * ( (*location)->x - (*dimension)->x ) +
            frustum[i]->y * ( (*location)->y + (*dimension)->y ) +
            frustum[i]->z * ( (*location)->z + (*dimension)->z ) +
            frustum[i]->w > 0.0f )
        {
            ++i;
            continue;
        }
        
        
        if (frustum[i]->x * ( (*location)->x + (*dimension)->x ) +
            frustum[i]->y * ( (*location)->y + (*dimension)->y ) +
            frustum[i]->z * ( (*location)->z + (*dimension)->z ) +
            frustum[i]->w > 0.0f )
        {
            ++i;
            continue;
        }
        
        return false;
    }
    
    return true;
}


InFrustum sphere_intersect_frustum( vec4  *frustum, vec3  *location, float radius )
{
    vec4 loc(*location, 1.0f);

    unsigned char c = 0;

    for (int i=0; i!=6; ++i) {
        float   d = frustum[i].dotProduct(loc);

        if (d < -radius)
            return IF_Outside;
        else if (d > radius)
            ++c;
    }

    return c == 6 ? IF_Inside : IF_Intersect;
}


InFrustum box_intersect_frustum(vec4 *frustum,
                                vec3 *location,
                                vec3 *dimension)
{
    unsigned char c1,
    c2 = 0;

    for (int i=0; i!=6; ++i) {
        c1 = 0;

        if (frustum[i]->x * ( (*location)->x - (*dimension)->x ) +
            frustum[i]->y * ( (*location)->y - (*dimension)->y ) +
            frustum[i]->z * ( (*location)->z - (*dimension)->z ) +
            frustum[i]->w > 0.0f )
        { ++c1; }


        if (frustum[i]->x * ( (*location)->x + (*dimension)->x ) +
            frustum[i]->y * ( (*location)->y - (*dimension)->y ) +
            frustum[i]->z * ( (*location)->z - (*dimension)->z ) +
            frustum[i]->w > 0.0f )
        { ++c1; }


        if (frustum[i]->x * ( (*location)->x - (*dimension)->x ) +
            frustum[i]->y * ( (*location)->y + (*dimension)->y ) +
            frustum[i]->z * ( (*location)->z - (*dimension)->z ) +
            frustum[i]->w > 0.0f )
        { ++c1; }


        if (frustum[i]->x * ( (*location)->x + (*dimension)->x ) +
            frustum[i]->y * ( (*location)->y + (*dimension)->y ) +
            frustum[i]->z * ( (*location)->z - (*dimension)->z ) +
            frustum[i]->w > 0.0f )
        { ++c1; }


        if (frustum[i]->x * ( (*location)->x - (*dimension)->x ) +
            frustum[i]->y * ( (*location)->y - (*dimension)->y ) +
            frustum[i]->z * ( (*location)->z + (*dimension)->z ) +
            frustum[i]->w > 0.0f )
        { ++c1; }


        if (frustum[i]->x * ( (*location)->x + (*dimension)->x ) +
            frustum[i]->y * ( (*location)->y - (*dimension)->y ) +
            frustum[i]->z * ( (*location)->z + (*dimension)->z ) +
            frustum[i]->w > 0.0f )
        { ++c1; }


        if (frustum[i]->x * ( (*location)->x - (*dimension)->x ) +
            frustum[i]->y * ( (*location)->y + (*dimension)->y ) +
            frustum[i]->z * ( (*location)->z + (*dimension)->z ) +
            frustum[i]->w > 0.0f )
        { ++c1; }
        
        
        if (frustum[i]->x * ( (*location)->x + (*dimension)->x ) +
            frustum[i]->y * ( (*location)->y + (*dimension)->y ) +
            frustum[i]->z * ( (*location)->z + (*dimension)->z ) +
            frustum[i]->w > 0.0f )
        { ++c1; }
        
        
        if (!c1) return IF_Outside;
        
        if (c1 == 8) ++c2;
    }
    
    return c2 == 6 ? IF_Inside : IF_Intersect;
}


unsigned int get_next_pow2(unsigned int size)
{
    switch( size )  {
        case 1:
        case 2:
        case 4:
        case 8:
        case 16:
        case 32:
        case 64:
        case 128:
        case 256:
        case 512:
        case 1024:
        case 2048:
        case 4096:
        case 8092:
            return size;
    }

    return ( unsigned int )( powf( 2.0f, ceilf( logf( ( float )size ) / logf( 2.0f ) ) ) );
}

// CRL
// Is this loop really correct?  Do we want the nearest power of 2
// or do we really want the nearest power of 2 greater than or equal
// to size?  If we really want the latter the loop should be:
//
// for (unsigned int i=1; true; i<<=1 /* or i*=2 */)
//     if (i >= size)
//         return i;
//
// Or, perhaps, this loop should be substituted for the return statement
// above in get_next_pow2().  This would avoid a bunch of floating point
// logic which tends to be really expensive.  Unfortunately, this is
// another case of providing functions which aren't used in any of the
// sample programs so we have no way of testing which hypothesis is
// correct.
unsigned int get_nearest_pow2(unsigned int size)
{
    for (unsigned int i=1; true; i*=2, size>>=1) {
        if (size == 1)
            return i;
        else if (size == 3)
            return i * 4;
    }
}


void create_direction_vector( vec3 *dst, vec3 *up_axis, float rotx, float roty, float rotz )
{
    TStack  l;
    // Convert all angles to radians & divide by 2.
    float   alpha = rotz*DEG_TO_RAD_DIV_2;
    float   cosAlpha(cosf(alpha)), sinAlpha(sinf(alpha));
    float   beta  = roty*DEG_TO_RAD_DIV_2;
    float   cosBeta(cosf(beta)), sinBeta(sinf(beta));
    float   gamma = rotx*DEG_TO_RAD_DIV_2;
    float   cosGamma(cosf(gamma)), sinGamma(sinf(gamma));
    float   cAcB(cosAlpha*cosBeta);
    float   sAsB(sinAlpha*sinBeta);
    float   cAsB(cosAlpha*sinBeta);
    float   sAcB(sinAlpha*cosBeta);
    l.loadRotation(quaternion(cAcB*cosGamma+sAsB*sinGamma,
                              cAcB*sinGamma-sAsB*cosGamma,
                              cAsB*cosGamma+sAcB*sinGamma,
                              sAcB*cosGamma-cAsB*sinGamma));

    *up_axis = -*up_axis;
    
    *dst = vec3(vec4(*up_axis, 0.0f) * l.back(), true);
}
