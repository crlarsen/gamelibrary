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

void OBJMESH_add_vertex_data( OBJMESH		  *objmesh,
							  OBJTRIANGLELIST *objtrianglelist,
							  int			  vertex_index,
							  int			  uv_index )
{
	unsigned short index = 0;
	
	while( index != objmesh->n_objvertexdata )
	{
		if( vertex_index == objmesh->objvertexdata[ index ].vertex_index )
		{
			if( uv_index == -1 ) goto add_index_to_triangle_list;
			
			else if( uv_index == objmesh->objvertexdata[ index ].uv_index ) goto add_index_to_triangle_list;
		}
		
		++index;
	}
	
	index = objmesh->n_objvertexdata;
	
	++objmesh->n_objvertexdata;
	
	objmesh->objvertexdata = ( OBJVERTEXDATA * ) realloc( objmesh->objvertexdata, 
														  objmesh->n_objvertexdata *
														  sizeof( OBJVERTEXDATA ) );
	
	objmesh->objvertexdata[ index ].vertex_index = vertex_index;
	objmesh->objvertexdata[ index ].uv_index	   = uv_index;


add_index_to_triangle_list:
	
	++objtrianglelist->n_indice_array;
	
	objtrianglelist->indice_array = ( unsigned short * ) realloc( objtrianglelist->indice_array,
																  objtrianglelist->n_indice_array *
																  sizeof( unsigned short ) );

	objtrianglelist->indice_array[ objtrianglelist->n_indice_array - 1 ] = index;	
}


int OBJ_get_texture_index( OBJ  *obj,
						   char *filename )
{
	unsigned int i = 0;
	
	while( i != obj->n_texture )
	{
		if( !strcmp( filename, obj->texture[ i ]->name ) ) return i;
	
		++i;
	}
	
	return -1;
}


void OBJ_add_texture( OBJ  *obj,
					  char *filename )
{
	if( OBJ_get_texture_index( obj, filename ) != -1 ) return;

	++obj->n_texture;

	obj->texture = ( TEXTURE ** ) realloc( obj->texture,
										   obj->n_texture *
										   sizeof( TEXTURE * ) );
	
	obj->texture[ obj->n_texture - 1 ] = TEXTURE_init( filename );
}


int OBJ_get_program_index( OBJ  *obj,
						   char *filename )
{
	unsigned int i = 0;
	
	while( i != obj->n_program )
	{
		if( !strcmp( filename, obj->program[ i ]->name ) ) return i;
	
		++i;
	}
	
	return -1;
}


void OBJ_add_program( OBJ  *obj,
					  char *filename )
{
	if( OBJ_get_program_index( obj, filename ) != -1 ) return;

	++obj->n_program;

	obj->program = ( PROGRAM ** ) realloc( obj->program,
										   obj->n_program *
										   sizeof( PROGRAM * ) );
	
	obj->program[ obj->n_program - 1 ] = PROGRAM_init( filename );
}


void OBJ_build_texture( OBJ			  *obj,
						unsigned int  texture_index,
						char		  *texture_path,
						unsigned int  flags,
						unsigned char filter,
						float		  anisotropic_filter )
{
	TEXTURE *texture = obj->texture[ texture_index ];

	MEMORY *m = NULL;
	
	char filename[ MAX_PATH ] = {""};
	
	sprintf( filename, "%s%s", texture_path, texture->name  );
	
	m = mopen( filename, 0 );
	
	if( m )
	{
		TEXTURE_load( texture, m );
		
		TEXTURE_generate_id( texture,
							 flags,
							 filter,
							 anisotropic_filter );
					 
		TEXTURE_free_texel_array( texture );
		
		mclose( m );
	}
}


void OBJ_build_program( OBJ							*obj,
						unsigned int				program_index,
						PROGRAMBINDATTRIBCALLBACK	*programbindattribcallback,
						PROGRAMDRAWCALLBACK			*programdrawcallback,
						unsigned char				debug_shader,
						char						*program_path )
{
	PROGRAM *program = obj->program[ program_index ];
	
	char filename[ MAX_PATH ] = {""};
	
	sprintf( filename, "%s%s", program_path, program->name );

	PROGRAM_load_gfx( program,
					  programbindattribcallback,
					  programdrawcallback,
					  filename,
					  debug_shader,
					  0 );
}


void OBJ_build_material( OBJ		  *obj,
						 unsigned int material_index,
						 PROGRAM	  *program )
{
	int index;
	
	OBJMATERIAL *objmaterial = &obj->objmaterial[ material_index ];

	char ext[ MAX_CHAR ] = {""};


	if( objmaterial->map_ambient[ 0 ] )
	{
		get_file_extension( objmaterial->map_ambient, ext, 1 );

		if( !strcmp( ext, "GFX" ) )
		{
			index = OBJ_get_program_index( obj, objmaterial->map_ambient );
			
			if( index != -1 ) objmaterial->program = obj->program[ index ];
		}
		else
		{
			index = OBJ_get_texture_index( obj, objmaterial->map_ambient );
			
			if( index != -1 ) objmaterial->texture_ambient = obj->texture[ index ];
		}
	}


	if( objmaterial->map_diffuse[ 0 ] )
	{
		get_file_extension( objmaterial->map_diffuse, ext, 1 );

		if( !strcmp( ext, "GFX" ) )
		{
			index = OBJ_get_program_index( obj, objmaterial->map_diffuse );
			
			if( index != -1 ) objmaterial->program = obj->program[ index ];
		}
		else
		{
			index = OBJ_get_texture_index( obj, objmaterial->map_diffuse );
			
			if( index != -1 ) objmaterial->texture_diffuse = obj->texture[ index ];
		}
	}


	if( objmaterial->map_specular[ 0 ] )
	{
		get_file_extension( objmaterial->map_specular, ext, 1 );

		if( !strcmp( ext, "GFX" ) )
		{
			index = OBJ_get_program_index( obj, objmaterial->map_specular );
			
			if( index != -1 ) objmaterial->program = obj->program[ index ];
		}
		else
		{
			index = OBJ_get_texture_index( obj, objmaterial->map_specular );
			
			if( index != -1 ) objmaterial->texture_specular = obj->texture[ index ];
		}
	}


	if( objmaterial->map_translucency[ 0 ] )
	{
		get_file_extension( objmaterial->map_translucency, ext, 1 );

		if( !strcmp( ext, "GFX" ) )
		{
			index = OBJ_get_program_index( obj, objmaterial->map_translucency );
			
			if( index != -1 ) objmaterial->program = obj->program[ index ];
		}
		else
		{
			index = OBJ_get_texture_index( obj, objmaterial->map_translucency );
			
			if( index != -1 ) objmaterial->texture_translucency = obj->texture[ index ];
		}
	}


	if( objmaterial->map_disp[ 0 ] )
	{
		get_file_extension( objmaterial->map_disp, ext, 1 );

		if( !strcmp( ext, "GFX" ) )
		{
			index = OBJ_get_program_index( obj, objmaterial->map_disp );
			
			if( index != -1 ) objmaterial->program = obj->program[ index ];
		}
		else
		{
			index = OBJ_get_texture_index( obj, objmaterial->map_disp );
			
			if( index != -1 ) objmaterial->texture_disp = obj->texture[ index ];
		}
	}


	if( objmaterial->map_bump[ 0 ] )
	{
		get_file_extension( objmaterial->map_bump, ext, 1 );

		if( !strcmp( ext, "GFX" ) )
		{
			index = OBJ_get_program_index( obj, objmaterial->map_bump );
			
			if( index != -1 ) objmaterial->program = obj->program[ index ];
		}
		else
		{
			index = OBJ_get_texture_index( obj, objmaterial->map_bump );
			
			if( index != -1 ) objmaterial->texture_bump = obj->texture[ index ];
		}
	}
	

	if( program ) objmaterial->program = program;
}


void OBJ_set_draw_callback_material( OBJ *obj, unsigned int material_index, MATERIALDRAWCALLBACK *materialdrawcallback )
{ obj->objmaterial[ material_index ].materialdrawcallback = materialdrawcallback; }


void OBJ_update_bound_mesh( OBJ *obj, unsigned int mesh_index )
{
	unsigned int i, index;
	
	OBJMESH *objmesh = &obj->objmesh[ mesh_index ];

	// Get the mesh min and max.
	objmesh->min.x =
	objmesh->min.y =
	objmesh->min.z = 99999.999f;

	objmesh->max.x =
	objmesh->max.y =
	objmesh->max.z = -99999.999f;
	
	
	i = 0;
	while( i != objmesh->n_objvertexdata )
	{
		index = objmesh->objvertexdata[ i ].vertex_index;
		
		if( obj->indexed_vertex[ index ].x < objmesh->min.x ) objmesh->min.x = obj->indexed_vertex[ index ].x;
		if( obj->indexed_vertex[ index ].y < objmesh->min.y ) objmesh->min.y = obj->indexed_vertex[ index ].y;
		if( obj->indexed_vertex[ index ].z < objmesh->min.z ) objmesh->min.z = obj->indexed_vertex[ index ].z;

		if( obj->indexed_vertex[ index ].x > objmesh->max.x ) objmesh->max.x = obj->indexed_vertex[ index ].x;
		if( obj->indexed_vertex[ index ].y > objmesh->max.y ) objmesh->max.y = obj->indexed_vertex[ index ].y;
		if( obj->indexed_vertex[ index ].z > objmesh->max.z ) objmesh->max.z = obj->indexed_vertex[ index ].z;
	
		++i;
	}


	// Mesh location
	vec3_mid( &objmesh->location,
			  &objmesh->min,
			  &objmesh->max );


	// Mesh dimension
	vec3_diff( &objmesh->dimension,
			   &objmesh->max,
			   &objmesh->min );


	// Bounding sphere radius
	objmesh->radius = objmesh->dimension.x > objmesh->dimension.y ?
					  objmesh->dimension.x:
					  objmesh->dimension.y;
	
	objmesh->radius = objmesh->radius > objmesh->dimension.z ?
					  objmesh->radius * 0.5f:
					  objmesh->dimension.z * 0.5f;
	
	/*
	objmesh->radius = vec3_dist( &objmesh->min,
								 &objmesh->max ) * 0.5f;
	*/
}


void OBJ_build_vbo_mesh( OBJ *obj, unsigned int mesh_index )
{
	// Build the VBO for the vertex data
	unsigned int i,
				 index,
				 offset;
	
	OBJMESH *objmesh = &obj->objmesh[ mesh_index ];
	
	objmesh->stride  = sizeof( vec3 ); // Vertex
	objmesh->stride += sizeof( vec3 ); // Normals
	objmesh->stride += sizeof( vec3 ); // Face Normals
		
	if( objmesh->objvertexdata[ 0 ].uv_index != -1 )
	{
		objmesh->stride += sizeof( vec3 ); // Tangent
		objmesh->stride += sizeof( vec2 ); // Uv
	}
	
	objmesh->size = objmesh->n_objvertexdata * objmesh->stride;
	
	unsigned char *vertex_array = ( unsigned char * ) malloc( objmesh->size ),
				  *vertex_start = vertex_array;

	i = 0;
	while( i != objmesh->n_objvertexdata )
	{ 
		index = objmesh->objvertexdata[ i ].vertex_index;
		
		memcpy( vertex_array,
				&obj->indexed_vertex[ index ],
				sizeof( vec3 ) );

		// Center the pivot
		vec3_diff( ( vec3 * )vertex_array,
				   ( vec3 * )vertex_array,
				   &objmesh->location );

		vertex_array += sizeof( vec3 );


		memcpy( vertex_array,
				&obj->indexed_normal[ index ],
				sizeof( vec3 ) );
		
		vertex_array += sizeof( vec3 );


		memcpy( vertex_array,
				&obj->indexed_fnormal[ index ],
				sizeof( vec3 ) );
		
		vertex_array += sizeof( vec3 );

	
		if( objmesh->objvertexdata[ 0 ].uv_index != -1 )
		{
			memcpy( vertex_array,
					&obj->indexed_uv[ objmesh->objvertexdata[ i ].uv_index ],
					sizeof( vec2 ) );

			vertex_array += sizeof( vec2 );
			
			memcpy( vertex_array,
					&obj->indexed_tangent[ index ],
					sizeof( vec3 ) );

			vertex_array += sizeof( vec3 );				
		}
					
		++i;
	}
	
	
	glGenBuffers( 1, &objmesh->vbo );
	
	glBindBuffer( GL_ARRAY_BUFFER, objmesh->vbo );
	
	glBufferData( GL_ARRAY_BUFFER,
				  objmesh->size,
				  vertex_start,
				  GL_STATIC_DRAW );	
	
	free( vertex_start );
	

	objmesh->offset[ 0 ] = 0;
			
	offset = sizeof( vec3 );

	objmesh->offset[ 1 ] = offset;
			
	offset += sizeof( vec3 );

	objmesh->offset[ 2 ] = offset;
			
	offset += sizeof( vec3 );

	
	if( objmesh->objvertexdata[ 0 ].uv_index != -1 )
	{
		objmesh->offset[ 3 ] = offset;
		
		offset += sizeof( vec2 );

		objmesh->offset[ 4 ] = offset;
	}
		
	
	i = 0;
	while( i != objmesh->n_objtrianglelist )
	{
		glGenBuffers( 1, &objmesh->objtrianglelist[ i ].vbo );
		
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, objmesh->objtrianglelist[ i ].vbo );
		
		glBufferData( GL_ELEMENT_ARRAY_BUFFER,
					  objmesh->objtrianglelist[ i ].n_indice_array * sizeof( unsigned short ),
					  objmesh->objtrianglelist[ i ].indice_array,
					  GL_STATIC_DRAW );
		++i;
	}
}


void OBJ_set_attributes_mesh( OBJ *obj, unsigned int mesh_index )
{
	OBJMESH *objmesh = &obj->objmesh[ mesh_index ];

	glBindBuffer( GL_ARRAY_BUFFER, objmesh->vbo );			

	glEnableVertexAttribArray( 0 );
	
	glVertexAttribPointer( 0,
						   3,
						   GL_FLOAT,
						   GL_FALSE,
						   objmesh->stride,
						   ( void * )NULL );


	glEnableVertexAttribArray( 1 );

	glVertexAttribPointer( 1,
						   3,
						   GL_FLOAT,
						   GL_FALSE, 
						   objmesh->stride,
						   BUFFER_OFFSET( objmesh->offset[ 1 ] ) );


	glEnableVertexAttribArray( 4 );

	glVertexAttribPointer( 4,
						   3,
						   GL_FLOAT,
						   GL_FALSE, 
						   objmesh->stride,
						   BUFFER_OFFSET( objmesh->offset[ 2 ] ) );


	if( objmesh->offset[ 3 ] != -1 )
	{
		glEnableVertexAttribArray( 2 );

		glVertexAttribPointer( 2,
							   2,
							   GL_FLOAT,
							   GL_FALSE,
							   objmesh->stride,
							   BUFFER_OFFSET( objmesh->offset[ 3 ] ) );

		glEnableVertexAttribArray( 3 );

		glVertexAttribPointer( 3,
							   3,
							   GL_FLOAT,
							   GL_FALSE, 
							   objmesh->stride,
							   BUFFER_OFFSET( objmesh->offset[ 4 ] ) );
	}
}



void OBJ_build_mesh( OBJ *obj, unsigned int mesh_index )
{
	OBJMESH *objmesh = &obj->objmesh[ mesh_index ];


	OBJ_update_bound_mesh( obj, mesh_index );
	
	OBJ_build_vbo_mesh( obj, mesh_index );
	

	glGenVertexArraysOES( 1, &objmesh->vao );
	
	glBindVertexArrayOES( objmesh->vao );
	
	
	OBJ_set_attributes_mesh( obj, mesh_index );
	
	
	if( objmesh->n_objtrianglelist == 1 )
	{ glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, objmesh->objtrianglelist[ 0 ].vbo ); }
	
	
	glBindVertexArrayOES( 0 );
}


void OBJ_build_mesh2( OBJ *obj, unsigned int mesh_index )
{
	OBJ_update_bound_mesh( obj, mesh_index );
	
	OBJ_build_vbo_mesh( obj, mesh_index );
}


void OBJ_optimize_mesh( OBJ *obj, unsigned int mesh_index, unsigned int vertex_cache_size )
{
	OBJMESH *objmesh = &obj->objmesh[ mesh_index ];
	
	unsigned int i = 0,
				 s = 0;
	
	unsigned short n_group = 0;

	if( vertex_cache_size ) SetCacheSize( vertex_cache_size );
	
	while( i != objmesh->n_objtrianglelist )
	{
		PrimitiveGroup *primitivegroup;
	
		if( GenerateStrips( objmesh->objtrianglelist[ i ].indice_array,
							objmesh->objtrianglelist[ i ].n_indice_array,
							&primitivegroup,
							&n_group,
							true ) )
		{
			if( primitivegroup[ 0 ].numIndices < objmesh->objtrianglelist[ i ].n_indice_array )
			{
				objmesh->objtrianglelist[ i ].mode = GL_TRIANGLE_STRIP;
				objmesh->objtrianglelist[ i ].n_indice_array = primitivegroup[ 0 ].numIndices;
			
				s = primitivegroup[ 0 ].numIndices * sizeof( unsigned short );
						
				objmesh->objtrianglelist[ i ].indice_array = ( unsigned short * ) realloc( objmesh->objtrianglelist[ i ].indice_array,
																						   s );
				
				memcpy( &objmesh->objtrianglelist[ i ].indice_array[ 0 ],
						&primitivegroup[ 0 ].indices[ 0 ],
						s );
			}
			
			delete[] primitivegroup;
		}

		++i;
	}
}


OBJMESH *OBJ_get_mesh( OBJ *obj, const char *name, unsigned char exact_name )
{
	unsigned int i = 0;
	
	while( i != obj->n_objmesh )
	{
		if( exact_name )
		{ if( !strcmp( obj->objmesh[ i ].name, name ) ) return &obj->objmesh[ i ]; }
		
		else
		{ if( strstr( obj->objmesh[ i ].name, name ) ) return &obj->objmesh[ i ]; }
	
		++i;
	}
	
	return NULL;
}


int OBJ_get_mesh_index( OBJ *obj, const char *name, unsigned char exact_name )
{
	unsigned int i = 0;
	
	while( i != obj->n_objmesh )
	{
		if( exact_name )
		{ if( !strcmp( obj->objmesh[ i ].name, name ) ) return i; }
		
		else
		{ if( strstr( obj->objmesh[ i ].name, name ) ) return i; }
	
		++i;
	}
	
	return -1;
}


PROGRAM *OBJ_get_program( OBJ *obj, const char *name, unsigned char exact_name )
{
	unsigned int i = 0;
	
	while( i != obj->n_program )
	{
		if( exact_name )
		{ if( !strcmp( obj->program[ i ]->name, name ) ) return obj->program[ i ]; }
		
		else
		{ if( strstr( obj->program[ i ]->name, name ) ) return obj->program[ i ]; }
	
		++i;
	}
	
	return NULL;
}


OBJMATERIAL *OBJ_get_material( OBJ *obj, const char *name, unsigned char exact_name )
{
	unsigned int i = 0;
	
	while( i != obj->n_objmaterial )
	{
		if( exact_name )
		{ if( !strcmp( obj->objmaterial[ i ].name, name ) ) return &obj->objmaterial[ i ]; }
		
		else
		{ if( strstr( obj->objmaterial[ i ].name, name ) ) return &obj->objmaterial[ i ]; }
	
		++i;
	}
	
	return NULL;
}


TEXTURE *OBJ_get_texture( OBJ *obj, const char *name, unsigned char exact_name )
{
	unsigned int i = 0;
	
	while( i != obj->n_texture )
	{
		if( exact_name )
		{ if( !strcmp( obj->texture[ i ]->name, name ) ) return obj->texture[ i ]; }
		
		else
		{ if( strstr(obj->texture[ i ]->name, name ) ) return obj->texture[ i ]; }
	
		++i;
	}
	
	return NULL;
}


void OBJ_draw_material( OBJMATERIAL *objmaterial )
{
	if( objmaterial )
	{
		if( objmaterial->program ) PROGRAM_draw( objmaterial->program );


		if( objmaterial->texture_ambient )
		{
			glActiveTexture( GL_TEXTURE0 );

			TEXTURE_draw( objmaterial->texture_ambient );
		}

		
		if( objmaterial->texture_diffuse )
		{
			glActiveTexture( GL_TEXTURE1 );

			TEXTURE_draw( objmaterial->texture_diffuse );
		}

		
		if( objmaterial->texture_specular )
		{
			glActiveTexture( GL_TEXTURE2 );

			TEXTURE_draw( objmaterial->texture_specular );
		}


		if( objmaterial->texture_disp )
		{
			glActiveTexture( GL_TEXTURE3 );

			TEXTURE_draw( objmaterial->texture_disp );
		}	
		
		
		if( objmaterial->texture_bump )
		{
			glActiveTexture( GL_TEXTURE4 );

			TEXTURE_draw( objmaterial->texture_bump );
		}


		if( objmaterial->texture_translucency )
		{
			glActiveTexture( GL_TEXTURE5 );

			TEXTURE_draw( objmaterial->texture_translucency );
		}
		

		if( objmaterial->materialdrawcallback ) objmaterial->materialdrawcallback( objmaterial );
	}
}


void OBJ_draw_mesh( OBJ *obj, unsigned int mesh_index )
{
	OBJMESH *objmesh = &obj->objmesh[ mesh_index ];

	if( objmesh->visible && objmesh->distance )
	{
		unsigned int i = 0;
		
		if( objmesh->vao ) glBindVertexArrayOES( objmesh->vao );

		else OBJ_set_attributes_mesh( obj, mesh_index );
		
		
		while( i != objmesh->n_objtrianglelist )
		{
			objmesh->current_material = objmesh->objtrianglelist[ i ].objmaterial;
		
			if( objmesh->current_material ) OBJ_draw_material( objmesh->current_material );
			
			if( objmesh->vao )
			{
				if( objmesh->n_objtrianglelist != 1 )
				{ glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, objmesh->objtrianglelist[ i ].vbo ); }
			}
			else
			{ glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, objmesh->objtrianglelist[ i ].vbo ); }
  			
			
			glDrawElements( objmesh->objtrianglelist[ i ].mode,
							objmesh->objtrianglelist[ i ].n_indice_array,
							GL_UNSIGNED_SHORT,
							( void * )NULL );
			
			++i;
		}
	}
}


void OBJ_draw_mesh2( OBJ *obj, OBJMESH *objmesh )
{
	unsigned int i = 0;
	
	while( i != obj->n_objmesh )
	{
		if( &obj->objmesh[ i ] == objmesh )
		{
			OBJ_draw_mesh( obj, i );
			return;
		}
		
		++i;
	}
}


void OBJ_draw_mesh3( OBJ *obj, OBJMESH *objmesh )
{
	unsigned int i = 0;
	
	while( i != obj->n_objmesh )
	{
		if( &obj->objmesh[ i ] == objmesh )
		{
			GFX_push_matrix();
			
			GFX_translate( objmesh->location.x,
						   objmesh->location.y,
						   objmesh->location.z );

			GFX_rotate( objmesh->rotation.z, 0.0f, 0.0f, 1.0f );
			GFX_rotate( objmesh->rotation.y, 0.0f, 1.0f, 0.0f );
			GFX_rotate( objmesh->rotation.x, 1.0f, 0.0f, 0.0f );
			
			GFX_scale( objmesh->scale.x,
					   objmesh->scale.y,
					   objmesh->scale.z );

			OBJ_draw_mesh( obj, i );
			
			GFX_pop_matrix();
			
			return;
		}
		
		++i;
	}
}


void OBJ_free_mesh_vertex_data( OBJ *obj, unsigned int mesh_index )
{
	unsigned int i = 0;
	
	OBJMESH *objmesh = &obj->objmesh[ mesh_index ];

	if( objmesh->objvertexdata )
	{
		free( objmesh->objvertexdata );
		objmesh->objvertexdata = NULL;
	}
	
	objmesh->n_objvertexdata = 0;
	
	while( i != objmesh->n_objtrianglelist )
	{
		free( objmesh->objtrianglelist[ i ].objtriangleindex );
		objmesh->objtrianglelist[ i ].objtriangleindex = NULL;
		
		objmesh->objtrianglelist[ i ].n_objtriangleindex = 0;
		
		free( objmesh->objtrianglelist[ i ].indice_array );
		objmesh->objtrianglelist[ i ].indice_array = NULL;
		
		++i;
	}
}


unsigned char OBJ_load_mtl( OBJ *obj, char *filename, unsigned char relative_path )
{
	MEMORY *m = mopen( filename, relative_path );

	OBJMATERIAL *objmaterial = NULL;

	if( !m ) return 0; 

	get_file_path( m->filename, obj->texture_path );

	get_file_path( m->filename, obj->program_path );

	char *line = strtok( ( char * )m->buffer, "\n" ),
		 str[ MAX_PATH ] = {""};
		 
	vec3 v;

	while( line )
	{
		if( !line[ 0 ] || line[ 0 ] == '#' ) goto next_mat_line;
		
		else if( sscanf( line, "newmtl %s", str ) == 1 )
		{
			++obj->n_objmaterial;
			
			obj->objmaterial = ( OBJMATERIAL * ) realloc( obj->objmaterial, 
														  obj->n_objmaterial *
														  sizeof( OBJMATERIAL ) );
			
			objmaterial = &obj->objmaterial[ obj->n_objmaterial - 1 ];
			
			memset( objmaterial, 0, sizeof( OBJMATERIAL ) );
					
			strcpy( objmaterial->name, str );
		}

		else if( sscanf( line, "Ka %f %f %f", &v.x, &v.y, &v.z ) == 3 )
		{ memcpy( &objmaterial->ambient, &v, sizeof( vec3 ) ); }

		else if( sscanf( line, "Kd %f %f %f", &v.x, &v.y, &v.z ) == 3 )
		{ memcpy( &objmaterial->diffuse, &v, sizeof( vec3 ) ); }

		else if( sscanf( line, "Ks %f %f %f", &v.x, &v.y, &v.z ) == 3 )
		{ memcpy( &objmaterial->specular, &v, sizeof( vec3 ) ); }

		else if( sscanf( line, "Tf %f %f %f", &v.x, &v.y, &v.z ) == 3 )
		{ memcpy( &objmaterial->transmission_filter, &v, sizeof( vec3 ) ); }

		else if( sscanf( line, "illum %f", &v.x ) == 1 )
		{ objmaterial->illumination_model = ( int )v.x; }

		else if( sscanf( line, "d %f", &v.x ) == 1 )
		{
			objmaterial->ambient.w  = v.x;
			objmaterial->diffuse.w  = v.x;
			objmaterial->specular.w = v.x;
			objmaterial->dissolve   = v.x;
		}

		else if( sscanf( line, "Ns %f", &v.x ) == 1 )
		{ objmaterial->specular_exponent = v.x; }

		else if( sscanf( line, "Ni %f", &v.x ) == 1 )
		{ objmaterial->optical_density = v.x; }

		else if( sscanf( line, "map_Ka %s", str ) == 1 )
		{
			get_file_name( str, objmaterial->map_ambient );
			
			get_file_extension( objmaterial->map_ambient, str, 1 );
			
			if( !strcmp( str, "GFX" ) ) OBJ_add_program( obj, objmaterial->map_ambient );
			
			else OBJ_add_texture( obj, objmaterial->map_ambient );
		}

		else if( sscanf( line, "map_Kd %s", str ) == 1 )
		{
			get_file_name( str, objmaterial->map_diffuse );
			
			get_file_extension( objmaterial->map_diffuse, str, 1 );
			
			if( !strcmp( str, "GFX" ) ) OBJ_add_program( obj, objmaterial->map_diffuse );
			
			else OBJ_add_texture( obj, objmaterial->map_diffuse );

		}

		else if( sscanf( line, "map_Ks %s", str ) == 1 )
		{
			get_file_name( str, objmaterial->map_specular );
			
			get_file_extension( objmaterial->map_specular, str, 1 );
			
			if( !strcmp( str, "GFX" ) ) OBJ_add_program( obj, objmaterial->map_specular );
			
			else OBJ_add_texture( obj, objmaterial->map_specular );
		}

		else if( sscanf( line, "map_Tr %s", str ) == 1 )
		{
			get_file_name( str, objmaterial->map_translucency );
			
			get_file_extension( objmaterial->map_translucency, str, 1 );
			
			if( !strcmp( str, "GFX" ) ) OBJ_add_program( obj, objmaterial->map_translucency );
			
			else OBJ_add_texture( obj, objmaterial->map_translucency );
		}

		else if( sscanf( line, "map_disp %s", str ) == 1 ||
				 sscanf( line, "map_Disp %s", str ) == 1 ||
				 sscanf( line, "disp %s"    , str ) == 1 )
		{
			get_file_name( str, objmaterial->map_disp );
			
			get_file_extension( objmaterial->map_disp, str, 1 );
			
			if( !strcmp( str, "GFX" ) ) OBJ_add_program( obj, objmaterial->map_disp );
			
			else OBJ_add_texture( obj, objmaterial->map_disp );

		}

		else if( sscanf( line, "map_bump %s", str ) == 1 ||
				 sscanf( line, "map_Bump %s", str ) == 1 ||
				 sscanf( line, "bump %s"	, str ) == 1 )
		{
			get_file_name( str, objmaterial->map_bump );
			
			get_file_extension( objmaterial->map_bump, str, 1 );
			
			if( !strcmp( str, "GFX" ) ) OBJ_add_program( obj, objmaterial->map_bump );
			
			else OBJ_add_texture( obj, objmaterial->map_bump );
		}

		next_mat_line:
		
			line = strtok( NULL, "\n" );
	}

	mclose( m );

	return 1;
}



OBJ *OBJ_load( char *filename, unsigned char relative_path )
{
	OBJ *obj = NULL;
	
	MEMORY *o = mopen( filename, relative_path );
	
	if( !o ) return obj;

	else
	{
		char name  [ MAX_CHAR ] = {""},	
			 group [ MAX_CHAR ] = {""},
			 usemtl[ MAX_CHAR ] = {""},
			 str   [ MAX_PATH ] = {""},
			 last  = 0,
			 *line = strtok( ( char * )o->buffer, "\n" );
		
		unsigned char use_smooth_normals;
		
		OBJMESH *objmesh = NULL;
		
		OBJTRIANGLELIST *objtrianglelist = NULL;
		
		vec3 v;

		obj = ( OBJ * ) calloc( 1, sizeof( OBJ ) );

		while( line )
		{	
			if( !line[ 0 ] || line[ 0 ] == '#' ) goto next_obj_line;
			
			else if( line[ 0 ] == 'f' && line[ 1 ] == ' ' )
			{
				unsigned char useuvs;
						
				int vertex_index[ 3 ] = { 0, 0, 0 },
					normal_index[ 3 ] = { 0, 0, 0 },
					uv_index	[ 3 ] = { 0, 0, 0 },
					triangle_index;
					
				if( sscanf( line, "f %d %d %d", &vertex_index[ 0 ],
												&vertex_index[ 1 ],
												&vertex_index[ 2 ] ) == 3 )
												{ useuvs = 0; }
				
				else if( sscanf( line, "f %d//%d %d//%d %d//%d", &vertex_index[ 0 ],
																 &normal_index[ 0 ],
																 &vertex_index[ 1 ],
																 &normal_index[ 1 ],
																 &vertex_index[ 2 ],
																 &normal_index[ 2 ] ) == 6 )
																 { useuvs = 0; }
				
				else if( sscanf( line, "f %d/%d %d/%d %d/%d", &vertex_index[ 0 ],
															  &uv_index    [ 0 ],
															  &vertex_index[ 1 ],
															  &uv_index    [ 1 ],
															  &vertex_index[ 2 ],
															  &uv_index    [ 2 ] ) == 6 )
															  { useuvs = 1; }
				
				else
				{
					sscanf( line, "f %d/%d/%d %d/%d/%d %d/%d/%d", &vertex_index[ 0 ],
																  &uv_index    [ 0 ],
																  &normal_index[ 0 ],
																  &vertex_index[ 1 ],
																  &uv_index    [ 1 ],
																  &normal_index[ 1 ],
																  &vertex_index[ 2 ],
																  &uv_index    [ 2 ],
																  &normal_index[ 2 ] );
					useuvs = 1;
				}
				
				
				if( last != 'f' )
				{
					++obj->n_objmesh;
								
					obj->objmesh = ( OBJMESH * ) realloc( obj->objmesh,
														  obj->n_objmesh *
														  sizeof( OBJMESH ) );

					objmesh = &obj->objmesh[ obj->n_objmesh - 1 ];
	
					memset( objmesh, 0, sizeof( OBJMESH ) );

					objmesh->scale.x  =
					objmesh->scale.y  =
					objmesh->scale.z  =
					objmesh->distance = 1.0f;
					objmesh->visible  = 1;

					if( name[ 0 ] ) strcpy( objmesh->name, name );
					
					else if( usemtl[ 0 ] ) strcpy( objmesh->name, usemtl );
					
					if( group[ 0 ] ) strcpy( objmesh->group, group );
					
					objmesh->use_smooth_normals = use_smooth_normals;
					
					++objmesh->n_objtrianglelist;

					objmesh->objtrianglelist = ( OBJTRIANGLELIST * ) realloc( objmesh->objtrianglelist, 
																			  objmesh->n_objtrianglelist *
																			  sizeof( OBJTRIANGLELIST ) );

					objtrianglelist = &objmesh->objtrianglelist[ objmesh->n_objtrianglelist - 1 ];
					
					memset( objtrianglelist,
							0,
							sizeof( OBJTRIANGLELIST ) );
					
					objtrianglelist->mode = GL_TRIANGLES;
					
					if( useuvs ) objtrianglelist->useuvs = useuvs;
					
					
					if( usemtl[ 0 ] ) objtrianglelist->objmaterial = OBJ_get_material( obj, usemtl, 1 );
					
					name  [ 0 ] = 0;
					usemtl[ 0 ] = 0;
				}
				
				--vertex_index[ 0 ];
				--vertex_index[ 1 ];
				--vertex_index[ 2 ];

				--uv_index[ 0 ];
				--uv_index[ 1 ];
				--uv_index[ 2 ];
				
				
				OBJMESH_add_vertex_data( objmesh,
										 objtrianglelist,
										 vertex_index[ 0 ], 
										 uv_index    [ 0 ] );

				OBJMESH_add_vertex_data( objmesh,
										 objtrianglelist,
										 vertex_index[ 1 ], 
										 uv_index    [ 1 ] );

				OBJMESH_add_vertex_data( objmesh,
										 objtrianglelist,
										 vertex_index[ 2 ], 
										 uv_index    [ 2 ] );
										 
				
				triangle_index = objtrianglelist->n_objtriangleindex;
				
				++objtrianglelist->n_objtriangleindex;
				
				objtrianglelist->objtriangleindex = ( OBJTRIANGLEINDEX * ) realloc( objtrianglelist->objtriangleindex, 
																					objtrianglelist->n_objtriangleindex * sizeof( OBJTRIANGLEINDEX ) );
				
				objtrianglelist->objtriangleindex[ triangle_index ].vertex_index[ 0 ] = vertex_index[ 0 ];
				objtrianglelist->objtriangleindex[ triangle_index ].vertex_index[ 1 ] = vertex_index[ 1 ];
				objtrianglelist->objtriangleindex[ triangle_index ].vertex_index[ 2 ] = vertex_index[ 2 ];

				objtrianglelist->objtriangleindex[ triangle_index ].uv_index[ 0 ] = uv_index[ 0 ];
				objtrianglelist->objtriangleindex[ triangle_index ].uv_index[ 1 ] = uv_index[ 1 ];
				objtrianglelist->objtriangleindex[ triangle_index ].uv_index[ 2 ] = uv_index[ 2 ];
			}			
			
			else if( sscanf( line, "v %f %f %f", &v.x, &v.y, &v.z ) == 3 )
			{
				// Vertex
				++obj->n_indexed_vertex;
				
				obj->indexed_vertex = ( vec3 * ) realloc( obj->indexed_vertex,
														  obj->n_indexed_vertex * 
														  sizeof( vec3 ) );

				memcpy( &obj->indexed_vertex[ obj->n_indexed_vertex - 1 ],
						&v,
						sizeof( vec3 ) );

				
				// Normal
				obj->indexed_normal = ( vec3 * ) realloc( obj->indexed_normal,
														  obj->n_indexed_vertex * 
														  sizeof( vec3 ) );

				obj->indexed_fnormal = ( vec3 * ) realloc( obj->indexed_fnormal,
														   obj->n_indexed_vertex * 
														   sizeof( vec3 ) );
														  
				memset( &obj->indexed_normal[ obj->n_indexed_vertex - 1 ],
						0,
						sizeof( vec3 ) );
			
				memset( &obj->indexed_fnormal[ obj->n_indexed_vertex - 1 ],
						0,
						sizeof( vec3 ) );			
			
				// Tangent
				obj->indexed_tangent = ( vec3 * ) realloc( obj->indexed_tangent,
														   obj->n_indexed_vertex * 
														   sizeof( vec3 ) );

				memset( &obj->indexed_tangent[ obj->n_indexed_vertex - 1 ],
						0,
						sizeof( vec3 ) );
			}

			// Drop the normals.
			else if( sscanf( line, "vn %f %f %f", &v.x, &v.y, &v.z ) == 3 ) goto next_obj_line;
			
			else if( sscanf( line, "vt %f %f", &v.x, &v.y ) == 2 )
			{
				++obj->n_indexed_uv;
				
				obj->indexed_uv = ( vec2 * ) realloc( obj->indexed_uv,
													  obj->n_indexed_uv * 
													  sizeof( vec2 ) );
				v.y = 1.0f - v.y;
				
				memcpy( &obj->indexed_uv[ obj->n_indexed_uv - 1 ],
						&v,
						sizeof( vec2 ) );
			}			

			else if( line[ 0 ] == 'v' && line[ 1 ] == 'n' ) goto next_obj_line;
			
			else if( sscanf( line, "usemtl %s", str ) == 1 ) strcpy( usemtl, str );
			
			else if( sscanf( line, "o %s", str ) == 1 ) strcpy( name, str );

			else if( sscanf( line, "g %s", str ) == 1 ) strcpy( group, str );
			
			else if( sscanf( line, "s %s", str ) == 1 )
			{
				use_smooth_normals = 1;
				
				if( !strcmp( str, "off" ) || !strcmp( str, "0" ) )
				{ use_smooth_normals = 0; }
			}
			
			else if( sscanf( line, "mtllib %s", str ) == 1 )
			{
				o->position = ( unsigned char * )line - o->buffer + strlen( line ) + 1;
				
				OBJ_load_mtl( obj, str, relative_path );
				
				line = strtok( ( char * )&o->buffer[ o->position ], "\n" );
				continue;
			}

			next_obj_line:
			
				last = line[ 0 ];
				line = strtok( NULL, "\n" );
		}
		
		mclose( o );
	}

	
	// Build Normals and Tangent
	{
		unsigned int i, j, k,index;
		
		i = 0;
		while( i != obj->n_objmesh )
		{
			OBJMESH *objmesh = &obj->objmesh[ i ];
		
			// Accumulate Normals and Tangent
			j = 0;
			while( j != objmesh->n_objtrianglelist )
			{
				OBJTRIANGLELIST *objtrianglelist = &objmesh->objtrianglelist[ j ];

				k = 0;
				while( k != objtrianglelist->n_objtriangleindex )
				{
					vec3 v1,
						 v2,
						 normal;


					vec3_diff( &v1,
							   &obj->indexed_vertex[ objtrianglelist->objtriangleindex[ k ].vertex_index[ 0 ] ],
							   &obj->indexed_vertex[ objtrianglelist->objtriangleindex[ k ].vertex_index[ 1 ] ] );

					vec3_diff( &v2,
							   &obj->indexed_vertex[ objtrianglelist->objtriangleindex[ k ].vertex_index[ 0 ] ],
							   &obj->indexed_vertex[ objtrianglelist->objtriangleindex[ k ].vertex_index[ 2 ] ] );


					vec3_cross( &normal, &v1, &v2 );
						
					vec3_normalize( &normal, &normal );
					
					
					// Face normals
					memcpy( &obj->indexed_fnormal[ objtrianglelist->objtriangleindex[ k ].vertex_index[ 0 ] ],
							&normal,
							sizeof( vec3 ) );

					memcpy( &obj->indexed_fnormal[ objtrianglelist->objtriangleindex[ k ].vertex_index[ 1 ] ],
							&normal,
							sizeof( vec3 ) );

					memcpy( &obj->indexed_fnormal[ objtrianglelist->objtriangleindex[ k ].vertex_index[ 2 ] ],
							&normal,
							sizeof( vec3 ) );
					
					
					// Smooth normals
					vec3_add( &obj->indexed_normal[ objtrianglelist->objtriangleindex[ k ].vertex_index[ 0 ] ], 
							  &obj->indexed_normal[ objtrianglelist->objtriangleindex[ k ].vertex_index[ 0 ] ],
							  &normal );

					vec3_add( &obj->indexed_normal[ objtrianglelist->objtriangleindex[ k ].vertex_index[ 1 ] ], 
							  &obj->indexed_normal[ objtrianglelist->objtriangleindex[ k ].vertex_index[ 1 ] ],
							  &normal );

					vec3_add( &obj->indexed_normal[ objtrianglelist->objtriangleindex[ k ].vertex_index[ 2 ] ], 
							  &obj->indexed_normal[ objtrianglelist->objtriangleindex[ k ].vertex_index[ 2 ] ],
							  &normal );

				
					if( objtrianglelist->useuvs )
					{
						vec3 tangent;
						
						vec2 uv1, uv2;
					
						float c;
						
						vec2_diff( &uv1,
								   &obj->indexed_uv[ objtrianglelist->objtriangleindex[ k ].uv_index[ 2 ] ],
								   &obj->indexed_uv[ objtrianglelist->objtriangleindex[ k ].uv_index[ 0 ] ] );

						vec2_diff( &uv2,
								   &obj->indexed_uv[ objtrianglelist->objtriangleindex[ k ].uv_index[ 1 ] ],
								   &obj->indexed_uv[ objtrianglelist->objtriangleindex[ k ].uv_index[ 0 ] ] );
						

						c = 1.0f / ( uv1.x * uv2.y - uv2.x * uv1.y );
						
						tangent.x = ( v1.x * uv2.y + v2.x * uv1.y ) * c;
						tangent.y = ( v1.y * uv2.y + v2.y * uv1.y ) * c;
						tangent.z = ( v1.z * uv2.y + v2.z * uv1.y ) * c;


						vec3_add( &obj->indexed_tangent[ objtrianglelist->objtriangleindex[ k ].vertex_index[ 0 ] ], 
								  &obj->indexed_tangent[ objtrianglelist->objtriangleindex[ k ].vertex_index[ 0 ] ],
								  &tangent );

						vec3_add( &obj->indexed_tangent[ objtrianglelist->objtriangleindex[ k ].vertex_index[ 1 ] ], 
								  &obj->indexed_tangent[ objtrianglelist->objtriangleindex[ k ].vertex_index[ 1 ] ],
								  &tangent );

						vec3_add( &obj->indexed_tangent[ objtrianglelist->objtriangleindex[ k ].vertex_index[ 2 ] ], 
								  &obj->indexed_tangent[ objtrianglelist->objtriangleindex[ k ].vertex_index[ 2 ] ],
								  &tangent );			
					}
				
					++k;
				}
			
				++j;
			}
		
			++i;
		}
		

		// Normalize Normals & Tangent
		i = 0;
		while( i != obj->n_objmesh )
		{
			j = 0;
			while( j != obj->objmesh[ i ].n_objvertexdata )
			{
				index = obj->objmesh[ i ].objvertexdata[ j ].vertex_index;
				
				// Average smooth normals.
				vec3_normalize( &obj->indexed_normal[ index ],
								&obj->indexed_normal[ index ] );
				
				if( obj->objmesh[ i ].objvertexdata[ j ].uv_index != -1 )
				{
					vec3_normalize( &obj->indexed_tangent[ index ],
									&obj->indexed_tangent[ index ] );
				}
			
				++j;
			}
			
			++i;
		}	
	}

	return obj;
}


void OBJ_free_vertex_data( OBJ *obj )
{
	if( obj->indexed_vertex )
	{
		free( obj->indexed_vertex );
		obj->indexed_vertex = NULL;
	}

	if( obj->indexed_normal )
	{
		free( obj->indexed_normal );
		obj->indexed_normal = NULL;
	}

	if( obj->indexed_fnormal )
	{
		free( obj->indexed_fnormal );
		obj->indexed_fnormal = NULL;
	}

	if( obj->indexed_tangent )
	{
		free( obj->indexed_tangent );
		obj->indexed_tangent = NULL;
	}

	if( obj->indexed_uv )
	{
		free( obj->indexed_uv );
		obj->indexed_uv = NULL; 
	}
	
	obj->n_indexed_vertex = 
	obj->n_indexed_uv	  = 0;	
}


OBJ *OBJ_free( OBJ *obj )
{
	unsigned int i,
				 j;
	
	OBJ_free_vertex_data( obj );
	
	i = 0;
	while( i != obj->n_objmesh )
	{
		if( obj->objmesh[ i ].vao ) glDeleteVertexArraysOES( 1, &obj->objmesh[ i ].vao );
		
		if( obj->objmesh[ i ].vbo ) glDeleteBuffers( 1, &obj->objmesh[ i ].vbo );
	
		OBJ_free_mesh_vertex_data( obj, i );
		
		if( obj->objmesh[ i ].objtrianglelist )
		{
			j = 0;
			while( j != obj->objmesh[ i ].n_objtrianglelist )
			{
				glDeleteBuffers( 1, &obj->objmesh[ i ].objtrianglelist[ j ].vbo );
				++j;
			}
		
			free( obj->objmesh[ i ].objtrianglelist );
			obj->objmesh[ i ].objtrianglelist = NULL;
		}
		
		++i;
	}
	
	free( obj->objmesh );
	obj->objmesh = NULL;
	
	
	free( obj->objmaterial );
	obj->objmaterial = NULL;


	i = 0;
	while( i != obj->n_program )
	{
		if( obj->program[ i ]->vertex_shader   ) obj->program[ i ]->vertex_shader   = SHADER_free( obj->program[ i ]->vertex_shader   );
		if( obj->program[ i ]->fragment_shader ) obj->program[ i ]->fragment_shader = SHADER_free( obj->program[ i ]->fragment_shader );
		
		obj->program[ i ] = PROGRAM_free( obj->program[ i ] );
		++i;
	}
	
	if( obj->program )
	{
		free( obj->program );
		obj->program = NULL;
	}


	i = 0;
	while( i != obj->n_texture )
	{
		obj->texture[ i ] = TEXTURE_free( obj->texture[ i ] );
		++i;
	}
	
	if( obj->texture )
	{
		free( obj->texture );
		obj->texture = NULL;
	}
	
	obj->n_objmesh	   =
	obj->n_objmaterial = 
	obj->n_texture	   = 0;
	
	free( obj );
	return NULL;
}
