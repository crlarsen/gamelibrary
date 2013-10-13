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

GFX gfx;


#ifndef __IPHONE_4_0

	PFNGLBINDVERTEXARRAYOESPROC		glBindVertexArrayOES;
	PFNGLGENVERTEXARRAYSOESPROC		glGenVertexArraysOES;
	PFNGLDELETEVERTEXARRAYSOESPROC	glDeleteVertexArraysOES;
#endif


void GFX_start( void )
{
	memset( &gfx, 0, sizeof( GFX ) );
	
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
		glDeleteVertexArraysOES = ( PFNGLDELETEVERTEXARRAYSOESPROC ) eglGetProcAddress("glDeleteVertexArraysOES");
	#endif
	
	GFX_set_matrix_mode( TEXTURE_MATRIX );
	GFX_load_identity();

	GFX_set_matrix_mode( PROJECTION_MATRIX );
	GFX_load_identity();

	GFX_set_matrix_mode( MODELVIEW_MATRIX );
	GFX_load_identity();
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
{ gfx.matrix_mode = mode; }


void GFX_load_identity( void )
{
	switch( gfx.matrix_mode )
	{
		case MODELVIEW_MATRIX:
		{
			mat4_identity( GFX_get_modelview_matrix() );
			
			break;
		}
		
		case PROJECTION_MATRIX:
		{
			mat4_identity( GFX_get_projection_matrix() );
			
			break;
		}
		
		case TEXTURE_MATRIX:
		{
			mat4_identity( GFX_get_texture_matrix() );
			
			break;
		}		
	}
}


void GFX_push_matrix( void )
{
	switch( gfx.matrix_mode )
	{
		case MODELVIEW_MATRIX:
		{
			mat4_copy_mat4( &gfx.modelview_matrix[ gfx.modelview_matrix_index + 1 ],
							&gfx.modelview_matrix[ gfx.modelview_matrix_index	  ] );
		
			++gfx.modelview_matrix_index;
			
			break;
		}

		case PROJECTION_MATRIX:
		{
			mat4_copy_mat4( &gfx.projection_matrix[ gfx.projection_matrix_index + 1 ],
							&gfx.projection_matrix[ gfx.projection_matrix_index	    ] );
			
			++gfx.projection_matrix_index;
			
			break;
		}
		
		case TEXTURE_MATRIX:
		{
			mat4_copy_mat4( &gfx.texture_matrix[ gfx.texture_matrix_index + 1 ],
							&gfx.texture_matrix[ gfx.texture_matrix_index     ] );
			
			++gfx.texture_matrix_index;
			
			break;
		}		
	}
}


void GFX_pop_matrix( void )
{
	switch( gfx.matrix_mode )
	{
		case MODELVIEW_MATRIX:
		{
			--gfx.modelview_matrix_index;
			
			break;
		}
			
		case PROJECTION_MATRIX:
		{
			--gfx.projection_matrix_index;
			
			break;
		}
		
		case TEXTURE_MATRIX:
		{
			--gfx.texture_matrix_index;
			
			break;
		}		
	}
}


void GFX_load_matrix( mat4 *m )
{
	switch( gfx.matrix_mode )
	{
		case MODELVIEW_MATRIX:
		{
			mat4_copy_mat4( GFX_get_modelview_matrix(), m );
			
			break;
		}
	
		case PROJECTION_MATRIX:
		{
			mat4_copy_mat4( GFX_get_projection_matrix(), m );
			
			break;
		}
		
		case TEXTURE_MATRIX:
		{
			mat4_copy_mat4( GFX_get_texture_matrix(), m );
			
			break;
		}		
	}
}


void GFX_multiply_matrix( mat4 *m )
{
	switch( gfx.matrix_mode )
	{
		case MODELVIEW_MATRIX:
		{
			mat4_multiply_mat4( GFX_get_modelview_matrix(), GFX_get_modelview_matrix(), m );

			break;
		}
			
		case PROJECTION_MATRIX:
		{
			mat4_multiply_mat4( GFX_get_projection_matrix(), GFX_get_projection_matrix(), m );
			
			break;
		}
		
		case TEXTURE_MATRIX:
		{
			mat4_multiply_mat4( GFX_get_texture_matrix(), GFX_get_texture_matrix(), m );
			
			break;
		}		
	}
}


void GFX_translate( float x, float y, float z )
{
	vec3 v = { x, y, z };
	
	switch( gfx.matrix_mode )
	{
		case MODELVIEW_MATRIX:
		{
			mat4_translate( GFX_get_modelview_matrix(), GFX_get_modelview_matrix(), &v );
			
			break;
		}
	
		case PROJECTION_MATRIX:
		{
			mat4_translate( GFX_get_projection_matrix(), GFX_get_projection_matrix(), &v );
			
			break;
		}

		case TEXTURE_MATRIX:
		{
			mat4_translate( GFX_get_texture_matrix(), GFX_get_texture_matrix(), &v );
			
			break;
		}		
	}
}


void GFX_rotate( float angle, float x, float y, float z )
{
	if( !angle ) return;
	
	vec4 v = { x, y, z, angle };
	
	switch( gfx.matrix_mode )
	{
		case MODELVIEW_MATRIX:
		{
			mat4_rotate( GFX_get_modelview_matrix(), GFX_get_modelview_matrix(), &v );
			
			break;
		}
	
		case PROJECTION_MATRIX:
		{
			mat4_rotate( GFX_get_projection_matrix(), GFX_get_projection_matrix(), &v );
			
			break;
		}

		case TEXTURE_MATRIX:
		{
			mat4_rotate( GFX_get_texture_matrix(), GFX_get_texture_matrix(), &v );
			
			break;
		}		
	}
}


void GFX_scale( float x, float y, float z )
{
	static vec3 scale = { 1.0f, 1.0f, 1.0f };
	
	vec3 v = { x, y, z };
	
	if( !memcmp( &v, &scale, sizeof( vec3 ) ) ) return;
	
	
	switch( gfx.matrix_mode )
	{
		case MODELVIEW_MATRIX:
		{
			mat4_scale( GFX_get_modelview_matrix(), GFX_get_modelview_matrix(), &v );
			
			break;
		}
	
		case PROJECTION_MATRIX:
		{
			mat4_scale( GFX_get_projection_matrix(), GFX_get_projection_matrix(), &v );
			
			break;
		}

		case TEXTURE_MATRIX:
		{
			mat4_scale( GFX_get_texture_matrix(), GFX_get_texture_matrix(), &v );
			
			break;
		}		
	}
}


mat4 *GFX_get_modelview_matrix( void )
{ return &gfx.modelview_matrix[ gfx.modelview_matrix_index ]; }


mat4 *GFX_get_projection_matrix( void )
{ return &gfx.projection_matrix[ gfx.projection_matrix_index ]; }


mat4 *GFX_get_texture_matrix( void )
{ return &gfx.texture_matrix[ gfx.texture_matrix_index ]; }


mat4 *GFX_get_modelview_projection_matrix( void )
{
	mat4_multiply_mat4( &gfx.modelview_projection_matrix, 
						GFX_get_projection_matrix(),
						GFX_get_modelview_matrix() );
	
	return &gfx.modelview_projection_matrix; 
}


mat3 *GFX_get_normal_matrix( void )
{
	mat4 mat;
	
	mat4_copy_mat4( &mat, GFX_get_modelview_matrix() );

	mat4_invert_full( &mat );

	mat4_transpose( &mat );
	
	mat3_copy_mat4( &gfx.normal_matrix, &mat );

	return &gfx.normal_matrix;
}


void GFX_ortho( float left, float right, float bottom, float top, float clip_start, float clip_end )
{
	switch( gfx.matrix_mode )
	{
		case MODELVIEW_MATRIX:
		{
			mat4_ortho( GFX_get_modelview_matrix(), left, right, bottom, top, clip_start, clip_end ); 
			
			break;
		}
	
		case PROJECTION_MATRIX:
		{
			mat4_ortho( GFX_get_projection_matrix(), left, right, bottom, top, clip_start, clip_end );
			
			break;
		}

		case TEXTURE_MATRIX:
		{
			mat4_ortho( GFX_get_texture_matrix(), left, right, bottom, top, clip_start, clip_end );
			
			break;
		}		
	}
}


void GFX_set_orthographic_2d( float left, float right, float bottom, float top )
{ GFX_ortho( left, right, bottom, top, -1.0f, 1.0f ); }


void GFX_set_orthographic( float screen_ratio, float scale, float aspect_ratio, float clip_start, float clip_end, float screen_orientation )
{
	scale = ( scale * 0.5f ) * aspect_ratio;

	GFX_ortho( -1.0f, 
			    1.0f, 
			   -screen_ratio,
			    screen_ratio, 
			    clip_start,
				clip_end );
	
	GFX_scale( 1.0f / scale, 1.0f / scale, 1.0f );
		
	if( screen_orientation ) GFX_rotate( screen_orientation, 0.0f, 0.0f, 1.0f );
}


void GFX_set_perspective( float fovy, float aspect_ratio, float clip_start, float clip_end, float screen_orientation )
{
	mat4 mat;
	
	float d = clip_end - clip_start,
		  r = ( fovy * 0.5f ) * DEG_TO_RAD,
		  s = sinf( r ),
		  c = cosf( r ) / s;

	mat4_identity( &mat );
	
    mat.m[ 0 ].x = c / aspect_ratio;
    mat.m[ 1 ].y = c;
    mat.m[ 2 ].z = -( clip_end + clip_start ) / d;
    mat.m[ 2 ].w = -1.0f;
    mat.m[ 3 ].z = -2.0f * clip_start * clip_end / d;
    mat.m[ 3 ].w =  0.0f;
	
	GFX_multiply_matrix( &mat );
	
	if( screen_orientation ) GFX_rotate( screen_orientation, 0.0f, 0.0f, 1.0f );
}


void GFX_look_at( vec3 *eye, vec3 *center, vec3 *up )
{
	vec3 f,
		 s,
		 u;

	mat4 mat;

	mat4_identity( &mat );

	vec3_diff( &f, center, eye );

	vec3_normalize( &f, &f );

	vec3_cross( &s, &f, up );

	vec3_normalize( &s, &s );

	vec3_cross( &u, &s, &f );

	mat.m[ 0 ].x = s.x;
	mat.m[ 1 ].x = s.y;
	mat.m[ 2 ].x = s.z;

	mat.m[ 0 ].y = u.x;
	mat.m[ 1 ].y = u.y;
	mat.m[ 2 ].y = u.z;

	mat.m[ 0 ].z = -f.x;
	mat.m[ 1 ].z = -f.y;
	mat.m[ 2 ].z = -f.z;

	GFX_multiply_matrix( &mat );

	GFX_translate( -eye->x, -eye->y, -eye->z );
}


int GFX_project( float objx, float objy, float objz, mat4 *modelview_matrix, mat4 *projection_matrix, int *viewport_matrix, float *winx, float *winy, float *winz )
{
	vec4 vin,
		 vout;
		 
	vin.x = objx;
	vin.y = objy;
	vin.z = objz;
	vin.w = 1.0f;
	
	vec4_multiply_mat4( &vout, &vin, modelview_matrix );

	vec4_multiply_mat4( &vin, &vout, projection_matrix );

	if( !vin.w ) return 0;
		
	vin.x /= vin.w;
	vin.y /= vin.w;
	vin.z /= vin.w;
	
	vin.x = vin.x * 0.5f + 0.5f;
	vin.y = vin.y * 0.5f + 0.5f;
	vin.z = vin.z * 0.5f + 0.5f;

	vin.x = vin.x * viewport_matrix[ 2 ] + viewport_matrix[ 0 ];
	vin.y = vin.y * viewport_matrix[ 3 ] + viewport_matrix[ 1 ];

	*winx = vin.x;
	*winy = vin.y;
	*winz = vin.z;

	return 1;
}


int GFX_unproject( float winx, float winy, float winz, mat4 *modelview_matrix, mat4 *projection_matrix, int *viewport_matrix, float *objx, float *objy, float *objz )
{
	mat4 final;

	vec4 vin,
		 vout;

	mat4_multiply_mat4( &final,
						projection_matrix,
						modelview_matrix );

	mat4_invert_full( &final );

	vin.x = winx;
	vin.y = winy;
	vin.z = winz;
	vin.w = 1.0f;

	vin.x = ( vin.x - viewport_matrix[ 0 ] ) / viewport_matrix[ 2 ];
	vin.y = ( vin.y - viewport_matrix[ 1 ] ) / viewport_matrix[ 3 ];

	vin.x = vin.x * 2.0f - 1.0f;
	vin.y = vin.y * 2.0f - 1.0f;
	vin.z = vin.z * 2.0f - 1.0f;

	vec4_multiply_mat4( &vout, &vin, &final );
	
	if( !vout.w ) return 0;
	
	vout.x /= vout.w;
	vout.y /= vout.w;
	vout.z /= vout.w;
	
	*objx = vout.x;
	*objy = vout.y;
	*objz = vout.z;
	
	return 1;
}
