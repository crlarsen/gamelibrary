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

float screen_size;

vec2 view_location,
 	 view_delta = { 0.0f, 0.0f };

vec3 move_location = { 0.0f, 0.0f, 0.0f },
	 move_delta;

vec3 eye,
     up = { 0.0f, 0.0f, 1.0f };

OBJMESH *player = NULL;

/* Variable that you are going to use to interpolate the current eye
 * position to the next.
 */
vec3 next_eye;
/* The camera rotation on the X axis, with a default value of -165. */
 float rotx = -165.0f,
       /* To interpolate the camera rotationX.  At initialization, give it
        * the same value as the initial X rotation.
        */
       next_rotx = rotx,
       rotz = 180.0f,
       /* Similar to the rotx variable, but for the Z axis rotation. */
       next_rotz = rotz,
       /* The camera will orbit around the player (the 3D ball).  This variable
        * determines the default distance the eye should be from the object
        * (assuming there is no collision with the walls).
        */
       distance = -5.0f;

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

	MEMORY *memory = mopen(PHYSIC_FILE, true);

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

    glBindAttribLocation(program->pid, VA_Position,  VA_Position_String);
    glBindAttribLocation(program->pid, VA_TexCoord0, VA_TexCoord0_String);
}



class ClosestNotMeRayResultCallback:public btCollisionWorld::ClosestRayResultCallback {

public:
    ClosestNotMeRayResultCallback(btRigidBody *rb,
                                  const btVector3 &p1,
                                  const btVector3 &p2) :
    btCollisionWorld::ClosestRayResultCallback(p1, p2)
    { m_btRigidBody = rb; }

    virtual btScalar addSingleResult(btCollisionWorld::LocalRayResult &localray,
                                     bool normalinworldspace)
    {
        if (localray.m_collisionObject == m_btRigidBody)
            return 1.0f;

        return ClosestRayResultCallback::addSingleResult(localray, normalinworldspace);
    }

protected:
    btRigidBody *m_btRigidBody;
};

void templateAppInit(int width, int height) {
	screen_size = height;

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

	player = obj->get_mesh("player", false);
	
	player->btrigidbody->setFriction(10.0f);
	
	memcpy(&eye, &player->location, sizeof(vec3));
	
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
	
	glUniform1i(program->get_uniform_location(TM_Diffuse_String), TM_Diffuse);
}


void templateAppDraw(void) {
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);


	GFX_set_matrix_mode(MODELVIEW_MATRIX);
	GFX_load_identity();

    /* First check if the direction vector on the X or Y axis was triggered
     * from the user touch on the right side of the screen.
     */
    if (view_delta.x || view_delta.y) {
        /* If the Y is active (!=0), then add the value to the next Z
         * rotation.  Since you are going to interpolate the rotation, you have to
         * assign the value to the next camera Z rotation.
         */
        if (view_delta.y) next_rotz -= view_delta.y;
        /* Same as above, but this time for the X rotation axis.  In addition,
         * clamp the value in the range of -180 to -90 degrees to allow the camera
         * to only look from straight up to straight down.
         */
        if (view_delta.x) {
            next_rotx -= view_delta.x;
            next_rotx = CLAMP(next_rotx, -180.0f, -90.0f);
        }
        /* Reset the view deltas to avoid triggering another pass inside this
         * block on the next rendering pass.
         */
        view_delta.x =
        view_delta.y = 0.0f;
    }

    /* If you got a force comming from the left side of the screen. */
    if (move_delta.z) {
        /* Temp. variable to calculate the direction (aka forward) vector. */
        vec3 direction;
        /* Rotate the coordinate system to fit the current Z rotation
         * of the camera.
         */
        float r = rotz * DEG_TO_RAD,
              c = cosf(r),
              s = sinf(r);

        direction.x = c * move_delta.y - s * move_delta.x;
        direction.y = s * move_delta.y + c * move_delta.x;

        /* Assign the direction vector to the angular velocity of the ball. */
        player->btrigidbody->setAngularVelocity(btVector3(direction.y * (move_delta.z * 6.7f),
                                                          -direction.x * (move_delta.z * 6.7f),
                                                          0.0f));
        /* Make sure the state of the rigid body is active in order to
         * trigger the rotation.
         */
        player->btrigidbody->setActivationState(ACTIVE_TAG);
    }

    next_eye.x = player->location.x +
                 distance *
                 cosf(rotx * DEG_TO_RAD) *
                 sinf(rotz * DEG_TO_RAD);

    next_eye.y = player->location.y -
                 distance *
                 cosf(rotx * DEG_TO_RAD) *
                 cosf(rotz * DEG_TO_RAD);


    next_eye.z = player->location.z +
                 distance *
                 sinf(rotx * DEG_TO_RAD);

    /* Declare the starting point and end point of the collision ray.
     * Basically, what you are trying to achieve is that the ray starts
     * from the ball and aims straight at the next_eye position.  If anthing
     * collides with the ray (with the exception of the ball), you need to
     * re-adjust the next_eye position to be located where this is a hit.
     * This will prevent the camera from seeing through walls and insure
     * that the ball is focused at all times.
     */
    btVector3 p1(player->location.x,
                 player->location.y,
                 player->location.z),

              p2(next_eye.x,
                 next_eye.y,
                 next_eye.z);
    /* Initialize the collision ray, passing in as parameters the ball rigid
     * body pointer and the start and end points of the ray.
     */
    ClosestNotMeRayResultCallback back_ray(player->btrigidbody,
                                           p1,
                                           p2);
    /* Launch the ray in 3D space. */
    dynamicsworld->rayTest(p1,
                           p2,
                           back_ray);
    /* If the collision ray got hit. */
    if (back_ray.hasHit()) {
        /* Normalize the hit point normal. */
        back_ray.m_hitNormalWorld.normalize();
        /* Adjust the next_eye position to be located where the collision ray
         * hits inside the world.  In addition, to make sure that the camera
         * stays inside the scene and does not simple "stick" on the wall, add
         * a slight offset based on the bit point normal.  This will ensure that
         * the camera next_position will always be located in front of where
         * the collision ray hits.
         */
        next_eye.x =   back_ray.m_hitPointWorld.x() +
        (back_ray.m_hitNormalWorld.x() * 0.1f);

        next_eye.y =   back_ray.m_hitPointWorld.y() +
        (back_ray.m_hitNormalWorld.y()* 0.1f);

        next_eye.z =   back_ray.m_hitPointWorld.z() +
        (back_ray.m_hitNormalWorld.z()* 0.1f);
    }

    /* Linearly interpolate the rotation between the current and the next. */
    rotx = rotx * 0.9f + next_rotx * 0.1f;
    rotz = rotz * 0.9f + next_rotz * 0.1f;

    /* Same as for the rotation, but this time for the current eye position. */
    eye.x = eye.x * 0.95f + next_eye.x * 0.05f;
    eye.y = eye.y * 0.95f + next_eye.y * 0.05f;
    eye.z = eye.z * 0.95f + next_eye.z * 0.05f;

    /* Give an offset to the player Z location to make sure that the camera
     * is always looking at the top of the ball and not at it's center.  This
     * way, even in tight corners, the user will always be able to see in front
     * of the ball.
     */
    player->location.z += player->dimension.z * 0.5f;

    /* Feed the current eye position and player location to the GFX_look_at
     * function to be able to generate the view matrix.
     */
    GFX_look_at(&eye,
                &player->location,
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
	if (y < (screen_size * 0.5f)) {
		move_location.x = x;
		move_location.y = y;
	} else {
		view_location.x = x;
		view_location.y = y;
	}
}


void templateAppToucheMoved(float x, float y, unsigned int tap_count)
{
	if (y > ((screen_size * 0.5f) - (screen_size * 0.05f)) && 
		y < ((screen_size * 0.5f) + (screen_size * 0.05f))) {
		
		move_delta.z =
		view_delta.x =
		view_delta.y = 0.0f;
		
		move_location.x = x;
		move_location.y = y;
		
		view_location.x = x;
		view_location.y = y;
	} else if (y < (screen_size * 0.5f)) {
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
	} else {
		view_delta.x = view_delta.x * 0.75f + (x - view_location.x) * 0.25f;
		view_delta.y = view_delta.y * 0.75f + (y - view_location.y) * 0.25f;

		view_location.x = x;
		view_location.y = y;
	}
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
