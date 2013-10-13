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

#define OBJ_FILE ( char * )"game.obj"

#define VERTEX_SHADER ( char * )"vertex.glsl"

#define FRAGMENT_SHADER ( char * )"fragment.glsl"

OBJ *obj = NULL;

PROGRAM *program = NULL;

TEMPLATEAPP templateApp = { templateAppInit,
							templateAppDraw,
							templateAppToucheBegan,
							NULL,
							templateAppToucheEnded };

vec2 start_pos = { 0.0f, 0.0f };

unsigned int momo_index = 0;

OBJMESH *momo = NULL;

OBJMESH *gameover = NULL;	

unsigned char restart_game = 0,
			  momo_launch  = 0,
			  banana	   = 0;

vec3 eye    = { 3.5f, -10.8f, 5.3f },
	 center = { 3.5f,  -9.8f, 5.3f },
	 up     = { 0.0f,   0.0f, 1.0f };


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


enum
{
	BOX		 = 0,
	SPHERE	 = 1,
	CYLINDER = 2
};

void add_rigid_body( OBJMESH *objmesh, unsigned char bound, float mass, unsigned char dynamic_only )
{
	btCollisionShape *btcollisionshape = NULL;
	
	switch ( bound )
	{
		case BOX:
		{
			btcollisionshape = new btBoxShape( btVector3( objmesh->dimension.x * 0.5f,
														  objmesh->dimension.y * 0.5f,
														  objmesh->dimension.z * 0.5f ) );
			break;
		}

		case SPHERE:
		{ 
			btcollisionshape = new btSphereShape( objmesh->radius );
			break;
		}
		 
		case CYLINDER:
		{ 
			btcollisionshape = new btCylinderShapeZ( btVector3( objmesh->dimension.x * 0.5f,
																objmesh->dimension.y * 0.5f,
																objmesh->dimension.z * 0.5f ) );
			break;
		}
	}

	btTransform bttransform;
	
	bttransform.setIdentity();

	bttransform.setOrigin( btVector3( objmesh->location.x,
									  objmesh->location.y,
									  objmesh->location.z ) );

	btDefaultMotionState *btdefaultmotionstate = NULL;
	
	btdefaultmotionstate = new btDefaultMotionState( bttransform );
	
	btVector3 localinertia( 0.0f, 0.0f, 0.0f );

	if( mass > 0.0f )
		btcollisionshape->calculateLocalInertia( mass, localinertia );

	objmesh->btrigidbody = new btRigidBody( mass,
											btdefaultmotionstate,
											btcollisionshape,
											localinertia ); 

	if( mass > 0.0f ) { 

		objmesh->btrigidbody->setLinearFactor( btVector3( 1.0f, 0.0f, 1.0f ) );

		if( !dynamic_only )	objmesh->btrigidbody->setAngularFactor( btVector3( 0.0f, 1.0f, 0.0f ) );
		
		else objmesh->btrigidbody->setAngularFactor( 0.0f );
	}

	
	objmesh->btrigidbody->setUserPointer( objmesh );

	dynamicsworld->addRigidBody( objmesh->btrigidbody );
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


bool contact_added_callback( btManifoldPoint &btmanifoldpoint,
							 const btCollisionObject *btcollisionobject0,
							 int part_0, int index_0,
							 const btCollisionObject *btcollisionobject1,
							 int part_1, int index_1 ) {

	OBJMESH *objmesh0 = ( OBJMESH * )( ( btRigidBody * )btcollisionobject0 )->getUserPointer();

	OBJMESH *objmesh1 = ( OBJMESH * )( ( btRigidBody * )btcollisionobject1 )->getUserPointer();


	/* Check if one of the two object involve in the collision is a momo. */
	if( ( strstr( objmesh0->name, "momo" ) || 
		  strstr( objmesh1->name, "momo" ) )
	&&
		( strstr( objmesh0->name, "banana" ) || 
		  strstr( objmesh1->name, "banana" ) ) ) { 

		OBJMESH *objmesh = NULL;
		btCollisionObject *btcollisionobject = NULL;

		if( strstr( objmesh0->name, "banana" ) ) { 
			objmesh = objmesh0;
			btcollisionobject = ( btCollisionObject * )btcollisionobject0;
		} 
		else { 
			objmesh = objmesh1;
			btcollisionobject = ( btCollisionObject * )btcollisionobject1;
		}
		
		objmesh->visible = 0;
		
		--banana;
		
		delete objmesh->btrigidbody->getCollisionShape();
		
		delete objmesh->btrigidbody->getMotionState();
		
		dynamicsworld->removeRigidBody( objmesh->btrigidbody );
		
		dynamicsworld->removeCollisionObject( btcollisionobject );
		
		delete objmesh->btrigidbody;
		
		objmesh->btrigidbody = NULL;
	}
		 
	return false;
}


void program_bind_attrib_location( void *ptr ) {

	PROGRAM *program = ( PROGRAM * )ptr;

	glBindAttribLocation( program->pid, 0, "POSITION" );
	glBindAttribLocation( program->pid, 2, "TEXCOORD0" );
}


void get_next_momo( void ) {

	char tmp[ MAX_CHAR ] = {""};

	unsigned int i = 0;

	momo = NULL;

	++momo_index;

	sprintf( tmp, "momo%d", momo_index );

	while( i != obj->n_objmesh ) { 

		if( strstr( obj->objmesh[ i ].name, tmp ) ) { 

			momo = &obj->objmesh[ i ];
			
			momo_launch = 0;

			momo->btrigidbody->setActivationState( DISABLE_DEACTIVATION );
			
			return;
		}
		
		++i;
	}
}


void load_game( void )
{
	init_physic_world();
	
	gContactAddedCallback = contact_added_callback;

	obj = OBJ_load( OBJ_FILE, 1 );

	unsigned int i = 0;

	while( i != obj->n_objmesh ) {
		
		OBJMESH *objmesh = &obj->objmesh[ i ];
		
		OBJ_optimize_mesh( obj, i, 128 );
		
		OBJ_build_mesh( obj, i );

		if( strstr( objmesh->name, "momo" ) )
			add_rigid_body( objmesh, SPHERE, 2.0f, 0 );

		else if( strstr( objmesh->name, "barrel" ) )
			add_rigid_body( objmesh, CYLINDER, 1.0f, 0 );

		else if( strstr( objmesh->name, "plank" ) )
			add_rigid_body( objmesh, BOX, 1.0f, 0 );

		else if( strstr( objmesh->name, "ground" ) )
			add_rigid_body( objmesh, BOX, 0.0f, 0 );

		else if( strstr( objmesh->name, "steel" ) )
			add_rigid_body( objmesh, CYLINDER, 0.0f, 0 );

		else if( strstr( objmesh->name, "banana" ) ) { 

			add_rigid_body( objmesh, SPHERE, 1.0f, 1 );

			objmesh->btrigidbody->setCollisionFlags( objmesh->btrigidbody->getCollisionFlags() |
													 btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK );

			objmesh->btrigidbody->forceActivationState ( ISLAND_SLEEPING );
			
			++banana;
		}

		else if( strstr( objmesh->name, "gameover" ) ) {
		
			objmesh->visible = 0;
			
			gameover = objmesh;
		}
	  
		OBJ_free_mesh_vertex_data( obj, i ); 

		++i;
	}
	

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

	program = PROGRAM_create( ( char * )"default",
							  VERTEX_SHADER,
							  FRAGMENT_SHADER,
							  1,
							  0,
							  program_bind_attrib_location,
							  NULL );

	i = 0;
	while( i != obj->n_objmaterial ) { 

		OBJ_build_material( obj, i, NULL );
		
		++i;
	}	

	PROGRAM_draw( program );
	
	glUniform1i( PROGRAM_get_uniform_location( program, ( char * )"DIFFUSE" ), 1 );
	
	momo_index  = 0;

	center.x =
    eye.x    = 3.5f;
	  	
	get_next_momo();
}


void templateAppInit( int width, int height ) {

	atexit( templateAppExit );

	GFX_start();
	
	glViewport( 0.0f, 0.0f, width, height );

	GFX_set_matrix_mode( PROJECTION_MATRIX );
	GFX_load_identity();
	
	GFX_set_orthographic( ( float )height / ( float )width,
						  15.0f,
					      ( float )width / ( float )height,
						  1.0f,
					      100.0f,
					      -90.0f );
	load_game();
}


void templateAppDraw( void ) {

	glClear( GL_DEPTH_BUFFER_BIT );

	if( restart_game )
	{
		templateAppExit();
		
		load_game();
		
		restart_game = 0;
	}

	GFX_set_matrix_mode( MODELVIEW_MATRIX );
	GFX_load_identity();
	
	if( momo )
	{
		eye.x = eye.x * 0.98f + momo->location.x * 0.02f;
		center.x = 
		eye.x = CLAMP( eye.x, -2.0f, 3.5f );
	}
	
	GFX_look_at( &eye, &center, &up );
	
	unsigned int i = 0;

	while( i != obj->n_objmesh ) {

		OBJMESH *objmesh = &obj->objmesh[ i ];

		GFX_push_matrix();

		if( objmesh->btrigidbody )
		{
			mat4 mat;
			
			objmesh->btrigidbody->getWorldTransform().getOpenGLMatrix( ( float * )&mat );
			
			objmesh->location.x = mat.m[ 3 ].x;
			
			GFX_multiply_matrix( &mat );
		}
		else GFX_translate( objmesh->location.x, objmesh->location.y, objmesh->location.z );

		
		glUniformMatrix4fv( PROGRAM_get_uniform_location( program, ( char * )"MODELVIEWPROJECTIONMATRIX" ),
							1,
							GL_FALSE,
							( float * )GFX_get_modelview_projection_matrix() );

		OBJ_draw_mesh( obj, i );

		GFX_pop_matrix();
		
		++i;
	}
	
	dynamicsworld->stepSimulation( 1.0f / 60.0f );
	
	
	if(   momo && 
		( momo->btrigidbody->getLinearVelocity().length() > 20.0f ||
		  momo->btrigidbody->getActivationState() == ISLAND_SLEEPING ) ) get_next_momo();
		  
	if( !momo || !banana ) {

		gameover->visible = 1;
		gameover->location.x = eye.x;
		gameover->location.z = eye.z;
	}
}


void templateAppToucheBegan( float x, float y, unsigned int tap_count )
{
	start_pos.x = x;
	start_pos.y = y;
}


void templateAppToucheEnded( float x, float y, unsigned int tap_count )
{
	if( gameover->visible && !restart_game )
	{
		restart_game = 1;
		return;
	}

	if( momo && !momo_launch) { momo_launch = 1;

		momo->btrigidbody->forceActivationState( ACTIVE_TAG );

		momo->btrigidbody->setLinearVelocity( 
		btVector3( CLAMP( ( y - start_pos.y ) * 0.1f, 0.0f, 10.0f ),
				   0.0f,
				   CLAMP( ( x - start_pos.x ) * 0.1f, 0.0f, 10.0f ) ) );
	} 
		
}


void templateAppExit( void ) {

	if( dynamicsworld ) free_physic_world();

	if( program )
	{
		SHADER_free( program->vertex_shader );

		SHADER_free( program->fragment_shader );

		PROGRAM_free( program );
	}
	
	if( obj ) OBJ_free( obj );
}
