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

#define OBJ_FILE (char *)"game.obj"

#define VERTEX_SHADER (char *)"vertex.glsl"

#define FRAGMENT_SHADER (char *)"fragment.glsl"

OBJ *obj = NULL;

PROGRAM *program = NULL;

TEMPLATEAPP templateApp = { templateAppInit,
							templateAppDraw,
							templateAppToucheBegan,
							NULL,
							templateAppToucheEnded };

vec2 start_pos = { 0.0f, 0.0f };

/* Index of the current Momo.  Since the momo objects are name respectively
 * momo1, momo2, and so on, keeking an index enables you to easily retrieve
 * the current OBJMESH pointer for the current index by dynamically creating
 * the name of the mesh in code.
 */
unsigned int momo_index = 0;
/* Pointer to current Momo mesh. */
OBJMESH *momo = NULL;

OBJMESH *gameover = NULL;   /* To remember the gameover object. */

bool            restart_game = false,   // Flag to restart the game.
                momo_launch  = false;   // Flag to let momo be thrown.
unsigned char   banana       = 0;       // Banana counter.

vec3 eye    = { 3.5f, -10.8f, 5.3f },
	 center = { 3.5f,  -9.8f, 5.3f },
	 up     = { 0.0f,   0.0f, 1.0f };


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

enum ShapeType
{
    BOX      = 0,
    SPHERE   = 1,
    CYLINDER = 2
};

void add_rigid_body(OBJMESH *objmesh, ShapeType bound, float mass, bool dynamic_only)
{
    /* Initialize a blank collision shape pointer. */
    btCollisionShape *btcollisionshape = NULL;

    switch (bound) {
        /* If the current bound is a box, do the same as in the previous
         * sections.
         */
        case BOX:
        {
            btcollisionshape = new btBoxShape(btVector3(objmesh->dimension.x * 0.5f,
                                                        objmesh->dimension.y * 0.5f,
                                                        objmesh->dimension.z * 0.5f));
            break;
        }
        /* If it's a sphere, initialize a new sphere shape, passing in as a
         * parameter the radius of the mesh (which is automatically pre-calculated
         * when OBJMESH::build is called.
         */
        case SPHERE:
        {
            btcollisionshape = new btSphereShape(objmesh->radius);
            break;
        }
        /* Handle the cylinder shape, which initialized the same was as a
         * Box shape, by passing the bounding box extent to the constructor.
         */
        case CYLINDER:
        {
            btcollisionshape = new btCylinderShapeZ(btVector3(objmesh->dimension.x * 0.5f,
                                                              objmesh->dimension.y * 0.5f,
                                                              objmesh->dimension.z * 0.5f));
            break;
        }
    }

    btTransform bttransform;

    bttransform.setIdentity();

    /* Set the origin location of the transformation, which is basically
     * the pivot XYZ location of the object.  Note that the origin should always be
     * the center of the bounding box of the object in world space coordinates.
     */
    bttransform.setOrigin(btVector3(objmesh->location.x,
                                    objmesh->location.y,
                                    objmesh->location.z));

    btDefaultMotionState *btdefaultmotionstate = NULL;

    btdefaultmotionstate = new btDefaultMotionState(bttransform);

    btVector3 localinertia(0.0f, 0.0f, 0.0f);

    /* If the function receives a positive mass, calculate the local inertia
     * tensor of the object.
     */
    if (mass > 0.0f)
        btcollisionshape->calculateLocalInertia(mass, localinertia);

    objmesh->btrigidbody = new btRigidBody(mass,
                                           btdefaultmotionstate,
                                           btcollisionshape,
                                           localinertia);

    /* Constraint the linear velocity (the movementof the object) to the
     * XZ axis by setting the Y linear factor to zero.
     */
    if (mass > 0.0f) {
        objmesh->btrigidbody->setLinearFactor(btVector3(1.0f, 0.0f, 1.0f));

        /* Check to see if the object is not a dynamic object.  If it's not
         * you have to prevent the object from rolling on the XZ axis.  To do this,
         * in the same way you handled the linear factor, pass zero as the XZ value
         * of the angular factor of the rigid body.  This way, all you non-dynamic
         * objects will roll only on the Y-axis.
         */
        if (!dynamic_only) {
            objmesh->btrigidbody->setAngularFactor(btVector3(0.0f, 1.0f, 0.0f));
        } else {
            /* If the object is a fully dynamic object, set the XYZ angular factor
             * to 0, so the object (such as the bananas) won't respond to rolling
             * physics at all.
             */
            objmesh->btrigidbody->setAngularFactor(0.0f);
        }
    }


    objmesh->btrigidbody->setUserPointer(objmesh);

    dynamicsworld->addRigidBody(objmesh->btrigidbody);
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


bool contact_added_callback(btManifoldPoint &btmanifoldpoint,
                            const btCollisionObject *btcollisionobject0,
                            int part_0, int index_0,
                            const btCollisionObject *btcollisionobject1,
                            int part_1, int index_1) {

	OBJMESH *objmesh0 = (OBJMESH *)((btRigidBody *)btcollisionobject0)->getUserPointer();

	OBJMESH *objmesh1 = (OBJMESH *)((btRigidBody *)btcollisionobject1)->getUserPointer();
	
    /* Check if one of the two objects involved in the collision is a
     * momo and the other object is a banana. */
    if ((strstr(objmesh0->name, "momo") && strstr(objmesh1->name, "banana")) ||
        (strstr(objmesh0->name, "banana") && strstr(objmesh1->name, "momo"))) {

        OBJMESH *objmesh = NULL;
        btCollisionObject *btcollisionobject = NULL;

        /* Save the mesh and collision information for the banana. */
        if (strstr(objmesh0->name, "banana")) {
            objmesh = objmesh0;
            btcollisionobject = (btCollisionObject *)btcollisionobject0;
        } else {
            objmesh = objmesh1;
            btcollisionobject = (btCollisionObject *)btcollisionobject1;
        }

        /* We're taking this banana out of play; make it invisible. */
        objmesh->visible = false;
        /* Reduce the banana count.  If this reaches zero the player wins? */
        --banana;
        /* The following lines will remove the banana from the physical
         * world.  First delete the collision shape of the rigid body.
         */
        delete objmesh->btrigidbody->getCollisionShape();
        /* Delete the motion state. */
        delete objmesh->btrigidbody->getMotionState();
        /* Remove the rigid body from the physical world. */
        dynamicsworld->removeRigidBody(objmesh->btrigidbody);
        /* Remove the collision object from the physical world. */
        dynamicsworld->removeCollisionObject(btcollisionobject);

        delete objmesh->btrigidbody;

        objmesh->btrigidbody = NULL;
    }

	return false;
}


void program_bind_attrib_location(void *ptr) {
    PROGRAM *program = (PROGRAM *)ptr;

    glBindAttribLocation(program->pid, VA_Position,  VA_Position_String);
    glBindAttribLocation(program->pid, VA_TexCoord0, VA_TexCoord0_String);
}

void get_next_momo(void)
{
    /* Temp. characters to dynamically create the mesh name. */
    char tmp[MAX_CHAR] = {""};

    momo = NULL;

    ++momo_index;

    sprintf(tmp, "momo%d", momo_index);

    /* Find the mesh named in tmp. */
    for (auto objmesh=obj->objmesh.begin();
         objmesh!=obj->objmesh.end(); ++objmesh) {
        if (strstr(objmesh->name, tmp)) {
            /* Mesh found.  Update OBJMESH pointer */
            momo = &*objmesh;
            // This momo not yet launched?  CRL
            momo_launch = false;
            /* Disable the deactivation of the rigid body.  Since the object
             * will be thrown in the air, it has to be active in order to
             * respond to the linear velocity movement that your are going to
             * assign to it.  If the rigid body is disabled, the object won't
             * respond, even if you affect the velocity.  By calling the line
             * below, you can be sure that the object will stay "alive" and
             * ready to be thrown.
             */
            momo->btrigidbody->setActivationState(DISABLE_DEACTIVATION);

            return;
        }
    }
}

void load_game(void)
{
	init_physic_world();
	
	gContactAddedCallback = contact_added_callback;

	obj = new OBJ(OBJ_FILE, true);

    for (auto objmesh=obj->objmesh.begin();
         objmesh!=obj->objmesh.end(); ++objmesh) {

		objmesh->optimize(128);
		
		objmesh->build();

        if (strstr(objmesh->name, "momo")) {
            /* For each momo, create a sphere shape, with a mass of 2kg which
             * responds to rolling physics.
             */
            add_rigid_body(&*objmesh, SPHERE, 2.0f, false);
        } else if (strstr(objmesh->name, "barrel")) {
            /* For the barrels, create a cylinder shape with a mass of 1kg
             * (so Momo can bash them easily) that also responds to rolling
             * physics.
             */
            add_rigid_body(&*objmesh, CYLINDER, 1.0f, false);
        } else if (strstr(objmesh->name, "plank")) {
            /* Initialize each plank as a box, with a mass of 1kg that also
             * responds to angular velocity.
             */
            add_rigid_body(&*objmesh, BOX, 1.0f, false);
        } else if (strstr(objmesh->name, "ground")) {
            /* Create a static box for the ground. */
            add_rigid_body(&*objmesh, BOX, 0.0f, false);
        } else if (strstr(objmesh->name, "steel")) {
            /* Create a static cylinder for the steel barrel. */
            add_rigid_body(&*objmesh, CYLINDER, 0.0f, false);
        } else if (strstr(objmesh->name, "banana")) {
            /* Create a sphere shape for each banana that has a mass of 1kg
             * and that does not respond to rolling physics.  You do not want
             * the bananas to start rollin around!
             */
            add_rigid_body(&*objmesh, SPHERE, 1.0f, true);
            /* Add to the rigid body collision flags the tags needed to respond
             * to custom material callbacks (contact added).
             */
            objmesh->btrigidbody->setCollisionFlags(objmesh->btrigidbody->getCollisionFlags() | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
            /* Until disturbed, keep the bananas from responding to the force
             * of gravity.
             */
            objmesh->btrigidbody->forceActivationState (ISLAND_SLEEPING);

            ++banana;
        } else if (strstr(objmesh->name, "gameover")) {
            /* Hide the "gameover" object until the game is over!  ;-) */
            objmesh->visible = false;

            gameover = &*objmesh;
        }

		objmesh->free_vertex_data();
	}
	

    for (auto texture=obj->texture.begin();
         texture!=obj->texture.end(); ++texture) {
        (*texture)->build(obj->texture_path,
                          TEXTURE_MIPMAP | TEXTURE_16_BITS,
                          TEXTURE_FILTER_2X,
                          0.0f);
    }

	program = new PROGRAM((char *)"default",
                          VERTEX_SHADER,
                          FRAGMENT_SHADER,
                          true,
                          false,
                          program_bind_attrib_location,
                          NULL);

    for (auto objmaterial=obj->objmaterial.begin();
         objmaterial!=obj->objmaterial.end(); ++objmaterial)
		objmaterial->build(NULL);

	program->draw();
	
	glUniform1i(program->get_uniform_location(TM_Diffuse_String), TM_Diffuse);

    momo_index  = 0;
    momo_launch = false;
    center.x =
    eye.x    = 3.5f;

    get_next_momo();
}


void templateAppInit(int width, int height) {

	atexit(templateAppExit);

	GFX_start();
	
	glViewport(0.0f, 0.0f, width, height);

	GFX_set_matrix_mode(PROJECTION_MATRIX);
	GFX_load_identity();
	
	GFX_set_orthographic((float)height / (float)width,
                         15.0f,
                         (float)width / (float)height,
                         1.0f,
                         100.0f,
                         -90.0f);
	load_game();
}


void templateAppDraw(void) {

	glClear(GL_DEPTH_BUFFER_BIT);

    /* If the restart flag is != 0, call the templateAppExit() function
     * to clear the scene, and then reload it using the load_game() function.
     * Finally, reset the restart_game flag back to false to avoid loading again
     * and again if the flag is raised.
     */
    if (restart_game) {
        templateAppExit();

        load_game();

        restart_game = false;
    }

	GFX_set_matrix_mode(MODELVIEW_MATRIX);
	GFX_load_identity();
	
    if (momo) {
        /* Linearly interpolate the camera eye position with the current
         * location of momo on the X axis.
         */
        eye.x = eye.x * 0.98f + momo->location.x * 0.02f;
        /* Clamp the camera X position to be in the range of -2 to 3.5.
         * That way, even if Momo is going off screen, you will stop tracking it.
         */
        center.x =
        eye.x = CLAMP(eye.x, -2.0f, 3.5f);
    }

	GFX_look_at(&eye, &center, &up);

    for (auto objmesh=obj->objmesh.begin();
         objmesh!=obj->objmesh.end(); ++objmesh) {

		GFX_push_matrix();

        /* Check if the current mesh has a rigid body pointer. */
        if (objmesh->btrigidbody) {
            mat4 mat;
            /* Get the current transformation matrix from Bullet. */
            objmesh->btrigidbody->getWorldTransform().getOpenGLMatrix((float *)&mat);
            /* Up date the X location based on the current OpenGL matrix value. */
            // Why?  CRL
            objmesh->location.x = mat.m[3].x;
            /* Add Bullet's calculation of the object's position/orientation
             * to the "stack" of transformations contained in the modelview
             * matrix.
             */
            GFX_multiply_matrix(&mat);
        } else {
            GFX_translate(objmesh->location.x,
                          objmesh->location.y,
                          objmesh->location.z);
        }
		
		glUniformMatrix4fv(program->uniform_map["MODELVIEWPROJECTIONMATRIX"].location,
                           1,
                           GL_FALSE,
                           (float *)GFX_get_modelview_projection_matrix());

		objmesh->draw();

		GFX_pop_matrix();
	}
	
	dynamicsworld->stepSimulation(1.0f / 60.0f);

    if (momo &&
       (momo->btrigidbody->getLinearVelocity().length() > 20.0f ||
        momo->btrigidbody->getActivationState() == ISLAND_SLEEPING))
        get_next_momo();

    if (!momo || !banana) {
        gameover->visible = true;
        gameover->location.x = eye.x;
        gameover->location.z = eye.z;
    }
}


void templateAppToucheBegan(float x, float y, unsigned int tap_count)
{
	start_pos.x = x;
	start_pos.y = y;
}


void templateAppToucheEnded(float x, float y, unsigned int tap_count)
{
    if (gameover->visible && !restart_game) {
        restart_game = true;
        return;
    }

    if (momo && !momo_launch) {
        /* This flag is to keep the player from giving an already flying momo
         * an additional boost.
         */
        momo_launch = true;
        /* Force the activation state from DISABLE_DEACTIVATION to
         * ACTIVE_TAG.  This way, as soon as the object is thrown in the air, it
         * can then become deactivated when it lands and becomes immobile.
         */
        momo->btrigidbody->forceActivationState(ACTIVE_TAG);
        /* Use the direction vector created by the swipe of the user to
         * assign the linear velocity of the object.  In addition, to avoid a
         * very high velocity (since the vector is in pixels, and Bullet is
         * working in meters), adjust the value by multiplying it by 0.1f and
         * clamping it in the range of 0 to 10.  Note that the X and Y values are
         * inverted because you are in landscape mode.
         */
        momo->btrigidbody->setLinearVelocity(btVector3(CLAMP((y - start_pos.y) * 0.1f, 0.0f, 10.0f),
                                                       0.0f,
                                                       CLAMP((x - start_pos.x) * 0.1f, 0.0f, 10.0f)));
    }
}


void templateAppExit(void) {

	if (dynamicsworld) free_physic_world();

	if (program) {
        delete program;
        program = NULL;
	}
	
	delete obj;
}
