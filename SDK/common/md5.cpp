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


MD5 *MD5_load_mesh( char *filename, unsigned char relative_path )
{
	MEMORY *m = mopen( filename, relative_path );
	
	if( !m ) return NULL;
	
	MD5 *md5 = ( MD5 * ) calloc( 1, sizeof( MD5 ) );
	
	get_file_name( filename, md5->name );
	
	md5->distance =
	md5->scale.x  =
	md5->scale.y  =
	md5->scale.z  = 1.0f;
	md5->visible  = 1;
		
	char *line = strtok( ( char * )m->buffer, "\n" );
	
	int int_val = 0;
	
	unsigned int mesh_index = 0;
	
	while( line )
	{
		if( sscanf( line, "MD5Version %d", &int_val ) == 1 )
		{
			if( int_val != 10 ) goto cleanup;
		}
		
		else if( sscanf( line, "numJoints %d", &md5->n_joint ) == 1 )
		{
			md5->bind_pose = ( MD5JOINT * ) calloc( md5->n_joint, sizeof( MD5JOINT ) );
		}
		
		else if( sscanf( line, "numMeshes %d", &md5->n_mesh ) == 1 )
		{
			unsigned int i = 0;
			
			md5->md5mesh = ( MD5MESH * ) calloc( md5->n_mesh, sizeof( MD5MESH ) );
													
			while( i != md5->n_mesh )
			{										
				md5->md5mesh[ i ].mode = GL_TRIANGLES;
				md5->md5mesh[ i ].visible = 1;
				++i;
			}
		}

		else if( !strncmp( line, "joints {", 8 ) )
		{
			unsigned int i = 0;
			
			line = strtok( NULL, "\n" );
			
			while( line[ 0 ] != '}' )
			{
				if( sscanf( line,
							"%s %d ( %f %f %f ) ( %f %f %f )",
							 md5->bind_pose[ i ].name,
							&md5->bind_pose[ i ].parent,
							&md5->bind_pose[ i ].location.x,
							&md5->bind_pose[ i ].location.y,
							&md5->bind_pose[ i ].location.z,
							&md5->bind_pose[ i ].rotation.x,
							&md5->bind_pose[ i ].rotation.y,
							&md5->bind_pose[ i ].rotation.z ) == 8 )
				{
					vec4_build_w( &md5->bind_pose[ i ].rotation );
				
					++i;
				}
				
				line = strtok( NULL, "\n" );
			}
		}
		
		else if( !strncmp( line, "mesh {", 6 ) )
		{
			MD5VERTEX md5vertex;
			
			MD5TRIANGLE md5triangle;
			
			MD5WEIGHT md5weight;
			
			
			line = strtok( NULL, "\n" );
			
			while( line[ 0 ] != '}' )
			{
				if( sscanf( line, " shader \"%[^\"]", md5->md5mesh[ mesh_index ].shader ) == 1 ) goto next_mesh_line;

				else if( sscanf( line, " numverts %d", &md5->md5mesh[ mesh_index ].n_vertex ) == 1 )
				{
					md5->md5mesh[ mesh_index ].md5vertex = ( MD5VERTEX * ) calloc( md5->md5mesh[ mesh_index ].n_vertex,
																				   sizeof( MD5VERTEX ) );
				}
				
				else if( sscanf( line,
								 " vert %d ( %f %f ) %d %d",
								 &int_val,
								 &md5vertex.uv.x,
								 &md5vertex.uv.y,
								 &md5vertex.start,
								 &md5vertex.count ) == 5 )
				{
					memcpy( &md5->md5mesh[ mesh_index ].md5vertex[ int_val ],
							&md5vertex,
							sizeof( MD5VERTEX ) );
				}

				else if( sscanf( line, " numtris %d", &md5->md5mesh[ mesh_index ].n_triangle ) == 1 )
				{
					md5->md5mesh[ mesh_index ].n_indice = md5->md5mesh[ mesh_index ].n_triangle * 3;
					
					md5->md5mesh[ mesh_index ].md5triangle = ( MD5TRIANGLE * ) calloc( md5->md5mesh[ mesh_index ].n_triangle,
																					   sizeof( MD5TRIANGLE ) );
				}
				
				else if( sscanf( line,
								 " tri %d %hu %hu %hu",
								 &int_val,
								 &md5triangle.indice[ 2 ],
								 &md5triangle.indice[ 1 ],
								 &md5triangle.indice[ 0 ] ) == 4 )
				{
					memcpy( &md5->md5mesh[ mesh_index ].md5triangle[ int_val ],
							&md5triangle,
							sizeof( MD5TRIANGLE ) );
				}
				
				else if( sscanf( line, " numweights %d", &md5->md5mesh[ mesh_index ].n_weight ) == 1 )
				{
					md5->md5mesh[ mesh_index ].md5weight = ( MD5WEIGHT * ) calloc( md5->md5mesh[ mesh_index ].n_weight,
																				   sizeof( MD5WEIGHT ) );
				}
				
				else if( sscanf( line,
								 " weight %d %d %f ( %f %f %f )",
								 &int_val,
								 &md5weight.joint,
								 &md5weight.bias,
								 &md5weight.location.x,
								 &md5weight.location.y,
								 &md5weight.location.z ) == 6 )
								 
				{
					memcpy( &md5->md5mesh[ mesh_index ].md5weight[ int_val ],
							&md5weight,
							sizeof( MD5WEIGHT ) );
				}

next_mesh_line:
				line = strtok( NULL, "\n" );
			}
			
			unsigned int s = md5->md5mesh[ mesh_index ].n_indice * sizeof( unsigned short );
			
			md5->md5mesh[ mesh_index ].indice = ( unsigned short * ) malloc( s );
			
			memcpy( &md5->md5mesh[ mesh_index ].indice[ 0 ],
					&md5->md5mesh[ mesh_index ].md5triangle[ 0 ],
					s );

			++mesh_index;
		}


next_line:

		line = strtok( NULL, "\n" );
	}

	mclose( m );
	
	return md5;
	

cleanup:

	mclose( m );

	md5 = MD5_free( md5 );
	
	return md5;
}


int MD5_load_action( MD5 *md5, char *name, char *filename, unsigned char relative_path )
{
	MEMORY *m = mopen( filename, relative_path );
	
	if( !m ) return -1;	

	MD5ACTION *md5action;

	++md5->n_action;
	
	md5->md5action = ( MD5ACTION * ) realloc( md5->md5action,
											  md5->n_action * sizeof( MD5ACTION ) );

	md5action = &md5->md5action[ md5->n_action - 1 ];

	memset( md5action, 0, sizeof( MD5ACTION ) );

	strcpy( md5action->name, name );
	
	md5action->curr_frame = 0;
	md5action->next_frame = 1;

	
	char *line = strtok( ( char * )m->buffer, "\n" );
	
	int int_val = 0;
	
	while( line )
	{
		if( sscanf( line, "MD5Version %d", &int_val ) == 1 )
		{
			if( int_val != 10 ) goto cleanup;
		}
		
		else if( sscanf( line, "numFrames %d", &md5action->n_frame ) == 1 )
		{
			md5action->frame = ( MD5JOINT ** ) malloc( md5action->n_frame *
													   sizeof( MD5JOINT * ) );
			unsigned int i = 0;
			
			while( i != md5action->n_frame )
			{
				md5action->frame[ i ] = ( MD5JOINT * ) malloc( md5->n_joint *
															   sizeof( MD5JOINT ) );
				++i;
			}
		}

		else if( sscanf( line, "numJoints %d", &int_val ) == 1 )
		{
			if( md5->n_joint != int_val ) goto cleanup;
			
			md5action->pose = ( MD5JOINT * ) malloc( md5->n_joint *
													 sizeof( MD5JOINT ) );
			
			unsigned int i = 0;
			
			while( i != md5->n_joint )
			{
				strcpy( md5action->pose[ i ].name, md5->bind_pose[ i ].name );
				
				++i;
			}			
		}
		
		else if( sscanf( line, "frameRate %d", &int_val ) == 1 )
		{ md5action->fps = 1.0f / ( float )int_val; }

		else if( sscanf( line, "frame %d", &int_val ) )
		{
			MD5JOINT *md5joint = md5action->frame[ int_val ];
			
			line = strtok( NULL, "\n" );
			
			unsigned int i = 0;
			
			while( i != md5->n_joint )
			{
				if( sscanf( line,
							" %f %f %f %f %f %f",
							&md5joint[ i ].location.x,
							&md5joint[ i ].location.y,
							&md5joint[ i ].location.z,
							&md5joint[ i ].rotation.x,
							&md5joint[ i ].rotation.y,
							&md5joint[ i ].rotation.z ) == 6 )
				{
					strcpy( md5joint[ i ].name, md5->bind_pose[ i ].name );
					
					vec4_build_w( &md5joint[ i ].rotation );				
				}
				
				line = strtok( NULL, "\n" );
				
				++i;
			}
			
			
			vec3 location;
			
			vec4 rotation;
			
			i = 0;
			while( i != md5->n_joint )
			{
				if( md5->bind_pose[ i ].parent > -1 )
				{
					MD5JOINT *md5joint = &md5action->frame[ int_val ][ md5->bind_pose[ i ].parent ];
				
					vec3_rotate_vec4( &location,
									  &md5action->frame[ int_val ][ i ].location,
									  &md5joint->rotation );
					
					md5action->frame[ int_val ][ i ].location.x = location.x + md5joint->location.x;
					md5action->frame[ int_val ][ i ].location.y = location.y + md5joint->location.y;
					md5action->frame[ int_val ][ i ].location.z = location.z + md5joint->location.z;
					
					vec4_multiply_vec4( &rotation,
										&md5joint->rotation,
										&md5action->frame[ int_val ][ i ].rotation );
					
					vec4_normalize( &md5action->frame[ int_val ][ i ].rotation,
									&rotation );
				}
				
				++i;
			}
		}
			
next_line:

		line = strtok( NULL, "\n" );	
	}

	mclose( m );
	
	return ( md5->n_action - 1 );
	
	
cleanup:


	--md5->n_action;
	
	md5->md5action = ( MD5ACTION * ) realloc( md5->md5action,
											  md5->n_action * sizeof( MD5ACTION ) );
	mclose( m );
	
	return -1;
}


MD5 *MD5_free( MD5 *md5 )
{
	unsigned int i = 0,
			     j;

	MD5_free_mesh_data( md5 );

	while( i != md5->n_mesh )
	{
		MD5MESH *md5mesh = &md5->md5mesh[ i ];
		
		if( md5mesh->md5vertex ) free( md5mesh->md5vertex );
		
		if( md5mesh->vertex_data ) free( md5mesh->vertex_data );
		
		if( md5mesh->md5triangle ) free( md5mesh->md5triangle );
		
		if( md5mesh->md5weight ) free( md5mesh->md5weight );

		if( md5mesh->vbo ) glDeleteBuffers( 1, &md5mesh->vbo );

		if( md5mesh->vbo_indice ) glDeleteBuffers( 1, &md5mesh->vbo_indice );
		
		if( md5mesh->vao ) glDeleteVertexArraysOES( 1, &md5mesh->vao );
		
		++i;
	}

	if( md5->md5mesh ) free( md5->md5mesh );
	

	i = 0;
	while( i != md5->n_action )
	{
		MD5ACTION *md5action = &md5->md5action[ i ];
		
		j = 0;
		while( j != md5action->n_frame )
		{
			free( md5action->frame[ j ] );
			++j;
		}
		
		if( md5action->frame ) free( md5action->frame );
		
		if( md5action->pose ) free( md5action->pose );

		++i;
	}
	
	if( md5->md5action ) free( md5->md5action );

	
	if( md5->bind_pose )
	{
		free( md5->bind_pose );
		md5->bind_pose = NULL;
	}
	

	free( md5 );
	return NULL;
}


void MD5_free_mesh_data( MD5 *md5 )
{
	unsigned int i = 0;	

	while( i != md5->n_mesh )
	{
		MD5MESH *md5mesh = &md5->md5mesh[ i ];
		
		if( md5mesh->indice )
		{
			free( md5mesh->indice );
			md5mesh->indice = NULL;
		}
	
		if( md5mesh->md5triangle )
		{
			free( md5mesh->md5triangle );
			md5mesh->md5triangle = NULL;
		}
		
		++i;
	}	
}


MD5ACTION *MD5_get_action( MD5 *md5, char *name, unsigned char exact_name )
{
	unsigned int i = 0;
	
	while( i != md5->n_action )
	{
		if( exact_name )
		{ if( !strcmp( md5->md5action[ i ].name, name ) ) return &md5->md5action[ i ]; }
		
		else
		{ if( strstr( md5->md5action[ i ].name, name ) ) return &md5->md5action[ i ]; }
	
		++i;
	}
	
	return NULL;
}


MD5MESH *MD5_get_mesh( MD5 *md5, char *name, unsigned char exact_name )
{
	unsigned int i = 0;
	
	while( i != md5->n_mesh )
	{
		if( exact_name )
		{ if( !strcmp( md5->md5mesh[ i ].shader, name ) ) return &md5->md5mesh[ i ]; }
		
		else
		{ if( strstr( md5->md5mesh[ i ].shader, name ) ) return &md5->md5mesh[ i ]; }
	
		++i;
	}
	
	return NULL;
}



void MD5_action_play( MD5ACTION *md5action, unsigned char frame_interpolation_method, unsigned char loop )
{
	md5action->method = frame_interpolation_method;
	md5action->loop	  = loop;
	md5action->state  = PLAY;
	
	if( !md5action->frame_time && frame_interpolation_method == MD5_METHOD_FRAME )
	{
		md5action->frame_time =
		md5action->fps;
	}
}


void MD5_action_pause( MD5ACTION *md5action )
{ md5action->state = PAUSE; }


void MD5_action_stop( MD5ACTION *md5action )
{
	md5action->curr_frame = 0;
	md5action->next_frame = 1;
	md5action->state = STOP;
	md5action->frame_time = 0.0f;
}


void MD5_set_action_fps( MD5ACTION *md5action, float fps )
{ md5action->fps = 1.0f / fps; }


void MD5_set_mesh_attributes( MD5MESH *md5mesh )
{
	glBindBuffer( GL_ARRAY_BUFFER, md5mesh->vbo );
	
	glEnableVertexAttribArray( 0 );
	
	glVertexAttribPointer( 0,
						   3,
						   GL_FLOAT,
						   GL_FALSE,
						   0,
						   ( void * )NULL );	


	glEnableVertexAttribArray( 1 );
	
	glVertexAttribPointer( 1,
						   3,
						   GL_FLOAT,
						   GL_FALSE,
						   0,
						   BUFFER_OFFSET( md5mesh->offset[ 1 ] ) );


	glEnableVertexAttribArray( 2 );
	
	glVertexAttribPointer( 2,
						   2,
						   GL_FLOAT,
						   GL_FALSE,
						   0,
						   BUFFER_OFFSET( md5mesh->offset[ 2 ] ) );	


	glEnableVertexAttribArray( 3 );
	
	glVertexAttribPointer( 3,
						   3,
						   GL_FLOAT,
						   GL_FALSE,
						   0,
						   BUFFER_OFFSET( md5mesh->offset[ 3 ] ) );

	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, md5mesh->vbo_indice );
}


void MD5_set_mesh_visibility( MD5MESH *md5mesh, unsigned char visible )
{ md5mesh->visible = visible; }


void MD5_set_mesh_material( MD5MESH *md5mesh, OBJMATERIAL *objmaterial )
{ md5mesh->objmaterial = objmaterial; }


void MD5_optimize( MD5 *md5, unsigned int vertex_cache_size )
{
	unsigned int i = 0,
	             s;
	
	unsigned short n_group = 0;

	if( vertex_cache_size ) SetCacheSize( vertex_cache_size );

	while( i != md5->n_mesh )
	{
		MD5MESH *md5mesh = &md5->md5mesh[ i ];

		PrimitiveGroup *primitivegroup;
	
		if( GenerateStrips( ( unsigned short * )md5mesh->md5triangle,
							md5mesh->n_indice,
							&primitivegroup,
							&n_group,
							true ) )
		{
			if( primitivegroup[ 0 ].numIndices < md5mesh->n_indice )
			{
				md5mesh->mode = GL_TRIANGLE_STRIP;
				md5mesh->n_indice = primitivegroup[ 0 ].numIndices;
			
				s = primitivegroup[ 0 ].numIndices * sizeof( unsigned short );

				md5mesh->indice = ( unsigned short * ) realloc( md5mesh->indice, s );
				
				memcpy( &md5mesh->indice[ 0 ],
						&primitivegroup[ 0 ].indices[ 0 ],
						s );
			}

			delete[] primitivegroup;
		}
	
		++i;
	}
}


void MD5_build_vbo( MD5 *md5, unsigned int mesh_index )
{
	MD5MESH *md5mesh = &md5->md5mesh[ mesh_index ];

	md5mesh->size = md5mesh->n_vertex * ( sizeof( vec3 ) +  // Vertex
										  sizeof( vec3 ) +  // Normals
										  sizeof( vec2 ) +  // Texcoord0
										  sizeof( vec3 ) ); // Tangent0

	md5mesh->vertex_data = ( unsigned char * ) calloc( 1, md5mesh->size );

	md5mesh->offset[ 0 ] = 0;
			
	md5mesh->offset[ 1 ] = md5mesh->n_vertex * sizeof( vec3 );
	
	md5mesh->offset[ 2 ] =   md5mesh->offset[ 1 ] +
						   ( md5mesh->n_vertex * sizeof( vec3 ) );
		
	md5mesh->offset[ 3 ] =   md5mesh->offset[ 2 ] +
						   ( md5mesh->n_vertex * sizeof( vec2 ) );
						   
		
	glGenBuffers( 1, &md5mesh->vbo );
	
	glBindBuffer( GL_ARRAY_BUFFER, md5mesh->vbo );
	
	glBufferData( GL_ARRAY_BUFFER,
				  md5mesh->size,
				  md5mesh->vertex_data,
				  GL_DYNAMIC_DRAW );
	

	glGenBuffers( 1, &md5mesh->vbo_indice );

	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, md5mesh->vbo_indice );

	glBufferData( GL_ELEMENT_ARRAY_BUFFER,
				  md5mesh->n_indice * sizeof( unsigned short ),
				  md5mesh->indice,
				  GL_STATIC_DRAW );
}


void MD5_build_bind_pose_weighted_normals_tangents( MD5 *md5 )
{
	unsigned int i = 0,
				 j,
				 k;
				 
				 
	while( i != md5->n_mesh )
	{
		MD5MESH *md5mesh = &md5->md5mesh[ i ];
		
		vec3 *vertex_array = ( vec3 * )md5mesh->vertex_data;
		
		j = 0;
		while( j != md5mesh->n_vertex )
		{
			memset( &md5mesh->md5vertex[ j ].normal,
					0,
					sizeof( vec3 ) );

			memset( &md5mesh->md5vertex[ j ].tangent,
					0,
					sizeof( vec3 ) );
			++j;
		}
		
		j = 0;
		while( j != md5mesh->n_triangle )
		{
			MD5TRIANGLE *md5triangle = &md5mesh->md5triangle[ j ];

			vec3 v1,
				 v2,
				 normal;
			
			vec3_diff( &v1,
					   &vertex_array[ md5triangle->indice[ 0 ] ],
					   &vertex_array[ md5triangle->indice[ 1 ] ] );

			vec3_diff( &v2,
					   &vertex_array[ md5triangle->indice[ 0 ] ],
					   &vertex_array[ md5triangle->indice[ 2 ] ] );
			

			vec3_cross( &normal, &v1, &v2 );
			
			vec3_normalize( &normal, &normal );

			// Flat normals
			/*
			memcpy( &md5mesh->md5vertex[ md5triangle->indice[ 0 ] ].normal, 
					&normal,
					sizeof( vec3 ) );

			memcpy( &md5mesh->md5vertex[ md5triangle->indice[ 1 ] ].normal, 
					&normal,
					sizeof( vec3 ) );

			memcpy( &md5mesh->md5vertex[ md5triangle->indice[ 2 ] ].normal, 
					&normal,
					sizeof( vec3 ) );
			*/
			
			// Smooth normals
			vec3_add( &md5mesh->md5vertex[ md5triangle->indice[ 0 ] ].normal, 
					  &md5mesh->md5vertex[ md5triangle->indice[ 0 ] ].normal,
					  &normal );

			vec3_add( &md5mesh->md5vertex[ md5triangle->indice[ 1 ] ].normal, 
					  &md5mesh->md5vertex[ md5triangle->indice[ 1 ] ].normal,
					  &normal );

			vec3_add( &md5mesh->md5vertex[ md5triangle->indice[ 2 ] ].normal, 
					  &md5mesh->md5vertex[ md5triangle->indice[ 2 ] ].normal,
					  &normal );

			vec3 tangent;
									
			vec2 uv1, uv2;

			float c;

			vec2_diff( &uv1,
					   &md5mesh->md5vertex[ md5triangle->indice[ 1 ] ].uv,
					   &md5mesh->md5vertex[ md5triangle->indice[ 0 ] ].uv );

			vec2_diff( &uv2,
					   &md5mesh->md5vertex[ md5triangle->indice[ 2 ] ].uv,
					   &md5mesh->md5vertex[ md5triangle->indice[ 0 ] ].uv );


			c = 1.0f / ( uv1.x * uv2.y - uv2.x * uv1.y );

			tangent.x = ( v1.x * uv2.y + v2.x * uv1.y ) * c;
			tangent.y = ( v1.y * uv2.y + v2.y * uv1.y ) * c;
			tangent.z = ( v1.z * uv2.y + v2.z * uv1.y ) * c;


			vec3_add( &md5mesh->md5vertex[ md5triangle->indice[ 0 ] ].tangent, 
					  &md5mesh->md5vertex[ md5triangle->indice[ 0 ] ].tangent,
					  &tangent );

			vec3_add( &md5mesh->md5vertex[ md5triangle->indice[ 1 ] ].tangent, 
					  &md5mesh->md5vertex[ md5triangle->indice[ 1 ] ].tangent,
					  &tangent );

			vec3_add( &md5mesh->md5vertex[ md5triangle->indice[ 2 ] ].tangent, 
					  &md5mesh->md5vertex[ md5triangle->indice[ 2 ] ].tangent,
					  &tangent );
			
			++j;
		}
	

		j = 0;
		while( j != md5mesh->n_vertex )
		{
			// Average normals
			vec3_normalize( &md5mesh->md5vertex[ j ].normal, 
							&md5mesh->md5vertex[ j ].normal );

			vec3_normalize( &md5mesh->md5vertex[ j ].tangent, 
							&md5mesh->md5vertex[ j ].tangent );

			++j;
		}
		
		
		j = 0;
		while( j != md5mesh->n_weight )
		{
			memset( &md5mesh->md5weight[ j ].normal,
					0,
					sizeof( vec3 ) );

			memset( &md5mesh->md5weight[ j ].tangent,
					0,
					sizeof( vec3 ) );			
			++j;
		}
		
		
		j = 0;
		while( j != md5mesh->n_vertex )
		{
			MD5VERTEX *md5vertex = &md5mesh->md5vertex[ j ];
			
			k = 0;
			while( k != md5vertex->count )
			{
				MD5WEIGHT *md5weight = &md5mesh->md5weight[ md5vertex->start + k ];
				
				MD5JOINT *md5joint = &md5->bind_pose[ md5weight->joint ];	
				
				vec3 normal = { md5vertex->normal.x,
								md5vertex->normal.y,
								md5vertex->normal.z },
					
					 tangent = { md5vertex->tangent.x,
								 md5vertex->tangent.y,
								 md5vertex->tangent.z };
				
				vec4 rotation = { md5joint->rotation.x,
								  md5joint->rotation.y,
								  md5joint->rotation.z,
								  md5joint->rotation.w };

				
				vec4_conjugate( &rotation, &rotation );
				
				
				vec3_rotate_vec4( &normal,
								  &normal,
								  &rotation );

				vec3_rotate_vec4( &tangent,
								  &tangent,
								  &rotation );

			
				vec3_add( &md5weight->normal,
						  &md5weight->normal,
						  &normal );
						  
				vec3_add( &md5weight->tangent,
						  &md5weight->tangent,
						  &tangent );
				++k;
			}
		
			++j;
		}


		j = 0;
		while( j != md5mesh->n_weight )
		{
			vec3_normalize( &md5mesh->md5weight[ j ].normal,
							&md5mesh->md5weight[ j ].normal );

			vec3_normalize( &md5mesh->md5weight[ j ].tangent,
							&md5mesh->md5weight[ j ].tangent );
			++j;
		}


		++i;
	}
}


void MD5_set_pose( MD5 *md5, MD5JOINT *pose )
{
	unsigned int i = 0,
				 j,
				 k;
				 
	while( i != md5->n_mesh )
	{
		MD5MESH *md5mesh = &md5->md5mesh[ i ];

		vec3 *vertex_array  = ( vec3 * )md5mesh->vertex_data,
			 *normal_array  = ( vec3 * )&md5mesh->vertex_data[ md5mesh->offset[ 1 ] ],
			 *tangent_array = ( vec3 * )&md5mesh->vertex_data[ md5mesh->offset[ 3 ] ];
			 
		vec2 *uv_array = ( vec2 * )&md5mesh->vertex_data[ md5mesh->offset[ 2 ] ];
		
		memset( vertex_array , 0, md5mesh->offset[ 1 ] );
		memset( normal_array , 0, md5mesh->offset[ 1 ] );
		memset( tangent_array, 0, md5mesh->offset[ 1 ] );
		

		j = 0;
		while( j != md5mesh->n_vertex )
		{
			MD5VERTEX *md5vertex = &md5mesh->md5vertex[ j ];
		
			k = 0;
			while( k != md5vertex->count )
			{
				vec3 location = { 0.0f, 0.0f, 0.0f },
					 normal   = { 0.0f, 0.0f, 0.0f },
					 tangent  = { 0.0f, 0.0f, 0.0f };
					 

				MD5WEIGHT *md5weight = &md5mesh->md5weight[ md5vertex->start + k ];
				
				MD5JOINT *md5joint = &pose[ md5weight->joint ];

				vec3_rotate_vec4( &location,
								  &md5weight->location,
								  &md5joint->rotation );
								  
				vec3_rotate_vec4( &normal,
								  &md5weight->normal,
								  &md5joint->rotation );

				vec3_rotate_vec4( &tangent,
								  &md5weight->tangent,
								  &md5joint->rotation );
								  
				vertex_array[ j ].x += ( md5joint->location.x + location.x ) * md5weight->bias;
				vertex_array[ j ].y += ( md5joint->location.y + location.y ) * md5weight->bias;
				vertex_array[ j ].z += ( md5joint->location.z + location.z ) * md5weight->bias;

				normal_array[ j ].x += normal.x * md5weight->bias;
				normal_array[ j ].y += normal.y * md5weight->bias;
				normal_array[ j ].z += normal.z * md5weight->bias;

				tangent_array[ j ].x += tangent.x * md5weight->bias;
				tangent_array[ j ].y += tangent.y * md5weight->bias;
				tangent_array[ j ].z += tangent.z * md5weight->bias;
								
				++k;
			}

			uv_array[ j ].x = md5vertex->uv.x;
			uv_array[ j ].y = md5vertex->uv.y;
			
			++j;
		}
		
		
		glBindBuffer( GL_ARRAY_BUFFER, md5mesh->vbo );

		glBufferSubData( GL_ARRAY_BUFFER,
						 0,
						 md5mesh->size,
						 md5mesh->vertex_data );		

		++i;
	}

	glBindBuffer( GL_ARRAY_BUFFER, 0 );
}


void MD5_blend_pose( MD5 *md5, MD5JOINT *final_pose, MD5JOINT *pose0, MD5JOINT *pose1, unsigned char joint_interpolation_method, float blend )
{
	unsigned int i = 0;

	while( i != md5->n_joint )
	{
		vec3_lerp( &final_pose[ i ].location,
				   &pose0[ i ].location,
				   &pose1[ i ].location,
				   blend );
	
		switch( joint_interpolation_method )
		{
			case MD5_METHOD_FRAME:
			case MD5_METHOD_LERP:
			{
				vec4_lerp( &final_pose[ i ].rotation, 
						   &pose0[ i ].rotation,
						   &pose1[ i ].rotation,
						   blend );
				break;
			}
		
		
			case MD5_METHOD_SLERP:
			{
				vec4_slerp( &final_pose[ i ].rotation, 
							&pose0[ i ].rotation,
							&pose1[ i ].rotation,
							blend );

				break;
			}
		}


		++i;
	}
}


void MD5_add_pose( MD5 *md5, MD5JOINT *final_pose, MD5ACTION *action0, MD5ACTION *action1, unsigned char joint_interpolation_method, float action_weight )
{
	unsigned int i = 0;
	
	while( i != md5->n_joint )
	{
		if( memcmp( &action1->frame[ action1->curr_frame ][ i ].location, &action1->frame[ action1->next_frame ][ i ].location, sizeof( vec3 ) ) ||
			memcmp( &action1->frame[ action1->curr_frame ][ i ].rotation, &action1->frame[ action1->next_frame ][ i ].rotation, sizeof( vec4 ) ) )
		{
			vec3_lerp( &final_pose[ i ].location,
					   &action0->pose[ i ].location,
					   &action1->pose[ i ].location,
					   action_weight );
			
			switch( joint_interpolation_method )
			{
				case MD5_METHOD_FRAME:
				case MD5_METHOD_LERP:
				{
					vec4_lerp( &final_pose[ i ].rotation, 
							   &action0->pose[ i ].rotation,
							   &action1->pose[ i ].rotation,
							   action_weight );
					break;
				}
			
			
				case MD5_METHOD_SLERP:
				{
					vec4_slerp( &final_pose[ i ].rotation, 
								&action0->pose[ i ].rotation,
								&action1->pose[ i ].rotation,
								action_weight );
					break;
				}
			}
		}
		else
		{
			memcpy( &final_pose[ i ].location, &action0->pose[ i ].location, sizeof( vec3 ) );
			memcpy( &final_pose[ i ].rotation, &action0->pose[ i ].rotation, sizeof( vec4 ) );
		}

		++i;
	}
}


void MD5_update_bound_mesh( MD5 *md5 )
{
	unsigned int i = 0,
				 j;


	// Get the mesh min and max.
	md5->min.x =
	md5->min.y =
	md5->min.z = 99999.999f;

	md5->max.x =
	md5->max.y =
	md5->max.z = -99999.999f;


	while( i != md5->n_mesh )
	{
		MD5MESH *md5mesh = &md5->md5mesh[ i ];
		
		vec3 *vertex_array = ( vec3 * )md5mesh->vertex_data;
		
		j = 0;
		while( j != md5mesh->n_vertex )
		{
			if( vertex_array[ j ].x < md5->min.x ) md5->min.x = vertex_array[ j ].x;
			if( vertex_array[ j ].y < md5->min.y ) md5->min.y = vertex_array[ j ].y;
			if( vertex_array[ j ].z < md5->min.z ) md5->min.z = vertex_array[ j ].z;

			if( vertex_array[ j ].x > md5->max.x ) md5->max.x = vertex_array[ j ].x;
			if( vertex_array[ j ].y > md5->max.y ) md5->max.y = vertex_array[ j ].y;
			if( vertex_array[ j ].z > md5->max.z ) md5->max.z = vertex_array[ j ].z;
	
			++j;
		}
		
		++i;
	}


	// Mesh dimension
	vec3_diff( &md5->dimension,
			   &md5->max,
			   &md5->min );


	// Bounding sphere radius
	md5->radius = vec3_dist( &md5->min,
							 &md5->max ) * 0.5f;
}


void MD5_build( MD5 *md5 )
{
	unsigned int i = 0;
	
	while( i != md5->n_mesh )
	{
		MD5MESH *md5mesh = &md5->md5mesh[ i ];
	
		MD5_build_vbo( md5, i );
		
		glGenVertexArraysOES( 1, &md5mesh->vao );
		
		glBindVertexArrayOES( md5mesh->vao );	
		
		MD5_set_mesh_attributes( md5mesh );

		glBindVertexArrayOES( 0 );

		++i;
	}

	MD5_set_pose( md5, md5->bind_pose );
	
	MD5_build_bind_pose_weighted_normals_tangents( md5 );

	MD5_set_pose( md5, md5->bind_pose );
	
	MD5_update_bound_mesh( md5 );
}


void MD5_build2( MD5 *md5 )
{
	unsigned int i = 0;
	
	while( i != md5->n_mesh )
	{
		MD5_build_vbo( md5, i );

		++i;
	}

	MD5_set_pose( md5, md5->bind_pose );
	
	MD5_build_bind_pose_weighted_normals_tangents( md5 );	

	MD5_set_pose( md5, md5->bind_pose );
	
	MD5_update_bound_mesh( md5 );	
}


unsigned char MD5_draw_action( MD5 *md5, float time_step )
{
	unsigned int i = 0;
	
	unsigned char update = 0;
	
	while( i != md5->n_action )
	{
		MD5ACTION *md5action = &md5->md5action[ i ];
	
		if( md5action->state == PLAY )
		{
			md5action->frame_time += time_step;
			
			switch( md5action->method )
			{
				case MD5_METHOD_FRAME:
				{
					if( md5action->frame_time >= md5action->fps )
					{
						memcpy( md5action->pose,
								md5action->frame[ md5action->curr_frame ],
								md5->n_joint * sizeof( MD5JOINT ) );
						
						++md5action->curr_frame;

						if( md5action->curr_frame == md5action->n_frame )
						{
							if( md5action->loop ) md5action->curr_frame = 0;
							else
							{
								MD5_action_stop( md5action );
								break;
							}
						}
						
						md5action->next_frame = md5action->curr_frame + 1;
						
						if( md5action->next_frame == md5action->n_frame )
						{ md5action->next_frame = 0; }
						
						md5action->frame_time -= md5action->fps;
						
						update = 1;
					}
					
					break;
				}
			
				case MD5_METHOD_LERP:
				case MD5_METHOD_SLERP:					
				{
					float t = CLAMP( md5action->frame_time / md5action->fps, 0.0f, 1.0f );

					MD5_blend_pose( md5,
							 	    md5action->pose,
									md5action->frame[ md5action->curr_frame ],
									md5action->frame[ md5action->next_frame ],
									md5action->method,
									t );

					if( t >= 1.0f )
					{
						++md5action->curr_frame;

						md5action->next_frame = ( md5action->curr_frame + 1 );
						
						if( md5action->loop )
						{
							if( md5action->curr_frame == md5action->n_frame )
							{
								md5action->curr_frame = 0;
								md5action->next_frame = 1;
							}								

							if( md5action->next_frame == md5action->n_frame )
							{ md5action->next_frame = 0; }				
						}
						else
						{
							if( md5action->next_frame == md5action->n_frame )
							{
								MD5_action_stop( md5action );
								break;
							}
						}
						
						md5action->frame_time -= md5action->fps;
					}
					
					update = 1;

					break;
				}
			}
		}
	
		++i;
	}
	
	return update;
}


void MD5_draw( MD5 *md5 )
{
	if( md5->visible && md5->distance )
	{
		unsigned int i = 0;
	
		while( i != md5->n_mesh )
		{
			MD5MESH *md5mesh = &md5->md5mesh[ i ];
			
			if( md5mesh->visible )
			{
				if( md5mesh->objmaterial ) OBJ_draw_material( md5mesh->objmaterial );
			
				if( md5mesh->vao ) glBindVertexArrayOES( md5mesh->vao );
			
				else MD5_set_mesh_attributes( md5mesh );
				
				glDrawElements( md5mesh->mode,
								md5mesh->n_indice,
								GL_UNSIGNED_SHORT,
								( void * )NULL );
			}
							
			++i;
		}
	}	
}
