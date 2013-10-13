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

#ifndef OBJ_H
#define OBJ_H

typedef void( MATERIALDRAWCALLBACK( void * ) );


typedef struct
{
	char					name[ MAX_CHAR ];				// newmtl
	
	vec4					ambient;						// Ka
	
	vec4					diffuse;						// Kd
	
	vec4					specular;						// Ks
	
	vec3					transmission_filter;			// Tf
	
	int						illumination_model;				// illum
	
	float					dissolve;						// d
	
	float					specular_exponent;				// Ns
	
	float					optical_density;				// Ni
	
	char					map_ambient[ MAX_CHAR ];		// map_Ka
	
	char					map_diffuse[ MAX_CHAR ];		// map_Kd
	
	char					map_specular[ MAX_CHAR ];		// map_Ks

	char					map_translucency[ MAX_CHAR ];	// map_Tr
		
	char					map_disp[ MAX_CHAR ];			// disp or map_disp
	
	char					map_bump[ MAX_CHAR ];			// bump or map_bump	
	
	TEXTURE					*texture_ambient;
	
	TEXTURE					*texture_diffuse;
	
	TEXTURE					*texture_specular;

	TEXTURE					*texture_translucency;
		
	TEXTURE					*texture_disp;
	
	TEXTURE					*texture_bump;
	
	PROGRAM					*program;
	
	MATERIALDRAWCALLBACK	*materialdrawcallback;
	
} OBJMATERIAL;


typedef struct
{
	int vertex_index[ 3 ];
	
	int uv_index[ 3 ];

} OBJTRIANGLEINDEX;


typedef struct
{
	unsigned int	 n_objtriangleindex;
	
	OBJTRIANGLEINDEX *objtriangleindex;

	unsigned char	 useuvs;
	
	unsigned short	 n_indice_array;
	
	unsigned short   *indice_array;
	
	OBJMATERIAL		 *objmaterial;
	
	int				 mode;
	
	unsigned int	 vbo;

} OBJTRIANGLELIST;


typedef struct
{
	int vertex_index;
	
	int uv_index;

} OBJVERTEXDATA;


typedef struct
{
	char			name[ MAX_CHAR ];  // o
	
	unsigned char	visible;
		
	char			group[ MAX_CHAR ]; // g

	unsigned short	n_objvertexdata;
	
	OBJVERTEXDATA	*objvertexdata;
	
	unsigned char	n_objtrianglelist;
	
	OBJTRIANGLELIST	*objtrianglelist;
	
	OBJMATERIAL		*current_material;
	
	vec3			location;
	
	vec3			rotation;
	
	vec3			scale;
	
	vec3			min;
	
	vec3			max;
	
	vec3			dimension;
	
	float			radius;	

	float			distance;
	
	unsigned int	vbo;
	
	unsigned int	stride;
	
	unsigned int	size;
	
	unsigned int	offset[ 5 ];
	
	unsigned int	vao;	
	
	btRigidBody		*btrigidbody;
	
	unsigned char	use_smooth_normals;

} OBJMESH;


typedef struct
{
	char			texture_path[ MAX_PATH ];

	char			program_path[ MAX_PATH ];
	
	unsigned int	n_objmesh;
	
	OBJMESH			*objmesh;
	
	unsigned int	n_objmaterial;
	
	OBJMATERIAL		*objmaterial;
	
	unsigned int	n_texture;
	
	TEXTURE			**texture;

	unsigned int	n_program;
	
	PROGRAM			**program;
	
	unsigned int	n_indexed_vertex;
	
	vec3			*indexed_vertex;	// v

	vec3			*indexed_normal;

	vec3			*indexed_fnormal;

	vec3			*indexed_tangent;

	unsigned int	n_indexed_uv;
	
	vec2			*indexed_uv;		// vt

} OBJ;


void OBJ_build_texture( OBJ *obj, unsigned int texture_index, char *texture_path, unsigned int flags, unsigned char filter, float anisotropic_filter );

void OBJ_build_program( OBJ	*obj, unsigned int program_index, PROGRAMBINDATTRIBCALLBACK *programbindattribcallback, PROGRAMDRAWCALLBACK *programdrawcallback, unsigned char debug_shader, char *program_path );

void OBJ_build_material( OBJ *obj, unsigned int material_index, PROGRAM	*program );

void OBJ_set_draw_callback_material( OBJ *obj, unsigned int material_index, MATERIALDRAWCALLBACK *materialdrawcallback );

void OBJ_update_bound_mesh( OBJ *obj, unsigned int mesh_index );

void OBJ_build_vbo_mesh( OBJ *obj, unsigned int mesh_index );

void OBJ_set_attributes_mesh( OBJ *obj, unsigned int mesh_index );

void OBJ_build_mesh( OBJ *obj, unsigned int mesh_index );

void OBJ_build_mesh2( OBJ *obj, unsigned int mesh_index );

void OBJ_optimize_mesh( OBJ *obj, unsigned int mesh_index, unsigned int vertex_cache_size );

OBJMESH *OBJ_get_mesh( OBJ *obj, const char *name, unsigned char exact_name );

int OBJ_get_mesh_index( OBJ *obj, const char *name, unsigned char exact_name );

PROGRAM *OBJ_get_program( OBJ *obj, const char *name, unsigned char exact_name );

OBJMATERIAL *OBJ_get_material( OBJ *obj, const char *name, unsigned char exact_name );

TEXTURE *OBJ_get_texture( OBJ *obj, const char *name, unsigned char exact_name );

void OBJ_draw_material( OBJMATERIAL *objmaterial );

void OBJ_draw_mesh( OBJ *obj, unsigned int mesh_index );

void OBJ_draw_mesh2( OBJ *obj, OBJMESH *objmesh );

void OBJ_draw_mesh3( OBJ *obj, OBJMESH *objmesh );

void OBJ_free_mesh_vertex_data( OBJ *obj, unsigned int mesh_index );

unsigned char OBJ_load_mtl( OBJ *obj, char *filename, unsigned char relative_path );

OBJ *OBJ_load( char *filename, unsigned char relative_path );

void OBJ_free_vertex_data( OBJ *obj );

OBJ *OBJ_free( OBJ *obj );

#endif
