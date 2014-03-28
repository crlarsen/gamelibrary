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

GFX *gfx;


#ifndef __IPHONE_4_0

    PFNGLBINDVERTEXARRAYOESPROC		glBindVertexArrayOES;
    PFNGLGENVERTEXARRAYSOESPROC		glGenVertexArraysOES;
    PFNGLDELETEVERTEXARRAYSOESPROC	glDeleteVertexArraysOES;
#endif

GFX::GFX() :
    matrix_mode(MODELVIEW_MATRIX)
{
#ifdef __IPHONE_4_0

    printf("\nGL_VENDOR:      %s\n", ( char * )glGetString( GL_VENDOR     ) );
    printf("GL_RENDERER:    %s\n"  , ( char * )glGetString( GL_RENDERER   ) );
    printf("GL_VERSION:     %s\n"  , ( char * )glGetString( GL_VERSION    ) );
    printf("GL_EXTENSIONS:  %s\n"  , ( char * )glGetString( GL_EXTENSIONS ) );
#else

    __android_log_print( ANDROID_LOG_INFO, "", "\nGL_VENDOR:      %s\n", ( char * )glGetString( GL_VENDOR     ) );
    __android_log_print( ANDROID_LOG_INFO, "", "GL_RENDERER:    %s\n"  , ( char * )glGetString( GL_RENDERER   ) );
    __android_log_print( ANDROID_LOG_INFO, "", "GL_VERSION:     %s\n"  , ( char * )glGetString( GL_VERSION    ) );
    __android_log_print( ANDROID_LOG_INFO, "", "GL_EXTENSIONS:  %s\n"  , ( char * )glGetString( GL_EXTENSIONS ) );
#endif

    glHint( GL_GENERATE_MIPMAP_HINT, GL_NICEST );

    glHint( GL_FRAGMENT_SHADER_DERIVATIVE_HINT_OES, GL_NICEST );

    glEnable( GL_DEPTH_TEST );
    glEnable( GL_CULL_FACE  );
    glDisable( GL_DITHER );
    glDepthMask( GL_TRUE );
    glDepthFunc( GL_LESS );
    glDepthRangef( 0.0f, 1.0f );
    glCullFace ( GL_BACK );
    glFrontFace( GL_CCW  );
    glClearStencil( 0 );
    glStencilMask( 0xFFFFFFFF );

    glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );

    glClear( GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT );

#ifndef __IPHONE_4_0

    glBindVertexArrayOES 	= ( PFNGLBINDVERTEXARRAYOESPROC    ) eglGetProcAddress("glBindVertexArrayOES"  );
    glGenVertexArraysOES 	= ( PFNGLGENVERTEXARRAYSOESPROC    ) eglGetProcAddress("glGenVertexArraysOES"  );
    glDeleteVertexArraysOES 	= ( PFNGLDELETEVERTEXARRAYSOESPROC ) eglGetProcAddress("glDeleteVertexArraysOES");
#endif

}

void GFX_start( void )
{
    gfx = new GFX;
}


void GFX_error( void )
{
    unsigned int error;

    while( ( error = glGetError() ) != GL_NO_ERROR )
    {
        char str[ MAX_CHAR ] = {""};

        switch( error )
        {
            case GL_INVALID_ENUM:
            {
                strcpy( str, "GL_INVALID_ENUM" );
                break;
            }

            case GL_INVALID_VALUE:
            {
                strcpy( str, "GL_INVALID_VALUE" );
                break;
            }

            case GL_INVALID_OPERATION:
            {
                strcpy( str, "GL_INVALID_OPERATION" );
                break;
            }
                
            case GL_OUT_OF_MEMORY:
            {
                strcpy( str, "GL_OUT_OF_MEMORY" );
                break;
            }
        }
        
        console_print( "[ GL_ERROR ]\nERROR: %s\n", str );
    }
}


void GFX_set_matrix_mode( unsigned int mode )
{ gfx->matrix_mode = mode; }


void GFX_load_identity( void )
{
    switch( gfx->matrix_mode )
    {
        case MODELVIEW_MATRIX:
        {
            gfx->modelview_matrix.loadIdentity();

            break;
        }

        case PROJECTION_MATRIX:
        {
            gfx->projection_matrix.loadIdentity();

            break;
        }
            
        case TEXTURE_MATRIX:
        {
            gfx->texture_matrix.loadIdentity();

            break;
        }		
    }
}


void GFX_push_matrix( void )
{
    switch( gfx->matrix_mode )
    {
        case MODELVIEW_MATRIX:
        {
            gfx->modelview_matrix.push();

            break;
        }

        case PROJECTION_MATRIX:
        {
            gfx->projection_matrix.push();

            break;
        }

        case TEXTURE_MATRIX:
        {
            gfx->texture_matrix.push();

            break;
        }		
    }
}


void GFX_pop_matrix( void )
{
    switch( gfx->matrix_mode )
    {
        case MODELVIEW_MATRIX:
        {
            gfx->modelview_matrix.pop();


            break;
        }

        case PROJECTION_MATRIX:
        {
            gfx->projection_matrix.pop();

            break;
        }
            
        case TEXTURE_MATRIX:
        {
            gfx->texture_matrix.pop();

            break;
        }		
    }
}


void GFX_load_matrix(const mat4 &m)
{
    switch( gfx->matrix_mode )
    {
        case MODELVIEW_MATRIX:
        {
            gfx->modelview_matrix.loadMatrix(m);

            break;
        }

        case PROJECTION_MATRIX:
        {
            gfx->projection_matrix.loadMatrix(m);

            break;
        }
            
        case TEXTURE_MATRIX:
        {
            gfx->texture_matrix.loadMatrix(m);

            break;
        }		
    }
}


void GFX_multiply_matrix(const mat4 &m)
{
    switch( gfx->matrix_mode )
    {
        case MODELVIEW_MATRIX:
        {
            gfx->modelview_matrix.multiplyMatrix(m);


            break;
        }

        case PROJECTION_MATRIX:
        {
            gfx->projection_matrix.multiplyMatrix(m);

            break;
        }

        case TEXTURE_MATRIX:
        {
            gfx->texture_matrix.multiplyMatrix(m);

            break;
        }		
    }
}


void GFX_translate(const float x, const float y, const float z)
{
    vec3 v(x, y, z);

    GFX_translate(v);
}


void GFX_translate(const vec3 &t)
{
    switch( gfx->matrix_mode )
    {
        case MODELVIEW_MATRIX:
        {
            gfx->modelview_matrix.translate(t);

            break;
        }

        case PROJECTION_MATRIX:
        {
            gfx->projection_matrix.translate(t);

            break;
        }

        case TEXTURE_MATRIX:
        {
            gfx->texture_matrix.translate(t);

            break;
        }		
    }
}


void GFX_rotate(const float angle, const float x, const float y, const float z)
{
    if( !angle ) return;

    vec4 v(x, y, z, angle);

    switch( gfx->matrix_mode )
    {
        case MODELVIEW_MATRIX:
        {
            gfx->modelview_matrix.rotate(angle, vec3(x,y,z));

            break;
        }

        case PROJECTION_MATRIX:
        {
            gfx->projection_matrix.rotate(angle, vec3(x,y,z));

            break;
        }

        case TEXTURE_MATRIX:
        {
            gfx->texture_matrix.rotate(angle, vec3(x,y,z));

            break;
        }		
    }
}


void GFX_rotate(const quaternion &q)
{
    if(q.w==1.0f || q.w==-1.0f) return;

    switch( gfx->matrix_mode )
    {
        case MODELVIEW_MATRIX:
        {
            gfx->modelview_matrix.rotate(q);

            break;
        }

        case PROJECTION_MATRIX:
        {
            gfx->projection_matrix.rotate(q);

            break;
        }

        case TEXTURE_MATRIX:
        {
            gfx->texture_matrix.rotate(q);
            
            break;
        }		
    }
}


void GFX_scale(const float x, const float y, const float z)
{
    static vec3 scale(1.0f, 1.0f, 1.0f);

    vec3 v(x, y, z);

    if (v == scale) return;

    GFX_scale(v);
}


void GFX_scale(const vec3 &v)
{
    static vec3 scale(1.0f, 1.0f, 1.0f);

    if (v == scale) return;


    switch( gfx->matrix_mode )
    {
        case MODELVIEW_MATRIX:
        {
            gfx->modelview_matrix.scale(v);

            break;
        }

        case PROJECTION_MATRIX:
        {
            gfx->projection_matrix.scale(v);

            break;
        }

        case TEXTURE_MATRIX:
        {
            gfx->texture_matrix.scale(v);

            break;
        }		
    }
}


mat4 &GFX_get_modelview_matrix(const int i)
{
    return gfx->modelview_matrix[i];
}


mat4 &GFX_get_projection_matrix( void )
{
    return gfx->projection_matrix.back();
}


mat4 &GFX_get_texture_matrix( void )
{
    return gfx->texture_matrix.back();
}


mat4 &GFX_get_modelview_projection_matrix( void )
{
    gfx->modelview_projection_matrix =
        gfx->modelview_matrix.back() * gfx->projection_matrix.back();

    return gfx->modelview_projection_matrix;
}


mat3 &GFX_get_normal_matrix( void )
{
    gfx->normal_matrix = gfx->modelview_matrix.getNormalMatrix();

    return gfx->normal_matrix;
}


void GFX_ortho(const float left, const float right,
               const float bottom, const float top,
               const float clip_start, const float clip_end)
{
    switch( gfx->matrix_mode )
    {
        case MODELVIEW_MATRIX:
        {
            gfx->modelview_matrix.ortho(left, right, bottom, top, clip_start, clip_end);

            break;
        }

        case PROJECTION_MATRIX:
        {
            gfx->projection_matrix.ortho(left, right, bottom, top, clip_start, clip_end);

            break;
        }

        case TEXTURE_MATRIX:
        {
            gfx->texture_matrix.ortho(left, right, bottom, top, clip_start, clip_end);

            break;
        }		
    }
}


void GFX_set_orthographic_2d(const float left, const float right,
                             const float bottom, const float top)
{ GFX_ortho( left, right, bottom, top, -1.0f, 1.0f ); }


void GFX_set_orthographic(const float screen_ratio, float scale, const float aspect_ratio, const float clip_start, const float clip_end, const float screen_orientation)
{
    scale = ( scale * 0.5f ) * aspect_ratio;

    GFX_ortho(-1.0f,
              1.0f,
              -screen_ratio,
              screen_ratio,
              clip_start,
              clip_end );

    GFX_scale( 1.0f / scale, 1.0f / scale, 1.0f );

    if( screen_orientation ) GFX_rotate( screen_orientation, 0.0f, 0.0f, 1.0f );
}


void GFX_set_perspective(const float fovy, const float aspect_ratio,
                         const float clip_start, const float clip_end,
                         const float screen_orientation)
{
    mat4 mat(1, 0, 0, 0,
             0, 1, 0, 0,
             0, 0, 1, 0,
             0, 0, 0, 1);

    float   d = clip_end - clip_start,
            r = ( fovy * 0.5f ) * DEG_TO_RAD,
            s = sinf( r ),
            c = cosf( r ) / s;

    mat[0][0] = c / aspect_ratio;
    mat[1][1] = c;
    mat[2][2] = -( clip_end + clip_start ) / d;
    mat[2][3] = -1.0f;
    mat[3][2] = -2.0f * clip_start * clip_end / d;
    mat[3][3] =  0.0f;

    GFX_multiply_matrix(mat);

    if( screen_orientation ) GFX_rotate( screen_orientation, 0.0f, 0.0f, 1.0f );
}


void GFX_look_at(const vec3 &eye, const vec3 &center, const vec3 &up)
{
    vec3    f,
            s,
            u;

    mat4 mat(1, 0, 0, 0,
             0, 1, 0, 0,
             0, 0, 1, 0,
             0, 0, 0, 1);

    f = center - eye;

    f.safeNormalize();

    s = f.crossProduct(up);

    s.safeNormalize();

    u = s.crossProduct(f);

    mat[0][0] = s->x;
    mat[1][0] = s->y;
    mat[2][0] = s->z;

    mat[0][1] = u->x;
    mat[1][1] = u->y;
    mat[2][1] = u->z;

    mat[0][2] = -f->x;
    mat[1][2] = -f->y;
    mat[2][2] = -f->z;
    
    GFX_multiply_matrix(mat);
    
    GFX_translate(-eye);
}


bool GFX_project(const float objx, const float objy, const float objz,
                 const mat4 &modelview_matrix,
                 const mat4 &projection_matrix,
                 const int *viewport_matrix,
                 float *winx, float *winy, float *winz)
{
    return GFX_project(vec3(objx, objy, objz),
                       modelview_matrix,
                       projection_matrix,
                       viewport_matrix,
                       winx, winy, winz);
}

bool GFX_project(const vec3 &obj,
                 const mat4 &modelview_matrix,
                 const mat4 &projection_matrix,
                 const int *viewport_matrix,
                 float *winx, float *winy, float *winz)
{
    vec4    vin(obj, 1.0f),
            vout;

    vout = vin * modelview_matrix;

    vin = vout * projection_matrix;

    if( !vin->w ) return false;

    vin->x /= vin->w;
    vin->y /= vin->w;
    vin->z /= vin->w;

    vin->x = vin->x * 0.5f + 0.5f;
    vin->y = vin->y * 0.5f + 0.5f;
    vin->z = vin->z * 0.5f + 0.5f;

    vin->x = vin->x * viewport_matrix[2] + viewport_matrix[0];
    vin->y = vin->y * viewport_matrix[3] + viewport_matrix[1];

    *winx = vin->x;
    *winy = vin->y;
    *winz = vin->z;

    return true;
}

bool GFX_unproject(const float winx, const float winy, const float winz,
                   const mat4 &modelview_matrix,
                   const mat4 &projection_matrix,
                   const int *viewport_matrix,
                   float *objx, float *objy, float *objz)
{
    vec3    vout;
    bool    status = GFX_unproject(winx, winy, winz,
                                   modelview_matrix,
                                   projection_matrix,
                                   viewport_matrix,
                                   vout);
    *objx = vout->x;
    *objy = vout->y;
    *objz = vout->z;

    return status;
}

bool GFX_unproject(const float winx, const float winy, const float winz,
                   const mat4 &modelview_matrix,
                   const mat4 &projection_matrix,
                   const int *viewport_matrix, vec3 &obj)
{
    mat4    final;

    vec4    vin,
            vout;

    final = mat4(modelview_matrix * projection_matrix).inverse();

    vin->x = winx;
    vin->y = winy;
    vin->z = winz;
    vin->w = 1.0f;

    vin->x = (vin->x - viewport_matrix[0]) / viewport_matrix[2];
    vin->y = (vin->y - viewport_matrix[1]) / viewport_matrix[3];

    vin->x = vin->x * 2.0f - 1.0f;
    vin->y = vin->y * 2.0f - 1.0f;
    vin->z = vin->z * 2.0f - 1.0f;

    vout = vin * final;

    if(!vout->w) return false;

    obj = vec3(vout);

    return true;
}
