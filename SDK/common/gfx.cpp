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

void GFX::error( void )
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


void GFX::set_matrix_mode( unsigned int mode )
{ matrix_mode = mode; }


void GFX::load_identity( void )
{
    switch( matrix_mode )
    {
        case MODELVIEW_MATRIX:
        {
            modelview_matrix.loadIdentity();

            break;
        }

        case PROJECTION_MATRIX:
        {
            projection_matrix.loadIdentity();

            break;
        }
            
        case TEXTURE_MATRIX:
        {
            texture_matrix.loadIdentity();

            break;
        }		
    }
}


void GFX::push_matrix( void )
{
    switch( matrix_mode )
    {
        case MODELVIEW_MATRIX:
        {
            modelview_matrix.push();

            break;
        }

        case PROJECTION_MATRIX:
        {
            projection_matrix.push();

            break;
        }

        case TEXTURE_MATRIX:
        {
            texture_matrix.push();

            break;
        }		
    }
}


void GFX::pop_matrix( void )
{
    switch( matrix_mode )
    {
        case MODELVIEW_MATRIX:
        {
            modelview_matrix.pop();


            break;
        }

        case PROJECTION_MATRIX:
        {
            projection_matrix.pop();

            break;
        }
            
        case TEXTURE_MATRIX:
        {
            texture_matrix.pop();

            break;
        }		
    }
}


void GFX::load_matrix(const mat4 &m)
{
    switch( matrix_mode )
    {
        case MODELVIEW_MATRIX:
        {
            modelview_matrix.loadMatrix(m);

            break;
        }

        case PROJECTION_MATRIX:
        {
            projection_matrix.loadMatrix(m);

            break;
        }
            
        case TEXTURE_MATRIX:
        {
            texture_matrix.loadMatrix(m);

            break;
        }		
    }
}


void GFX::multiply_matrix(const mat4 &m)
{
    switch( matrix_mode )
    {
        case MODELVIEW_MATRIX:
        {
            modelview_matrix.multiplyMatrix(m);


            break;
        }

        case PROJECTION_MATRIX:
        {
            projection_matrix.multiplyMatrix(m);

            break;
        }

        case TEXTURE_MATRIX:
        {
            texture_matrix.multiplyMatrix(m);

            break;
        }		
    }
}


void GFX::translate(const float x, const float y, const float z)
{
    switch ( matrix_mode )
    {
        case MODELVIEW_MATRIX:
        {
            modelview_matrix.translate(x, y, z);

            break;
        }

        case PROJECTION_MATRIX:
        {
            projection_matrix.translate(x, y, z);

            break;
        }

        case TEXTURE_MATRIX:
        {
            texture_matrix.translate(x, y, z);
            
            break;
        }		
    }
}


void GFX::translate(const vec3 &t)
{
    switch( matrix_mode )
    {
        case MODELVIEW_MATRIX:
        {
            modelview_matrix.translate(t);

            break;
        }

        case PROJECTION_MATRIX:
        {
            projection_matrix.translate(t);

            break;
        }

        case TEXTURE_MATRIX:
        {
            texture_matrix.translate(t);

            break;
        }		
    }
}


void GFX::rotate(const float angle, const float x, const float y, const float z)
{
    if (!angle) return;

    vec4 v(x, y, z, angle);

    switch( matrix_mode )
    {
        case MODELVIEW_MATRIX:
        {
            modelview_matrix.rotate(angle, vec3(x,y,z));

            break;
        }

        case PROJECTION_MATRIX:
        {
            projection_matrix.rotate(angle, vec3(x,y,z));

            break;
        }

        case TEXTURE_MATRIX:
        {
            texture_matrix.rotate(angle, vec3(x,y,z));

            break;
        }		
    }
}


void GFX::rotate(const quaternion &q)
{
    if (q.w==1.0f || q.w==-1.0f) return;

    switch( matrix_mode )
    {
        case MODELVIEW_MATRIX:
        {
            modelview_matrix.rotate(q);

            break;
        }

        case PROJECTION_MATRIX:
        {
            projection_matrix.rotate(q);

            break;
        }

        case TEXTURE_MATRIX:
        {
            texture_matrix.rotate(q);
            
            break;
        }		
    }
}


void GFX::scale(const float x, const float y, const float z)
{
    if (x==1.0f && y==1.0f && z==1.0f) return;

    switch ( matrix_mode )
    {
        case MODELVIEW_MATRIX:
        {
            modelview_matrix.scale(x, y, z);

            break;
        }

        case PROJECTION_MATRIX:
        {
            projection_matrix.scale(x, y, z);

            break;
        }

        case TEXTURE_MATRIX:
        {
            texture_matrix.scale(x, y, z);
            
            break;
        }		
    }
}


void GFX::scale(const vec3 &v)
{
    if (v->x==1.0f && v->y==1.0f && v->z==1.0f) return;

    switch (matrix_mode )
    {
        case MODELVIEW_MATRIX:
        {
            modelview_matrix.scale(v);

            break;
        }

        case PROJECTION_MATRIX:
        {
            projection_matrix.scale(v);

            break;
        }

        case TEXTURE_MATRIX:
        {
            texture_matrix.scale(v);

            break;
        }		
    }
}


mat4 &GFX::get_modelview_matrix(const int i)
{
    return modelview_matrix[i];
}


mat4 &GFX::get_projection_matrix( void )
{
    return projection_matrix.back();
}


mat4 &GFX::get_texture_matrix( void )
{
    return texture_matrix.back();
}


mat4 &GFX::get_modelview_projection_matrix( void )
{
    modelview_projection_matrix =
        modelview_matrix.back() * projection_matrix.back();

    return modelview_projection_matrix;
}


mat3 &GFX::get_normal_matrix( void )
{
    normal_matrix = modelview_matrix.getNormalMatrix();

    return normal_matrix;
}


void GFX::ortho(const float left, const float right,
                const float bottom, const float top,
                const float clip_start, const float clip_end)
{
    switch( matrix_mode )
    {
        case MODELVIEW_MATRIX:
        {
            modelview_matrix.ortho(left, right, bottom, top, clip_start, clip_end);

            break;
        }

        case PROJECTION_MATRIX:
        {
            projection_matrix.ortho(left, right, bottom, top, clip_start, clip_end);

            break;
        }

        case TEXTURE_MATRIX:
        {
            texture_matrix.ortho(left, right, bottom, top, clip_start, clip_end);

            break;
        }		
    }
}


void GFX::set_orthographic_2d(const float left, const float right,
                              const float bottom, const float top)
{ ortho( left, right, bottom, top, -1.0f, 1.0f ); }


void GFX::set_orthographic(const float screen_ratio, float s, const float aspect_ratio, const float clip_start, const float clip_end, const float screen_orientation)
{
    s = ( s * 0.5f ) * aspect_ratio;

    ortho(-1.0f,
           1.0f,
          -screen_ratio,
           screen_ratio,
          clip_start,
          clip_end );

    scale( 1.0f / s, 1.0f / s, 1.0f );

    if (screen_orientation ) rotate( screen_orientation, 0.0f, 0.0f, 1.0f );
}


void GFX::set_perspective(const float fovy, const float aspect_ratio,
                          const float clip_start, const float clip_end,
                          const float screen_orientation)
{
    switch (matrix_mode )
    {
        case MODELVIEW_MATRIX:
        {
            modelview_matrix.perspective(fovy, aspect_ratio, clip_start, clip_end);
            modelview_matrix.rotate(screen_orientation, vec3(0,0,1));

            break;
        }

        case PROJECTION_MATRIX:
        {
            projection_matrix.perspective(fovy, aspect_ratio, clip_start, clip_end);
            projection_matrix.rotate(screen_orientation, vec3(0,0,1));

            break;
        }

        case TEXTURE_MATRIX:
        {
            texture_matrix.perspective(fovy, aspect_ratio, clip_start, clip_end);
            texture_matrix.rotate(screen_orientation, vec3(0,0,1));

            break;
        }		
    }
}


void GFX::look_at(const vec3 &eye, const vec3 &center, const vec3 &up)
{
    switch( matrix_mode )
    {
        case MODELVIEW_MATRIX:
        {
            modelview_matrix.lookAt(eye, center, up);

            break;
        }

        case PROJECTION_MATRIX:
        {
            projection_matrix.lookAt(eye, center, up);

            break;
        }

        case TEXTURE_MATRIX:
        {
            texture_matrix.lookAt(eye, center, up);
            
            break;
        }		
    }
}


bool GFX::project(const float objx, const float objy, const float objz,
                  const mat4 &modelview_matrix,
                  const mat4 &projection_matrix,
                  const int *viewport_matrix,
                  float *winx, float *winy, float *winz)
{
    return project(vec3(objx, objy, objz),
                   modelview_matrix,
                   projection_matrix,
                   viewport_matrix,
                   winx, winy, winz);
}

bool GFX::project(const vec3 &obj,
                  const mat4 &modelview_matrix,
                  const mat4 &projection_matrix,
                  const int *viewport_matrix,
                  float *winx, float *winy, float *winz)
{
    vec4    vin(obj, 1.0f),
            vout;

    vout = vin * modelview_matrix;

    vin = vout * projection_matrix;

    if (!vin->w) return false;

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

bool GFX::unproject(const float winx, const float winy, const float winz,
                    const mat4 &modelview_matrix,
                    const mat4 &projection_matrix,
                    const int *viewport_matrix,
                    float *objx, float *objy, float *objz)
{
    vec3    vout;
    bool    status = unproject(winx, winy, winz,
                               modelview_matrix,
                               projection_matrix,
                               viewport_matrix,
                               vout);
    if (status) {
        *objx = vout->x;
        *objy = vout->y;
        *objz = vout->z;
    }

    return status;
}

bool GFX::unproject(const float winx, const float winy, const float winz,
                    const mat4 &modelview_matrix,
                    const mat4 &projection_matrix,
                    const int *viewport_matrix, vec3 &obj)
{
    vec4    vin(winx, winy, winz, 1.0f);

    for (int i=0; i<2; ++i)
        vin[i] = (vin[i] - viewport_matrix[i]) / viewport_matrix[i+2];

    for (int i=0; i<3; ++i)
        vin[i] = vin[i] * 2.0f - 1.0f;
    
    // Equivalent to operations on vin above.  The operations above
    // are much more efficient but the operations below give the
    // reader better insight into what the code is actually doing
    // mathematically.
//    TStack  l;
//
//    l.translate(vec3(-1.0f, -1.0f, -1.0f));
//    l.scale(vec3(2.0f, 2.0f, 2.0f));
//    l.scale(vec3(1.0f/viewport_matrix[2], 1.0f/viewport_matrix[3], 1.0f));
//    l.translate(vec3(-viewport_matrix[0], -viewport_matrix[1], 0.0f));
//    vin *= l.back();

    vec4    vout = vin / (modelview_matrix * projection_matrix);

    return vout->w ? obj=vec3(vout),true : false;
}
