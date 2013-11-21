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
/*
 * Source code modified by Chris Larsen to make the following data types into
 * proper C++ classes:
 * - OBJ
 * - OBJMATERIAL
 * - OBJMESH
 * - OBJTRIANGLEINDEX
 * - OBJTRIANGLELIST
 * - OBJVERTEXDATA
 * - PROGRAM
 * - SHADER
 * - TEXTURE
 */

#include "templateApp.h"

#define OBJ_FILE (char *)"Scene.obj"

#define PHYSIC_FILE (char *)"Scene.bullet"

#define VERTEX_SHADER (char *)"vertex.glsl"

#define FRAGMENT_SHADER (char *)"fragment.glsl"

OBJ *obj = NULL;

PROGRAM *program = NULL;

vec3 move_location,
	 move_delta;
/* The variables that you are going to plug into the GFX_look_at function. */
vec3 eye,
     center,
     up = { 0.0, 0.0, 1.0f };
/* Global OBJMESH to remember the player object pointer. */
OBJMESH *player = NULL;

TEMPLATEAPP templateApp = { templateAppInit,
							templateAppDraw,
							templateAppToucheBegan,
							templateAppToucheMoved,
							templateAppToucheEnded };


btSoftBodyRigidBodyCollisionConfiguration *collisionconfiguration = NULL;

btCollisionDispatcher *dispatcher = NULL;

btBroadphaseInterface *broadphase = NULL;

btConstraintSolver *solver = NULL;

btSoftRigidDynamicsWorld *dynamicsworld = NULL;


void init_physic_world(void)
{
	collisionconfiguration = new btSoftBodyRigidBodyCollisionConfiguration();

	dispatcher = new btCollisionDispatcher(collisionconfiguration);

	broadphase = new btDbvtBroadphase();

	solver = new btSequentialImpulseConstraintSolver();

	dynamicsworld = new btSoftRigidDynamicsWorld(dispatcher,	
												  broadphase,
												  solver,
												  collisionconfiguration);

	dynamicsworld->setGravity(btVector3(0.0f, 0.0f, -9.8f));
}


void load_physic_world(void)
{
	btBulletWorldImporter *btbulletworldimporter = new btBulletWorldImporter(dynamicsworld);

	MEMORY *memory = mopen(PHYSIC_FILE, 1);

	btbulletworldimporter->loadFileFromMemory((char *)memory->buffer, memory->size);

	mclose(memory);

	for (int i=0; i!=btbulletworldimporter->getNumRigidBodies(); ++i) {
		OBJMESH *objmesh = obj->get_mesh(btbulletworldimporter->getNameForPointer(
										 btbulletworldimporter->getRigidBodyByIndex(i)), false); 

		if (objmesh) { 
			objmesh->btrigidbody = (btRigidBody *)btbulletworldimporter->getRigidBodyByIndex(i);
			
			objmesh->btrigidbody->setUserPointer(objmesh);
		} 
	}

	delete btbulletworldimporter;
}


void free_physic_world(void)
{
	while (dynamicsworld->getNumCollisionObjects()) {
		btCollisionObject *btcollisionobject = dynamicsworld->getCollisionObjectArray()[0];
		
		btRigidBody *btrigidbody = btRigidBody::upcast(btcollisionobject);

		if (btrigidbody) {
			delete btrigidbody->getCollisionShape();
			
			delete btrigidbody->getMotionState();
			
			dynamicsworld->removeRigidBody(btrigidbody);
			
			dynamicsworld->removeCollisionObject(btcollisionobject);
			
			delete btrigidbody;
		}
	}
	
	delete collisionconfiguration; collisionconfiguration = NULL;

	delete dispatcher; dispatcher = NULL;

	delete broadphase; broadphase = NULL;

	delete solver; solver = NULL;
	
	delete dynamicsworld; dynamicsworld = NULL;	
}


void program_bind_attrib_location(void *ptr) {
	PROGRAM *program = (PROGRAM *)ptr;

	glBindAttribLocation(program->pid, 0, "POSITION");
	glBindAttribLocation(program->pid, 2, "TEXCOORD0");
}


void templateAppInit(int width, int height) {
	atexit(templateAppExit);

	GFX_start();

	glViewport(0.0f, 0.0f, width, height);

	GFX_set_matrix_mode(PROJECTION_MATRIX);
	GFX_load_identity();
	
	GFX_set_perspective(80.0f,
						 (float)width / (float)height,
						 0.1f,
						 100.0f,
						 -90.0f);

	obj = new OBJ(OBJ_FILE, true);

	for (auto objmesh=obj->objmesh.begin();
         objmesh!=obj->objmesh.end(); ++objmesh) {
		objmesh->optimize(128);

		objmesh->build();
		
		objmesh->free_vertex_data();
	}
	
	
	init_physic_world();
	
	load_physic_world();
	
    /* Get the OBJMESH pointer for the player object. */
    player = obj->get_mesh("player", false);
    /* Set the friction of the rigid body to 10.  With a high friction, the
     * ball will not spin on itself before moving to the appropriate locations, but
     * will "stick" on the floor and execute the movement right away.
     */
    player->btrigidbody->setFriction(10.0f);
    /* Copy the initial location of the ball to the eye variable and to the
     * center variable.  This way, you have an initial location where the camera is
     * going to start looking the program starts. */
    memcpy(&eye, &player->location, sizeof(vec3));
		
    memcpy(&center, &player->location, sizeof(vec3));
    /* Give a little offset on the Y axis to make sure that the scene can be
     * covered by the camera eye position (well at least mostly).
     */
    eye.y -= 3.0f;

    for (auto texture=obj->texture.begin();
         texture!=obj->texture.end(); ++texture) {
        (*texture)->build(obj->texture_path,
                          TEXTURE_MIPMAP | TEXTURE_16_BITS,
                          TEXTURE_FILTER_2X,
                          0.0f);
    }

    
	for (auto objmaterial=obj->objmaterial.begin();
         objmaterial!=obj->objmaterial.end(); ++objmaterial) {
		objmaterial->build(NULL);
    }

	program = new PROGRAM((char *)"default",
                          VERTEX_SHADER,
                          FRAGMENT_SHADER,
                          true,
                          false,
                          program_bind_attrib_location,
                          NULL);

	program->draw();
	
	glUniform1i(program->get_uniform_location((char *)"DIFFUSE"), 1);
}


void templateAppDraw(void) {
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);


	GFX_set_matrix_mode(MODELVIEW_MATRIX);
	GFX_load_identity();
	
    /* First check if you have a force. */
    if (move_delta.z) {
        /* Just like you did in the previous chapter for the capsule shape,
         * assign the movement delta (aka the direction vector) coming from
         * the touch screen to the ball collision shape.
         *
         * Reverse the move_delta.x to fit the current coordinate system.
         */
        player->btrigidbody->setAngularVelocity(btVector3(-move_delta.x * move_delta.z * 6.7f,
                                                          move_delta.y * move_delta.z * 6.7f,
                                                          0.0f));

        /* Activate the rigid body; otherwise the setAngularVelocity call
         * will have no affect if the ball is deactivated.
         */
        player->btrigidbody->setActivationState(ACTIVE_TAG);
    }

    /* Linearly interpolate the current center point of the camera with the
     * current location of the player object in space.
     */
    center.x = center.x * 0.975f + player->location.x * 0.025f;
    center.y = center.y * 0.975f + player->location.y * 0.025f;
    center.z = center.z * 0.975f + player->location.z * 0.025f;

    GFX_look_at(&eye,
                &center,
                &up);

	for (auto objmesh=obj->objmesh.begin();
         objmesh!=obj->objmesh.end(); ++objmesh) {

		GFX_push_matrix();

		mat4 mat;
		
		objmesh->btrigidbody->getWorldTransform().getOpenGLMatrix((float *)&mat);
		
		memcpy(&objmesh->location, (vec3 *)&mat.m[3], sizeof(vec3));

		GFX_multiply_matrix(&mat);		

		glUniformMatrix4fv(program->uniform_map["MODELVIEWPROJECTIONMATRIX"].location,
                           1,
                           GL_FALSE,
                           (float *)GFX_get_modelview_projection_matrix());

		objmesh->draw();

		GFX_pop_matrix();
	}
	
	dynamicsworld->stepSimulation(1.0f / 60.0f);
}


void templateAppToucheBegan(float x, float y, unsigned int tap_count)
{
	move_location.x = x;
	move_location.y = y;
}


void templateAppToucheMoved(float x, float y, unsigned int tap_count)
{
	vec3 touche = { x, 
					y,
					0.0f };
	
	vec3_diff(&move_delta,
              &move_location,
              &touche);

	vec3_normalize(&move_delta,
                   &move_delta);

	move_delta.z = CLAMP(vec3_dist(&move_location, &touche) / 128.0f,
                         0.0f,
                         1.0f);		
}


void templateAppToucheEnded(float x, float y, unsigned int tap_count)
{
	move_delta.z = 0.0f;
}


void templateAppExit(void) {
	free_physic_world();
	
    delete program;
    program = NULL;

    delete obj;
}
