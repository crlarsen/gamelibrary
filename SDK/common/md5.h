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

#ifndef MD5_H
#define MD5_H


enum
{
	MD5_METHOD_FRAME = 0,
	MD5_METHOD_LERP  = 1,
	MD5_METHOD_SLERP = 2
};


typedef struct
{
	char			name[ MAX_CHAR ];
	
	int				parent;
	
	vec3			location;
	
	vec4			rotation;
  
} MD5JOINT;


typedef struct
{
	vec2			uv;
	
	vec3			normal;
	
	vec3			tangent;
	
	unsigned int	start;
	
	unsigned int	count;

} MD5VERTEX;


typedef struct
{
	unsigned short indice[ 3 ];

} MD5TRIANGLE;


typedef struct
{
	int		joint;

	float	bias;

	vec3	location;
	
	vec3	normal;
	
	vec3	tangent;
	
} MD5WEIGHT;


typedef struct
{
	char			shader[ MAX_CHAR ];
	
	unsigned int	n_vertex;

	MD5VERTEX		*md5vertex;

	unsigned int	vbo;

	unsigned int	size;

	unsigned int	stride;

	unsigned int	offset[ 4 ];

	unsigned char	*vertex_data;	
			
	unsigned int	n_triangle;

	MD5TRIANGLE		*md5triangle;

	unsigned int	mode;

	unsigned short	n_indice;
	
	unsigned short  *indice;

	unsigned int	vbo_indice;

	unsigned int	n_weight;

	MD5WEIGHT		*md5weight;
	
	unsigned int	vao;
	
	unsigned char	visible;
	
	OBJMATERIAL		*objmaterial;
	
} MD5MESH;


typedef struct
{
	char			name[ MAX_CHAR ];
	
	unsigned int	n_frame;
	
	MD5JOINT		**frame;
	
	MD5JOINT		*pose;
	
	int				curr_frame;
	
	int				next_frame;
	
	unsigned char	state;
	
	unsigned char	method;
	
	unsigned char	loop;
	
	float			frame_time;
	
	float			fps;

} MD5ACTION;


typedef struct
{
	char			name[ MAX_CHAR ];

	unsigned char	visible;
	
	unsigned int	n_joint;

	MD5JOINT		*bind_pose;

	unsigned int	n_mesh;

	MD5MESH			*md5mesh;
	
	unsigned int	n_action;
	 
	MD5ACTION		*md5action;	
	
	vec3			location;
	
	vec3			rotation;
	
	vec3			scale;
	
	vec3			min;
	
	vec3			max;
	
	vec3			dimension;
	
	float			radius;	
	
	float			distance;
	
	btRigidBody		*btrigidbody;
	
} MD5;


MD5 *MD5_load_mesh( char *filename, unsigned char relative_path );

int MD5_load_action( MD5 *md5, char *name, char *filename, unsigned char relative_path );

MD5 *MD5_free( MD5 *md5 );

void MD5_free_mesh_data( MD5 *md5 );

MD5ACTION *MD5_get_action( MD5 *md5, char *name, unsigned char exact_name );

MD5MESH *MD5_get_mesh( MD5 *md5, char *name, unsigned char exact_name );

void MD5_action_play( MD5ACTION *md5action, unsigned char frame_interpolation_method, unsigned char loop );

void MD5_action_pause( MD5ACTION *md5action );

void MD5_action_stop( MD5ACTION *md5action );

void MD5_set_action_fps( MD5ACTION *md5action, float fps );

void MD5_set_mesh_attributes( MD5MESH *md5mesh );

void MD5_set_mesh_visibility( MD5MESH *md5mesh, unsigned char visible );

void MD5_set_mesh_material( MD5MESH *md5mesh, OBJMATERIAL *objmaterial );

void MD5_optimize( MD5 *md5, unsigned int vertex_cache_size );

void MD5_build_vbo( MD5 *md5, unsigned int mesh_index );

void MD5_build_bind_pose_weighted_normals_tangents( MD5 *md5 );

void MD5_set_pose( MD5 *md5, MD5JOINT *pose );

void MD5_blend_pose( MD5 *md5, MD5JOINT *final_pose, MD5JOINT *pose0, MD5JOINT *pose1, unsigned char joint_interpolation_method, float blend );

void MD5_add_pose( MD5 *md5, MD5JOINT *final_pose, MD5ACTION *action0, MD5ACTION *action1, unsigned char joint_interpolation_method, float action_weight );

void MD5_build( MD5 *md5 );

void MD5_build2( MD5 *md5 );

unsigned char MD5_draw_action( MD5 *md5, float time_step );

void MD5_draw( MD5 *md5 );

#endif
