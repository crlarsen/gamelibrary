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


NAVIGATION *NAVIGATION_init( char *name )
{
	NAVIGATION *navigation = ( NAVIGATION * ) calloc( 1, sizeof( NAVIGATION ) );

	strcpy( navigation->name, name );

	navigation->tolerance.x = 1.0f;
	navigation->tolerance.y = 2.0f;
	navigation->tolerance.z = 1.0f;
	
	NAVIGATION_set_default_configuration( navigation );

	return navigation;
}


NAVIGATION *NAVIGATION_free( NAVIGATION *navigation )
{
	if( navigation->dtnavmesh ) dtFreeNavMesh( navigation->dtnavmesh );
	
	if( navigation->program )
	{
		SHADER_free( navigation->program->vertex_shader );

		SHADER_free( navigation->program->fragment_shader );
	
		PROGRAM_free( navigation->program );
	}
	
	free( navigation );
	
	return NULL;
}


void NAVIGATION_set_default_configuration( NAVIGATION *navigation )
{
	navigation->navigationconfiguration.cell_size				= 0.3f;
	navigation->navigationconfiguration.cell_height				= 0.2f;
	navigation->navigationconfiguration.agent_height			= 2.0f;
	navigation->navigationconfiguration.agent_radius			= 0.8f;
	navigation->navigationconfiguration.agent_max_climb			= 0.9f;
	navigation->navigationconfiguration.agent_max_slope			= 45.0f;
	navigation->navigationconfiguration.region_min_size			= 50.0f;
	navigation->navigationconfiguration.region_merge_size		= 20.0f;
	navigation->navigationconfiguration.edge_max_len			= 12.0f;
	navigation->navigationconfiguration.edge_max_error			= 1.3f;
	navigation->navigationconfiguration.vert_per_poly			= 6.0f;
	navigation->navigationconfiguration.detail_sample_dst		= 6.0f;
	navigation->navigationconfiguration.detail_sample_max_error	= 1.0f;	
}


unsigned char NAVIGATION_build( NAVIGATION *navigation, OBJ *obj, unsigned int mesh_index )
{
	unsigned int i = 0,
				 j = 0,
				 k = 0,
				 triangle_count = 0;
	
	int *indices = NULL;
	
	OBJMESH *objmesh = &obj->objmesh[ mesh_index ];
	
	vec3 *vertex_array = ( vec3 * ) malloc( objmesh->n_objvertexdata * sizeof( vec3 ) ),
		 *vertex_start = vertex_array;

	rcHeightfield *rcheightfield;
	
	rcCompactHeightfield *rccompactheightfield;
	
	rcContourSet *rccontourset;

	rcPolyMesh *rcpolymesh;
	
	rcPolyMeshDetail *rcpolymeshdetail;
	
	
	while( i != objmesh->n_objvertexdata )
	{ 
		memcpy( vertex_array,
				&obj->indexed_vertex[ objmesh->objvertexdata[ i ].vertex_index ],
				sizeof( vec3 ) );
				
		vec3_to_recast( vertex_array );
		
		++vertex_array;						
		++i;
	}
	
	
	i = 0;
	while( i != objmesh->n_objtrianglelist )
	{
		triangle_count += objmesh->objtrianglelist[ i ].n_indice_array;
	
		indices = ( int * ) realloc( indices, triangle_count * sizeof( int ) );
	
		j = 0;
		while( j != objmesh->objtrianglelist[ i ].n_indice_array )
		{
			indices[ k ] = objmesh->objtrianglelist[ i ].indice_array[ j ];
		
			++k;
			++j;
		}

		++i;
	}
	
	triangle_count /= 3;
	
	rcConfig rcconfig;

	memset( &rcconfig, 0, sizeof( rcConfig ) );
	
	rcconfig.cs						= navigation->navigationconfiguration.cell_size;
	rcconfig.ch						= navigation->navigationconfiguration.cell_height;
	rcconfig.walkableHeight			= ( int )ceilf ( navigation->navigationconfiguration.agent_height / rcconfig.ch );
	rcconfig.walkableRadius			= ( int )ceilf ( navigation->navigationconfiguration.agent_radius / rcconfig.cs );
	rcconfig.walkableClimb			= ( int )floorf( navigation->navigationconfiguration.agent_max_climb / rcconfig.ch );
	rcconfig.walkableSlopeAngle		= navigation->navigationconfiguration.agent_max_slope;
	rcconfig.minRegionSize			= ( int )rcSqr( navigation->navigationconfiguration.region_min_size );
	rcconfig.mergeRegionSize		= ( int )rcSqr( navigation->navigationconfiguration.region_merge_size );
	rcconfig.maxEdgeLen				= ( int )( navigation->navigationconfiguration.edge_max_len / rcconfig.cs );
	rcconfig.maxSimplificationError = navigation->navigationconfiguration.edge_max_error;
	rcconfig.maxVertsPerPoly		= ( int )navigation->navigationconfiguration.vert_per_poly;
	rcconfig.detailSampleDist		= rcconfig.cs * navigation->navigationconfiguration.detail_sample_dst;
	rcconfig.detailSampleMaxError   = rcconfig.ch * navigation->navigationconfiguration.detail_sample_max_error;
			
	
	rcCalcBounds( ( float * )vertex_start,
				  objmesh->n_objvertexdata,
				  rcconfig.bmin,
				  rcconfig.bmax );
	
	
	rcCalcGridSize(  rcconfig.bmin,
					 rcconfig.bmax,
					 rcconfig.cs,
					&rcconfig.width,
					&rcconfig.height );


	rcheightfield = rcAllocHeightfield();

	rcCreateHeightfield( *rcheightfield,
						  rcconfig.width,
						  rcconfig.height,
						  rcconfig.bmin,
						  rcconfig.bmax,
						  rcconfig.cs,
						  rcconfig.ch );


	navigation->triangle_flags = new unsigned char[ triangle_count ];
	
	memset( navigation->triangle_flags, 0, triangle_count * sizeof( unsigned char ) );
	
	rcMarkWalkableTriangles( rcconfig.walkableSlopeAngle,
							 ( float * )vertex_start,
							 objmesh->n_objvertexdata,
							 indices,
							 triangle_count,
							 navigation->triangle_flags );
	

	rcRasterizeTriangles( ( float * )vertex_start,
						  objmesh->n_objvertexdata,
						  indices,
						  navigation->triangle_flags,
						  triangle_count,
						 *rcheightfield,
						  rcconfig.walkableClimb );


	delete []navigation->triangle_flags;
	navigation->triangle_flags = NULL;

	free( vertex_start );
	free( indices      );
	

	rcFilterLowHangingWalkableObstacles(  rcconfig.walkableClimb,
										 *rcheightfield );
	
	
	rcFilterLedgeSpans(  rcconfig.walkableHeight,
						 rcconfig.walkableClimb,
						*rcheightfield );
	
	
	rcFilterWalkableLowHeightSpans(  rcconfig.walkableHeight,
									*rcheightfield );

	
	rccompactheightfield = rcAllocCompactHeightfield();

	rcBuildCompactHeightfield( rcconfig.walkableHeight,
							   rcconfig.walkableClimb,
							   RC_WALKABLE,
							   *rcheightfield,
							   *rccompactheightfield );

	rcFreeHeightField( rcheightfield );
	rcheightfield = NULL;

	rcErodeArea( RC_WALKABLE_AREA,
				 rcconfig.walkableRadius,
				 *rccompactheightfield );


	rcBuildDistanceField( *rccompactheightfield );


	rcBuildRegions( *rccompactheightfield,
					 rcconfig.borderSize,
					 rcconfig.minRegionSize,
					 rcconfig.mergeRegionSize );


	rccontourset = rcAllocContourSet();

	rcBuildContours( *rccompactheightfield,
					  rcconfig.maxSimplificationError,
					  rcconfig.maxEdgeLen,
					 *rccontourset );


	rcpolymesh = rcAllocPolyMesh();
	
	rcBuildPolyMesh( *rccontourset,
					  rcconfig.maxVertsPerPoly,
					 *rcpolymesh );


	rcpolymeshdetail = rcAllocPolyMeshDetail();

	rcBuildPolyMeshDetail( *rcpolymesh,
						   *rccompactheightfield,
							rcconfig.detailSampleDist,
							rcconfig.detailSampleMaxError,
						   *rcpolymeshdetail );


	rcFreeCompactHeightfield( rccompactheightfield );
	rccompactheightfield = NULL;
	
	rcFreeContourSet( rccontourset );
	rccontourset = NULL;


	if( rcconfig.maxVertsPerPoly <= DT_VERTS_PER_POLYGON )
	{
		dtNavMeshCreateParams dtnavmeshcreateparams;
		
		unsigned char *nav_data = NULL;
		
		int nav_data_size = 0;
	
		i = 0;
		while( i != rcpolymesh->npolys )
		{
			if( rcpolymesh->areas[ i ] == RC_WALKABLE_AREA )
			{
				rcpolymesh->areas[ i ] = 0;
				rcpolymesh->flags[ i ] = 0x01;
			}
							
			++i;
		}


		memset( &dtnavmeshcreateparams, 0, sizeof( dtNavMeshCreateParams ) );
		
		dtnavmeshcreateparams.verts			   = rcpolymesh->verts;
		dtnavmeshcreateparams.vertCount		   = rcpolymesh->nverts;
		dtnavmeshcreateparams.polys			   = rcpolymesh->polys;
		dtnavmeshcreateparams.polyAreas		   = rcpolymesh->areas;
		dtnavmeshcreateparams.polyFlags		   = rcpolymesh->flags;
		dtnavmeshcreateparams.polyCount		   = rcpolymesh->npolys;
		dtnavmeshcreateparams.nvp			   = rcpolymesh->nvp;
		
		dtnavmeshcreateparams.detailMeshes	   = rcpolymeshdetail->meshes;
		dtnavmeshcreateparams.detailVerts	   = rcpolymeshdetail->verts;
		dtnavmeshcreateparams.detailVertsCount = rcpolymeshdetail->nverts;
		dtnavmeshcreateparams.detailTris       = rcpolymeshdetail->tris;
		dtnavmeshcreateparams.detailTriCount   = rcpolymeshdetail->ntris;
		
		dtnavmeshcreateparams.walkableHeight   = navigation->navigationconfiguration.agent_height;
		dtnavmeshcreateparams.walkableRadius   = navigation->navigationconfiguration.agent_radius;
		dtnavmeshcreateparams.walkableClimb    = navigation->navigationconfiguration.agent_max_climb;
		
		rcVcopy( dtnavmeshcreateparams.bmin, rcpolymesh->bmin );
		rcVcopy( dtnavmeshcreateparams.bmax, rcpolymesh->bmax );
		
		dtnavmeshcreateparams.cs = rcconfig.cs;
		dtnavmeshcreateparams.ch = rcconfig.ch;
		
		
		dtCreateNavMeshData( &dtnavmeshcreateparams,
							 &nav_data,
							 &nav_data_size );
		
		if( !nav_data ) return 0;
		
		navigation->dtnavmesh = dtAllocNavMesh();
		
		navigation->dtnavmesh->init( nav_data,
									 nav_data_size,
									 DT_TILE_FREE_DATA,
									 NAVIGATION_MAX_NODE );
		
		rcFreePolyMesh( rcpolymesh );
		rcpolymesh = NULL;
		
		rcFreePolyMeshDetail( rcpolymeshdetail );
		rcpolymeshdetail = NULL;
		
		return 1;
	}
	
	return 0;
}


unsigned char NAVIGATION_get_path( NAVIGATION *navigation, NAVIGATIONPATH *navigationpath, NAVIGATIONPATHDATA *navigationpathdata )
{
	vec3 start_location = { navigationpath->start_location.x,
							navigationpath->start_location.y,
							navigationpath->start_location.z },
							
		 end_location = { navigationpath->end_location.x,
						  navigationpath->end_location.y,
						  navigationpath->end_location.z };
		
	vec3_to_recast( &start_location );
								
	vec3_to_recast( &end_location );

	navigationpath->start_reference = navigation->dtnavmesh->findNearestPoly( ( float * )&start_location,
																			  ( float * )&navigation->tolerance,
																			  &navigationpath->path_filter,
																			  0 );


	navigationpath->end_reference = navigation->dtnavmesh->findNearestPoly( ( float * )&end_location,
																			( float * )&navigation->tolerance,
																			&navigationpath->path_filter,
																			0 );
	

	navigationpath->poly_count = navigation->dtnavmesh->findPath( navigationpath->start_reference,
																  navigationpath->end_reference,
																  ( float * )&start_location,
																  ( float * )&end_location,
																  &navigationpath->path_filter,
																  navigationpath->poly_array,
																  NAVIGATION_MAX_POLY );

	if( navigationpath->poly_count )
	{
		vec3 closest_end;

		if( navigationpath->poly_array[ navigationpath->poly_count - 1 ] != navigationpath->end_reference )
		{
			navigation->dtnavmesh->closestPointOnPoly( navigationpath->poly_array[ navigationpath->poly_count - 1 ],
													   ( float * )&end_location,
													   ( float * )&closest_end );
		}
		else
		{ memcpy( &closest_end, &navigationpath->end_location, sizeof( vec3 ) ); }

	
		navigationpathdata->path_point_count = navigation->dtnavmesh->findStraightPath( ( float * )&start_location,
																						( float * )&closest_end,
																						navigationpath->poly_array,
																						navigationpath->poly_count,
																						( float * )navigationpathdata->path_point_array,
																						navigationpathdata->path_flags_array,
																						navigationpathdata->path_poly_array,
																						NAVIGATION_MAX_POLY );

		memcpy( &navigationpathdata->path_point_array[ navigationpathdata->path_point_count ],
				&end_location,
				sizeof( vec3 ) );
		

		if( navigationpathdata->path_point_count )
		{
			unsigned int i = 0;
					
			while( i != navigationpathdata->path_point_count + 1 )
			{
				recast_to_vec3( &navigationpathdata->path_point_array[ i ] );
				
				++i;
			}

			return 1;
		}
	}

	return 0;
}


void NAVIGATION_draw( NAVIGATION *navigation )
{
	if( !navigation->program )
	{
		navigation->program = PROGRAM_init( navigation->name );
		
		navigation->program->vertex_shader = SHADER_init( navigation->name, GL_VERTEX_SHADER );
		
		SHADER_compile( navigation->program->vertex_shader,
						"uniform highp mat4 MODELVIEWPROJECTIONMATRIX;"
						"attribute highp vec3 POSITION;"
						"void main( void ) {"
						"gl_Position = MODELVIEWPROJECTIONMATRIX * vec4( POSITION, 1.0 ); }",
						0 );

		navigation->program->fragment_shader = SHADER_init( navigation->name, GL_FRAGMENT_SHADER );
		
		SHADER_compile( navigation->program->fragment_shader,
						"void main( void ) {"
						"gl_FragColor = vec4( 0.25, 0.5, 1.0, 0.65 ); }",
						0 );

		PROGRAM_link( navigation->program, 0 );	
	}

	char vertex_attribute = PROGRAM_get_vertex_attrib_location( navigation->program,
															    ( char * )"POSITION" );

	glBindVertexArrayOES( 0 );

	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
	
	glEnable( GL_BLEND );
		
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	
	PROGRAM_draw( navigation->program );

	glUniformMatrix4fv( PROGRAM_get_uniform_location( navigation->program, ( char * )"MODELVIEWPROJECTIONMATRIX"),
						1,
						GL_FALSE, 
						( float * )GFX_get_modelview_projection_matrix() );

	glEnableVertexAttribArray( vertex_attribute );


	unsigned int j = 0;

	while( j != navigation->dtnavmesh->getMaxTiles() )
	{
		dtMeshTile *_dtMeshTile = navigation->dtnavmesh->getTile( j );
		
		if( !_dtMeshTile->header )
		{ continue; }
		
		unsigned int k = 0;
		
		while( k != _dtMeshTile->header->polyCount )
		{
			dtPoly *_dtPoly = &_dtMeshTile->polys[ k ];
			
			if( _dtPoly->type == DT_POLYTYPE_OFFMESH_CONNECTION )
			{ continue; }
			else
			{
				dtPolyDetail* pd = &_dtMeshTile->detailMeshes[ k ];

				unsigned int l = 0;
				
				while( l != pd->triCount )
				{
					vec3 v[ 3 ];
					
					const unsigned char *t = &_dtMeshTile->detailTris[ ( pd->triBase + l ) << 2 ];
					
					int m = 2;
					while( m != -1 )
					{
						if( t[ m ] < _dtPoly->vertCount )
						{
							memcpy( &v[ m ],
									&_dtMeshTile->verts[ _dtPoly->verts[ t[ m ] ] * 3 ],
									sizeof( vec3 ) );
									
							recast_to_vec3( &v[ m ] );							
						}
						else
						{
							memcpy( &v[ m ],
									&_dtMeshTile->detailVerts[ ( pd->vertBase + t[ m ] - _dtPoly->vertCount ) * 3 ],
									sizeof( vec3 ) );
											
							recast_to_vec3( &v[ m ] );
						}
					
						--m;
					}


					glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, ( float * )v );
					
					glDrawArrays( GL_TRIANGLES, 0, 3 );
				
					++l;
				}
			}
		
			++k;
		}

		++j;
	}
	
	glDisable( GL_BLEND );	
}
