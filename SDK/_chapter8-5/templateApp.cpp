/*

Book:      	Game and Graphics Programming for iOS and Android with OpenGL(R) ES 2.0
Author:    	Romain Marucchi-Foino
ISBN-10: 	1119975913
ISBN-13: 	978-1119975915
Publisher: 	John Wiley & Sons	

Copyright (C) 2011 Romain Marucchi-Foino

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of
this software. Permission is granted to anyone who either own or purchase a copy of
the book specified above, to use this software for any purpose, including commercial
applications subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that
you wrote the original software. If you use this software in a product, an acknowledgment
in the product would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be misrepresented
as being the original software.

3. This notice may not be removed or altered from any source distribution.

*/

#include "templateApp.h"

#define OBJ_FILE ( char * )"maze.obj"

#define PHYSIC_FILE ( char * )"maze.bullet"

#define VERTEX_SHADER ( char * )"vertex.glsl"

#define FRAGMENT_SHADER ( char * )"fragment.glsl"


OBJ *obj = NULL;

PROGRAM *program = NULL;

PROGRAM *path_point = NULL;

vec2 view_location,
 	 view_delta = { 0.0f, 0.0f };

vec3 eye,
	 next_eye,
	 center = { 0.0f, 0.0f, 0.0f },
     up = { 0.0f, 0.0f, 1.0f };


float rotx		= 45.0f,
      next_rotx = 0.0f, 
      rotz		= 0.0f,
      next_rotz	= -45.0f,
      distance	= 30.0f;	

OBJMESH *player = NULL;

OBJMESH *maze = NULL;

NAVIGATION *navigation = NULL;

unsigned char double_tap = 0;

NAVIGATIONPATH navigationpath_player;

NAVIGATIONPATHDATA navigationpathdata_player;

int viewport_matrix[ 4 ];

int player_next_point = -1;


FONT *font = NULL;

OBJMESH *enemy = NULL;

NAVIGATIONPATH navigationpath_enemy;

NAVIGATIONPATHDATA navigationpathdata_enemy;

int enemy_next_point = -1;

unsigned char game_over = 0;


TEMPLATEAPP templateApp = { templateAppInit,
							templateAppDraw,
							templateAppToucheBegan,
							templateAppToucheMoved };


btSoftBodyRigidBodyCollisionConfiguration *collisionconfiguration = NULL;

btCollisionDispatcher *dispatcher = NULL;

btBroadphaseInterface *broadphase = NULL;

btConstraintSolver *solver = NULL;

btSoftRigidDynamicsWorld *dynamicsworld = NULL;


void init_physic_world( void )
{
	collisionconfiguration = new btSoftBodyRigidBodyCollisionConfiguration();

	dispatcher = new btCollisionDispatcher( collisionconfiguration );

	broadphase = new btDbvtBroadphase();

	solver = new btSequentialImpulseConstraintSolver();

	dynamicsworld = new btSoftRigidDynamicsWorld( dispatcher,	
												  broadphase,
												  solver,
												  collisionconfiguration );

	dynamicsworld->setGravity( btVector3( 0.0f, 0.0f, -9.8f ) );
}


void load_physic_world( void )
{
	btBulletWorldImporter *btbulletworldimporter = new btBulletWorldImporter( dynamicsworld );

	MEMORY *memory = mopen( PHYSIC_FILE, 1 );

	btbulletworldimporter->loadFileFromMemory( ( char * )memory->buffer, memory->size );

	mclose( memory );

	unsigned int i = 0;

	while( i != btbulletworldimporter->getNumRigidBodies() ) { 

		OBJMESH *objmesh = OBJ_get_mesh( obj,
										 btbulletworldimporter->getNameForPointer(
										 btbulletworldimporter->getRigidBodyByIndex( i ) ), 0 ); 

		if( objmesh ) { 

			objmesh->btrigidbody = ( btRigidBody * )btbulletworldimporter->getRigidBodyByIndex( i );
			
			objmesh->btrigidbody->setUserPointer( objmesh );
		} 

		++i; 
	} 

	delete btbulletworldimporter;
}


void free_physic_world( void )
{
	while( dynamicsworld->getNumCollisionObjects() )
	{
		btCollisionObject *btcollisionobject = dynamicsworld->getCollisionObjectArray()[ 0 ];
		
		btRigidBody *btrigidbody = btRigidBody::upcast( btcollisionobject );

		if( btrigidbody )
		{
			delete btrigidbody->getCollisionShape();
			
			delete btrigidbody->getMotionState();
			
			dynamicsworld->removeRigidBody( btrigidbody );
			
			dynamicsworld->removeCollisionObject( btcollisionobject );
			
			delete btrigidbody;
		}
	}
	
	delete collisionconfiguration; collisionconfiguration = NULL;

	delete dispatcher; dispatcher = NULL;

	delete broadphase; broadphase = NULL;

	delete solver; solver = NULL;
	
	delete dynamicsworld; dynamicsworld = NULL;	
}


void program_bind_attrib_location( void *ptr ) {

	PROGRAM *program = ( PROGRAM * )ptr;

	glBindAttribLocation( program->pid, 0, "POSITION" );
	glBindAttribLocation( program->pid, 2, "TEXCOORD0" );
}


void near_callback( btBroadphasePair &btbroadphasepair,
                    btCollisionDispatcher  &btdispatcher,
                    const btDispatcherInfo &btdispatcherinfo ) {

	if( ( player->btrigidbody == btbroadphasepair.m_pProxy0->m_clientObject ||
		  player->btrigidbody == btbroadphasepair.m_pProxy1->m_clientObject )
		&&
		( enemy->btrigidbody == btbroadphasepair.m_pProxy0->m_clientObject ||
		  enemy->btrigidbody == btbroadphasepair.m_pProxy1->m_clientObject ) ) {

		game_over = 1;
	}
	
	btdispatcher.defaultNearCallback( btbroadphasepair,
									  btdispatcher,
									  btdispatcherinfo );
}


void load_game( void )
{
	obj = OBJ_load( OBJ_FILE, 1 );

	unsigned int i = 0;

	while( i != obj->n_objmesh ) {
	
		if( strstr( obj->objmesh[ i ].name, "maze" ) ) {
		
			navigation = NAVIGATION_init( ( char * )"maze" );

			navigation->navigationconfiguration.agent_height = 2.0f;

			navigation->navigationconfiguration.agent_radius = 0.4f;

			if( NAVIGATION_build( navigation, obj, i ) )
			{ console_print( "Navigation generated.\n"); }

			else
			{ console_print( "Unable to create the navigation mesh." ); }
		}
	
		OBJ_optimize_mesh( obj, i, 128 );

		OBJ_build_mesh2( obj, i );
		
		OBJ_free_mesh_vertex_data( obj, i );

		++i;
	}
	
	
	init_physic_world();
	
	load_physic_world();

	dispatcher->setNearCallback( near_callback );

	enemy = OBJ_get_mesh( obj, "enemy", 0 );

	enemy->btrigidbody->setAngularFactor( 0.0f );

	navigationpathdata_player.path_point_count =
	navigationpathdata_enemy.path_point_count  = 0;

	player_next_point =
	enemy_next_point  = -1;	


	player = OBJ_get_mesh( obj, "player", 0 );

	player->btrigidbody->setAngularFactor( 0.0f );

	maze = OBJ_get_mesh( obj, "maze", 0 );
	
	distance = maze->radius * 2.0f;


	i = 0;
	while( i != obj->n_texture ) { 

		OBJ_build_texture( obj,
						   i,
						   obj->texture_path,
						   TEXTURE_MIPMAP | TEXTURE_16_BITS,
						   TEXTURE_FILTER_2X,
						   0.0f );
		++i;
	}


	i = 0;
	while( i != obj->n_objmaterial ) { 

		OBJ_build_material( obj, i, NULL );
		
		++i;
	}	
	
	program = PROGRAM_create( ( char * )"default",
							  VERTEX_SHADER,
							  FRAGMENT_SHADER,
							  1,
							  0,
							  program_bind_attrib_location,
							  NULL );	
}


void templateAppInit( int width, int height ) {

	atexit( templateAppExit );

	GFX_start();

	glViewport( 0.0f, 0.0f, width, height );
	
	glGetIntegerv( GL_VIEWPORT, viewport_matrix );

	load_game();
}


void draw_navigation_points( NAVIGATIONPATHDATA *navigationpathdata, vec3 *color )
{
	unsigned int i = 0;

	while( i != navigationpathdata->path_point_count + 1 )
	{
		navigationpathdata->path_point_array[ i ].z = 1.0f;
		++i;
	}

					 
	glBindVertexArrayOES( 0 );

	glBindBuffer( GL_ARRAY_BUFFER, 0 );

	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );


	if( !path_point )
	{
		path_point = PROGRAM_create( ( char * )"path_point",
									 ( char * )"point_vert.glsl",
									 ( char * )"point_frag.glsl",
									 1,
									 0,
									 program_bind_attrib_location,
									 NULL );
	}
	

	PROGRAM_draw( path_point );

	glUniformMatrix4fv( PROGRAM_get_uniform_location(
						path_point,
						( char * )"MODELVIEWPROJECTIONMATRIX" ),
						1,
						GL_FALSE, 
						( float * )GFX_get_modelview_projection_matrix() );

	glUniform3fv( PROGRAM_get_uniform_location( 
				  path_point,
				  ( char * )"COLOR" ),
				  1,
				  ( float * )color );

	glEnableVertexAttribArray( 0 );

	glVertexAttribPointer( 0,
						   3,
						   GL_FLOAT,
						   GL_FALSE,
						   0,
						   navigationpathdata->path_point_array );

	glDrawArrays( GL_POINTS,
				  0,
				  navigationpathdata->path_point_count + 1 );

	glDrawArrays( GL_LINE_STRIP,
				  0,
				  navigationpathdata->path_point_count + 1 );
	
	glDisable( GL_BLEND );
}


void move_entity( OBJMESH *objmesh,
				  NAVIGATIONPATHDATA *navigationpathdata,
				  int *next_point,
				  float speed ) {

	objmesh->location.z = 
	navigationpathdata->path_point_array[ *next_point ].z = 0.0f;

	float distance = vec3_dist( &objmesh->location, 
								&navigationpathdata->path_point_array[ *next_point ] );

	if( distance < 0.1f ) { 
		
		++*next_point;

		if( *next_point == ( navigationpathdata->path_point_count + 1 ) ) { 
		
			*next_point = -1;
		}
	}

	if( *next_point != -1 ) {
		
		vec3 direction;
	
		vec3_diff( &direction,
				   &navigationpathdata->path_point_array[ *next_point ],
				   &objmesh->location );

		vec3_normalize( &direction,
						&direction );

		objmesh->btrigidbody->setLinearVelocity( btVector3( direction.x * speed,
															direction.y * speed,
															0.0f ) );
		objmesh->btrigidbody->setActivationState( ACTIVE_TAG );
	}
	else {
		
		objmesh->btrigidbody->setActivationState( WANTS_DEACTIVATION );
		navigationpathdata->path_point_count = 0;
	}
}


void templateAppDraw( void ) {

	glClearColor( 0.5f, 0.5f, 0.5f, 1.0f );
	glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );


	GFX_set_matrix_mode( PROJECTION_MATRIX );
	GFX_load_identity();
	
	
	GFX_set_perspective( 80.0f,
						 ( float )viewport_matrix[ 2 ] / ( float )viewport_matrix[ 3 ],
						 1.0f,
						 1000.0f,
						 -90.0f );

	if( game_over == 2 ) {

		templateAppExit();

		load_game();

		game_over = 0;
	}


	GFX_set_matrix_mode( MODELVIEW_MATRIX );
	GFX_load_identity();


	if( view_delta.x || view_delta.y ) { 

		if( view_delta.y ) next_rotz -= view_delta.y;

		if( view_delta.x ) { 
			next_rotx -= view_delta.x;
			next_rotx = CLAMP( next_rotx, 0.0f, 90.0f ); 
		}
		
		view_delta.x =
		view_delta.y = 0.0f;
	}

	rotx = rotx * 0.9f + next_rotx * 0.1f;
	rotz = rotz * 0.9f + next_rotz * 0.1f;



	eye.x = center.x + 
			distance * 
			cosf( rotx * DEG_TO_RAD ) * 
			sinf( rotz * DEG_TO_RAD );
	
	eye.y = center.y - 
			distance *
			cosf( rotx * DEG_TO_RAD ) *
			cosf( rotz * DEG_TO_RAD );
	
	
	eye.z = center.z +
			distance *
			sinf( rotx * DEG_TO_RAD );
			
			
	rotx = rotx * 0.9f + next_rotx * 0.1f;
	rotz = rotz * 0.9f + next_rotz * 0.1f;


	center.x = maze->location.x;
	center.y = maze->location.y;
	center.z = maze->location.z;


	GFX_look_at( &eye,
				 &center,
				 &up );


	if( double_tap ) { 
		
		vec3 location;
		
		if( GFX_unproject( view_location.x,
						   viewport_matrix[ 3 ] - view_location.y,
						   1.0f,
						   GFX_get_modelview_matrix(),
						   GFX_get_projection_matrix(),
						   viewport_matrix,
						   &location.x,
						   &location.y,
						   &location.z  ) ) {

			btVector3 ray_from( eye.x,
							    eye.y,
							    eye.z ),

					  ray_to( location.x + eye.x,
							  location.y + eye.y,
							  location.z + eye.z );

			btCollisionWorld::ClosestRayResultCallback collision_ray( ray_from,
																	  ray_to );
			
			dynamicsworld->rayTest( ray_from,
									ray_to,
									collision_ray );
			
			if( collision_ray.hasHit() &&
				collision_ray.m_collisionObject == maze->btrigidbody ) { 

				collision_ray.m_hitNormalWorld.normalize();

				if( collision_ray.m_hitNormalWorld.z() == 1.0f ) {
			
					navigationpath_player.start_location.x = player->location.x;
					navigationpath_player.start_location.y = player->location.y;
					navigationpath_player.start_location.z = player->location.z;

					navigationpath_player.end_location.x = collision_ray.m_hitPointWorld.x();
					navigationpath_player.end_location.y = collision_ray.m_hitPointWorld.y();
					navigationpath_player.end_location.z = collision_ray.m_hitPointWorld.z();
				
					if( NAVIGATION_get_path( navigation,
											&navigationpath_player,
											&navigationpathdata_player ) ) {
						
						player_next_point = 1;

						unsigned int i = 0;
						
						while( i != navigationpathdata_player.path_point_count + 1 ) { 
						
							console_print( "%d: %f %f %f\n",
										   i,
										   navigationpathdata_player.path_point_array[ i ].x,
										   navigationpathdata_player.path_point_array[ i ].y,
										   navigationpathdata_player.path_point_array[ i ].z );
							++i; 
						}
						
						printf( "\n" );
					}
				}
			}
		}

		double_tap = 0;
	}
	
	
	if( navigationpathdata_player.path_point_count ) {

		vec3 color = { 0.0f, 0.0f, 1.0f };

		draw_navigation_points( &navigationpathdata_player, &color );
   
		move_entity( player,
					 &navigationpathdata_player,
					 &player_next_point,
					 3.0f );
	}	
	
	
	static unsigned int start_time = get_milli_time();

	if( get_milli_time() - start_time > 1000 ) { 

		navigationpath_enemy.start_location.x = enemy->location.x;
		navigationpath_enemy.start_location.y = enemy->location.y;
		navigationpath_enemy.start_location.z = enemy->location.z;

		navigationpath_enemy.end_location.x = player->location.x;
		navigationpath_enemy.end_location.y = player->location.y;
		navigationpath_enemy.end_location.z = player->location.z;

		NAVIGATION_get_path( navigation,
							&navigationpath_enemy, 
							&navigationpathdata_enemy );

		enemy_next_point = 1;

		start_time = get_milli_time();
	}	
	
	
	if( navigationpathdata_enemy.path_point_count ) { 
	
		vec3 color = { 1.0f, 0.0f, 0.0f };

		move_entity( enemy,
					&navigationpathdata_enemy,
					&enemy_next_point,
					4.0f );

		draw_navigation_points( &navigationpathdata_enemy, &color );
	}
	
	
	PROGRAM_draw( program );

	glUniform1i( PROGRAM_get_uniform_location( program, ( char * )"DIFFUSE" ), 1 );

	unsigned int i = 0;

	while( i != obj->n_objmesh ) {

		OBJMESH *objmesh = &obj->objmesh[ i ];

		GFX_push_matrix();

		mat4 mat;

		objmesh->btrigidbody->getWorldTransform().getOpenGLMatrix( ( float * )&mat );

		memcpy( &objmesh->location, ( vec3 * )&mat.m[ 3 ], sizeof( vec3 ) );

		GFX_multiply_matrix( &mat );

		glUniformMatrix4fv( PROGRAM_get_uniform_location( program, ( char * )"MODELVIEWPROJECTIONMATRIX" ),
							1,
							GL_FALSE,
							( float * )GFX_get_modelview_projection_matrix() );

		OBJ_draw_mesh( obj, i );

		GFX_pop_matrix();

		++i;
	}
	
	
	if( !game_over ) dynamicsworld->stepSimulation( 1.0f / 60.0f );

	else { 

	  GFX_set_matrix_mode( PROJECTION_MATRIX );
	  GFX_load_identity();

	  float half_width  =
			( float )viewport_matrix[ 2 ] * 0.5f,
			half_height =
			( float )viewport_matrix[ 3 ] * 0.5f;

	  GFX_set_orthographic_2d( -half_width,
								half_width,
							   -half_height,
								half_height );

	  GFX_rotate( -90.0f, 0.0f, 0.0f, 1.0f );

	  GFX_translate( -half_height, -half_width, 0.0f );

	  GFX_set_matrix_mode( MODELVIEW_MATRIX );

	  GFX_load_identity();

	  vec4 color = { 0.0f, 0.0f, 0.0f, 1.0f };

	  char msg[ MAX_CHAR ] = {"GAME OVER!"};

	  if( !font ) {

		 font = FONT_init( ( char * )"foo.ttf" );

		 FONT_load( font,
					font->name,
					1,
					64.0f,
					512,
					512,
					32,
					96 );
	  }

	  float posx = ( viewport_matrix[ 3 ] * 0.5f ) -
				   ( FONT_length( font, msg ) * 0.5f ),

			posy = viewport_matrix[ 2 ] - font->font_size;

	  FONT_print( font,
				  posx + 4.0f,
				  posy - 4.0f,
				  msg,
				  &color );

	  color.y = 1.0f;

	  FONT_print( font,
			   posx,
			   posy,
			   msg,
			   &color );
	}
}


void templateAppToucheBegan( float x, float y, unsigned int tap_count )
{
	if( tap_count == 2 ) { 
		
		if( game_over == 1 ) game_over = 2;
		else double_tap = 1;
	}

	view_location.x = x;
	view_location.y = y;
}


void templateAppToucheMoved( float x, float y, unsigned int tap_count )
{
	view_delta.x = view_delta.x * 0.75f + ( x - view_location.x ) * 0.25f;
	view_delta.y = view_delta.y * 0.75f + ( y - view_location.y ) * 0.25f;

	view_location.x = x;
	view_location.y = y;
}


void templateAppExit( void ) {

	if( font ) {
	
		FONT_free( font );
		font = NULL;
	}
   
	if( path_point )
	{
		SHADER_free( path_point->vertex_shader );

		SHADER_free( path_point->fragment_shader );
		
		PROGRAM_free( path_point );

		path_point = NULL;
	}

	NAVIGATION_free( navigation );

	free_physic_world();

	SHADER_free( program->vertex_shader );

	SHADER_free( program->fragment_shader );

	PROGRAM_free( program );

	OBJ_free( obj );
}
