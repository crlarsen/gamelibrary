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

#define OBJ_FILE ( char * )"Scene.obj"

#define PHYSIC_FILE ( char * )"Scene.bullet"


OBJ *obj = NULL;

vec3 eye,
	 next_eye,
     up = { 0.0f, 0.0f, 1.0f };

OBJMESH *player = NULL;

THREAD *thread = NULL;

vec4 frustum[ 6 ];

int viewport_matrix[ 4 ];

FONT *font_small = NULL,
	 *font_big   = NULL;

float rotx	      = 40.0f,
	  rotz	      = 0.0f,
	  distance    = 5.0f;

TEMPLATEAPP templateApp = { templateAppInit,
							templateAppDraw,
							templateAppToucheBegan,
							NULL,
							NULL,
							templateAppAccelerometer };


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

	glBindAttribLocation( program->pid, 0, "POSITION"  );
	glBindAttribLocation( program->pid, 2, "TEXCOORD0" );
}


void program_draw( void *ptr )
{
	PROGRAM *program = ( PROGRAM * )ptr;
	
	unsigned int i = 0;
		
	while( i != program->uniform_count )
	{
		if( !program->uniform_array[ i ].constant &&
			!strcmp( program->uniform_array[ i ].name, "DIFFUSE" ) )
		{
			glUniform1i( program->uniform_array[ i ].location, 1 );
		
			program->uniform_array[ i ].constant = 1;
		}

		else if( !strcmp( program->uniform_array[ i ].name, "MODELVIEWPROJECTIONMATRIX" ) )
		{
			glUniformMatrix4fv( program->uniform_array[ i ].location,
								1,
								GL_FALSE,
								( float * )GFX_get_modelview_projection_matrix() );		
		}

		++i;
	}
}


class ClosestNotMeRayResultCallback:public btCollisionWorld::ClosestRayResultCallback { 

	public:
		ClosestNotMeRayResultCallback( btRigidBody *rb,
									   const btVector3 &p1,
									   const btVector3 &p2 ) :
		btCollisionWorld::ClosestRayResultCallback( p1, p2 )
		{ m_btRigidBody = rb; }

	virtual btScalar addSingleResult( btCollisionWorld::LocalRayResult &localray,
									  bool normalinworldspace )
	{ 
		if( localray.m_collisionObject == m_btRigidBody )
		{ return 1.0f; }
		
		return ClosestRayResultCallback::addSingleResult( localray, normalinworldspace );
	}

	protected:
		btRigidBody *m_btRigidBody;
};


bool contact_added_callback( btManifoldPoint &btmanifoldpoint,
							 const btCollisionObject *btcollisionobject0,
							 int part_0, int index_0,
							 const btCollisionObject *btcollisionobject1,
							 int part_1, int index_1 ) {

	OBJMESH *objmesh0 = ( OBJMESH * )( ( btRigidBody * )btcollisionobject0 )->getUserPointer();

	OBJMESH *objmesh1 = ( OBJMESH * )( ( btRigidBody * )btcollisionobject1 )->getUserPointer();
	
	
		 
	return false;
}


void decompress_stream( void *ptr )
{
	
}


void load_level( void )
{
	obj = OBJ_load( OBJ_FILE, 1 );

	unsigned int i = 0;

	while( i != obj->n_objmesh ) {
	
		OBJ_optimize_mesh( obj, i, 128 );

		OBJ_build_mesh( obj, i );
		
		OBJ_free_mesh_vertex_data( obj, i );

		++i;
	}
	
	
	init_physic_world();
	
	load_physic_world();
	
	gContactAddedCallback = contact_added_callback;
	
	

	player = OBJ_get_mesh( obj, "player", 0 );
	
	player->btrigidbody->setFriction( 10.0f );
	
	memcpy( &eye, &player->location, sizeof( vec3 ) );
	
	
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
	while( i != obj->n_program ) { 

		OBJ_build_program( obj,
						   i,
						   program_bind_attrib_location,
						   program_draw,
						   1,
						   obj->program_path );
		++i;
	}


	i = 0;
	while( i != obj->n_objmaterial ) { 

		OBJ_build_material( obj, i, NULL );
		
		++i;
	}	

	

	font_small = FONT_init( ( char * )"foo.ttf" );

	FONT_load( font_small,
			   font_small->name,
			   1,
			   24.0f,
			   512,
			   512,
			   32,
			   96 );
			   

	font_big = FONT_init( ( char * )"foo.ttf" );

	FONT_load( font_big,
			   font_big->name,
			   1,
			   48.0f,
			   512,
			   512,
			   32,
			   96 );
}


void free_level( void )
{
	player = NULL;

	THREAD_pause( thread );	

	font_small = FONT_free( font_small );
	
	font_big = FONT_free( font_big );
	
	free_physic_world();

	obj = OBJ_free( obj );
}


void templateAppInit( int width, int height ) {

	atexit( templateAppExit );

	GFX_start();
	
	AUDIO_start();

	thread = THREAD_create( decompress_stream, NULL, THREAD_PRIORITY_NORMAL, 1 );

	glViewport( 0.0f, 0.0f, width, height );
	
	glGetIntegerv( GL_VIEWPORT, viewport_matrix );

	srandom( get_milli_time() );

	load_level();
}


void templateAppDraw( void ) {

	glClearColor( 1.0f, 1.0f, 1.0f, 1.0f );
	glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );

	GFX_set_matrix_mode( PROJECTION_MATRIX );
	GFX_load_identity();
	
	GFX_set_perspective( 80.0f,
						 ( float )viewport_matrix[ 2 ] / ( float )viewport_matrix[ 3 ],
						 0.1f,
						 50.0f,
						 -90.0f );

	GFX_set_matrix_mode( MODELVIEW_MATRIX );
	GFX_load_identity();
	  
	next_eye.x = player->location.x + 
				 distance * 
				 cosf( rotx * DEG_TO_RAD ) * 
				 sinf( rotz * DEG_TO_RAD );
	
	next_eye.y = player->location.y - 
				 distance *
				 cosf( rotx * DEG_TO_RAD ) *
				 cosf( rotz * DEG_TO_RAD );
	
	next_eye.z = player->location.z +
				 distance *
				 sinf( rotx * DEG_TO_RAD );	

	player->location.z += player->dimension.z;

	btVector3 p1( player->location.x,
				  player->location.y,
				  player->location.z ),

			  p2( next_eye.x,
			 	  next_eye.y,
			 	  next_eye.z );

	ClosestNotMeRayResultCallback back_ray( player->btrigidbody,
										    p1,
										    p2 );
	dynamicsworld->rayTest( p1,
						    p2,
						    back_ray );

	if( back_ray.hasHit() ) { 

	  back_ray.m_hitNormalWorld.normalize();

	  next_eye.x =   back_ray.m_hitPointWorld.x() +
				   ( back_ray.m_hitNormalWorld.x() * 0.1f );
				   
	  next_eye.y =  back_ray.m_hitPointWorld.y() +
				  ( back_ray.m_hitNormalWorld.y()* 0.1f );
				   
	  next_eye.z =  back_ray.m_hitPointWorld.z() +
				  ( back_ray.m_hitNormalWorld.z()* 0.1f );
	}

	eye.x = next_eye.x * 0.05f + eye.x * 0.95f;
	eye.y = next_eye.y * 0.05f + eye.y * 0.95f;
	eye.z = next_eye.z * 0.05f + eye.z * 0.95f;

	GFX_look_at( &eye,
				 &player->location,
				 &up );
	
	build_frustum( frustum,
				  GFX_get_modelview_matrix(),
				  GFX_get_projection_matrix() );	


	unsigned int i = 0;

	while( i != obj->n_objmesh ) {

		OBJMESH *objmesh = &obj->objmesh[ i ];

		objmesh->distance = sphere_distance_in_frustum( frustum, 
														&objmesh->location,
														objmesh->radius );

		if( objmesh->distance && objmesh->visible )
		{
			GFX_push_matrix();

			if( objmesh->btrigidbody )
			{
				mat4 mat;

				objmesh->btrigidbody->getWorldTransform().getOpenGLMatrix( ( float * )&mat );
				
				memcpy( &objmesh->location, ( vec3 * )&mat.m[ 3 ], sizeof( vec3 ) );

				GFX_multiply_matrix( &mat );				
			}
			else
			{
				GFX_translate( objmesh->location.x, 
							   objmesh->location.y, 
							   objmesh->location.z );
			}

			OBJ_draw_mesh( obj, i );

			GFX_pop_matrix();
		}
		
		++i;
	}
	
	dynamicsworld->stepSimulation( 1.0f / 60.0f );

	
	GFX_set_matrix_mode( PROJECTION_MATRIX );
	GFX_load_identity();

	float half_width  = ( float )viewport_matrix[ 2 ] * 0.5f,
		  half_height = ( float )viewport_matrix[ 3 ] * 0.5f;

	GFX_set_orthographic_2d( -half_width,
							  half_width,
						     -half_height,
							  half_height );

	GFX_rotate( -90.0f, 0.0f, 0.0f, 1.0f );

	GFX_translate( -half_height, -half_width, 0.0f );

	GFX_set_matrix_mode( MODELVIEW_MATRIX );

	GFX_load_identity();
}


void templateAppToucheBegan( float x, float y, unsigned int tap_count )
{

}


void templateAppAccelerometer( float x, float y, float z )
{

}


void templateAppExit( void ) {
	
	free_level();

	thread = THREAD_free( thread );
	
	AUDIO_stop();
}
