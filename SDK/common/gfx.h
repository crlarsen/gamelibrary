
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
 * - FONT
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

#ifndef GFX_H
#define GFX_H

#define GFX_COPYRIGHT		"Copyright (C) 2011 Romain Marucchi-Foino. All Rights Reserved."
#define GFX_RENDERER		"GFX"

#define GFX_VERSION_MAJOR	1
#define GFX_VERSION_MINOR	0
#define GFX_VERSION_PATCH	22


#ifdef __IPHONE_4_0

	#include <OpenGLES/ES2/gl.h>
	#include <OpenGLES/ES2/glext.h>

	#include <OpenAL/al.h>
	#include <OpenAL/alc.h>
	
	#include "png.h"
	#include "zlib.h"
	#include "unzip.h"
	#include "NvTriStrip.h"
	
	#include "btBulletDynamicsCommon.h"
	#include "btSoftRigidDynamicsWorld.h"
	#include "btSoftBodyRigidBodyCollisionConfiguration.h"
	#include "btShapeHull.h"
	#include "btSoftBodyHelpers.h"
	#include "btSoftBody.h"
	#include "btGImpactShape.h"
	#include "btGImpactCollisionAlgorithm.h"
	#include "btBulletWorldImporter.h"
	
	#include "Recast.h"
	#include "DetourDebugDraw.h"
	#include "DetourNavMesh.h"
	#include "DetourNavMeshBuilder.h"
	
	#include "stb_truetype.h"

	#include "vorbisfile.h"

#else // Android

	#include <jni.h>
	#include <android/log.h>

	#include <EGL/egl.h>
	#include "GLES2/gl2.h"
	#include "GLES2/gl2ext.h"

	#include "png/png.h"

	#include "zlib/zlib.h"
	#include "zlib/unzip.h"

	#include "nvtristrip/NvTriStrip.h"

	#include "bullet/btAlignedAllocator.h"
	#include "bullet/btBulletDynamicsCommon.h"
	#include "bullet/btSoftRigidDynamicsWorld.h"
	#include "bullet/btSoftBodyRigidBodyCollisionConfiguration.h"
	#include "bullet/btShapeHull.h"
	#include "bullet/btSoftBodyHelpers.h"
	#include "bullet/btSoftBody.h"
	#include "bullet/btGImpactShape.h"
	#include "bullet/btGImpactCollisionAlgorithm.h"
	#include "bullet/btBulletWorldImporter.h"

	#include "recast/Recast.h"
	#include "detour/DetourDebugDraw.h"
	#include "detour/DetourNavMesh.h"
	#include "detour/DetourNavMeshBuilder.h"

	#include "ttf/stb_truetype.h"

	#include "openal/al.h"
	#include "openal/alc.h"

	#include "vorbis/vorbisfile.h"


	extern PFNGLBINDVERTEXARRAYOESPROC glBindVertexArrayOES;
	extern PFNGLGENVERTEXARRAYSOESPROC glGenVertexArraysOES;
	extern PFNGLDELETEVERTEXARRAYSOESPROC glDeleteVertexArraysOES;

#endif

#include <vector>
#include <map>
#include <memory>
#include <string>

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <stdarg.h>
#include <sys/time.h>
#include <unistd.h>

#include "glml.h"
#include "tstack.h"

#include "types.h"
#include "thread.h"
#include "utils.h"
#include "memory.h"
#include "shader.h"
#include "program.h"
#include "texture.h"
#include "obj.h"
#include "navigation.h"
#include "font.h"
#include "audio.h"
#include "sound.h"
#include "light.h"
#include "md5.h"


#define MAX_MODELVIEW_MATRIX	32

#define MAX_PROJECTION_MATRIX	2

#define MAX_TEXTURE_MATRIX		2


enum
{
	MODELVIEW_MATRIX	= 0,
	PROJECTION_MATRIX	= 1,
	TEXTURE_MATRIX		= 2
};


struct GFX {
    unsigned char	matrix_mode;

    TStack              modelview_matrix;

    TStack		projection_matrix;

    TStack		texture_matrix;

    mat4		modelview_projection_matrix;
    
    mat3		normal_matrix;

public:
    GFX();
};

extern GFX *gfx;

void GFX_start( void );

void GFX_error( void );

void GFX_set_matrix_mode( unsigned int mode );

void GFX_load_identity( void );

void GFX_push_matrix( void );

void GFX_pop_matrix( void );

void GFX_load_matrix(const mat4 &m);

void GFX_multiply_matrix(const mat4 &m);

void GFX_translate(const float x, const float y, const float z);

void GFX_translate(const vec3 &t);

void GFX_rotate(const float angle, const float x, const float y, const float z);

void GFX_rotate(const quaternion &q);

void GFX_scale(const float x, const float y, const float z);

void GFX_scale(const vec3 &v);

mat4 &GFX_get_modelview_matrix(const int i=INT_MAX);

mat4 &GFX_get_projection_matrix( void );

mat4 &GFX_get_texture_matrix( void );

mat4 &GFX_get_modelview_projection_matrix( void );

mat3 &GFX_get_normal_matrix( void );

void GFX_ortho(const float left, const float right,
               const float bottom, const float top,
               const float clip_start, const float clip_end);

void GFX_set_orthographic_2d(const float left, const float right,
                             const float bottom, const float top);

void GFX_set_orthographic(const float screen_ratio, float scale,
                          const float aspect_ratio,
                          const float clip_start, const float clip_end,
                          const float screen_orientation);

void GFX_set_perspective(const float fovy, const float aspect_ratio,
                         const float clip_start, const float clip_end,
                         const float screen_orientation);

void GFX_look_at(const vec3 &eye, const vec3 &center, const vec3 &up);

void GFX_clear_color( float r, float g, float b, float a );

bool GFX_project(float objx, float objy, float objz,
                 mat4 *modelview_matrix,
                 mat4 *projection_matrix,
                 int *viewport_matrix,
                 float *winx, float *winy, float *winz );

bool GFX_project(const vec3 &obj,
                 const mat4 &modelview_matrix,
                 const mat4 &projection_matrix,
                 const int *viewport_matrix,
                 float *winx, float *winy, float *winz);

bool GFX_unproject(const float winx, const float winy, const float winz,
                   const mat4 &modelview_matrix,
                   const mat4 &projection_matrix,
                   const int *viewport_matrix,
                   float *objx, float *objy, float *objz);

bool GFX_unproject(const float winx, const float winy, const float winz,
                   const mat4 &modelview_matrix,
                   const mat4 &projection_matrix,
                   const int *viewport_matrix,
                   vec3 &obj);

#endif
