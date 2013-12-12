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
 * - MEMORY
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

#define VERTEX_SHADER (char *)"vertex.glsl"

#define FRAGMENT_SHADER (char *)"fragment.glsl"

OBJ *obj = NULL;

PROGRAM *program = NULL;

TEMPLATEAPP templateApp = {
    templateAppInit,
    templateAppDraw
};

/* The collision world configuration. */
btSoftBodyRigidBodyCollisionConfiguration *collisionconfiguration = NULL;

/* The collision dispatcher to use for the collision world. */
btCollisionDispatcher *dispatcher = NULL;

/* Contain the algorithm to use to quickly calculate and maintain the
 * list of objects that are colliding, as well as the information about
 * the objects that intersect but are not close enough to collide.
 */
btBroadphaseInterface *broadphase = NULL;

/* Contain the algorithm to use to solve the physics constrains added to
 * the world (if any).
 */
btConstraintSolver *solver = NULL;

/* Declare a physical world variable capable of dealing with rigid and
 * soft bodies and their interactions.
 */
btSoftRigidDynamicsWorld *dynamicsworld = NULL;

void init_physic_world(void)
{
    /* Initialize a new collision configuration. */
    collisionconfiguration = new btSoftBodyRigidBodyCollisionConfiguration();

    /* Initialize the collision dispatcher. */
    dispatcher = new btCollisionDispatcher(collisionconfiguration);

    /* Determine which broad phase algorithm to use for the current
     * physical world.  The btDbvtBroadphase is the one that gives the best result
     * (from what I've observed) on most of the mobile devices for generic
     * physical world.  However Bullet offer many others; check the Bullet SDK
     * for more information.
     */
    broadphase = new btDbvtBroadphase();

    /* Initialize the constraint solver. */
    solver = new btSequentialImpulseConstraintSolver();

    /* Now that you have all the necessary variables and algorithms
     * initialized, you are ready to create your physical world.
     */
    dynamicsworld = new btSoftRigidDynamicsWorld(dispatcher,
                                                 broadphase,
                                                 solver,
                                                 collisionconfiguration);

    /* And finally, set up the world gravity direction vector using the
     * same value as the gravity on earth by assigning it on the -Z-axis.
     */
    dynamicsworld->setGravity(btVector3(0.0f, 0.0f, -9.8f));
}

void add_rigid_body(OBJMESH *objmesh, float mass)
{
    /* Create a new Box collision shape for the current mesh. */
    /* Use half of the dimension XYZ to represent the extent of the box
     * relative to its pivot point, which is already centered in the middle of
     * its bounding box.
     */
    btCollisionShape *btcollisionshape = new btBoxShape(btVector3(objmesh->dimension[0] * 0.5f,
                                                                  objmesh->dimension[1] * 0.5f,
                                                                  objmesh->dimension[2] * 0.5f));

    /* Declare a btTransform variable to be able to contain the transformation
     * matrix of the object in a form that Bullet will understand.
     */
    btTransform bttransform;

    mat4 mat;

    /* Declare 3 vectors to be able to hold the rotation of the mesh on
     * the XYZ axis.
     */
    vec4 rotx(1.0f, 0.0f, 0.0f, objmesh->rotation[0]),
         roty(0.0f, 1.0f, 0.0f, objmesh->rotation[1]),
         rotz(0.0f, 0.0f, 1.0f, objmesh->rotation[2]);

    /* Set up the identity matrix to makesure the matrix is clean. */
    mat4_identity(mat);

    mat4_translate(mat, mat, objmesh->location);

    mat4_rotate(mat, mat, rotz);

    mat4_rotate(mat, mat, roty);

    mat4_rotate(mat, mat, rotx);

    /* Assign the current transformation matrix that you create using the
     * standard "OpenGL way" and send it over to the Bullet transform variable.
     */
    bttransform.setFromOpenGLMatrix((float *)&mat);

    /* Create a new motion state in order for Bullet to be able to
     * maintain and interpolate the object transformation.
     */
    btDefaultMotionState *btdefaultmotionstate = NULL;

    btdefaultmotionstate = new btDefaultMotionState(bttransform);

    /* Create a Bullet vector to be able to hold the local inertia of
     * the object.
     */
    btVector3 localinertia(0.0f, 0.0f, 0.0f);

    /* If a mass greater than 0 is passed in as a parameter to the function,
     * use it to calculate the local inertia.  If a mass is equal to 0, it means
     * that the object is static and you do not need to execute this calculation.
     */
    if (mass > 0.0f)
        btcollisionshape->calculateLocalInertia(mass, localinertia);

    /* Create a new rigid body and link the information that you have
     * calculated above.  Note that you are using the btRigidBody pointer already
     * contained in the OBJMESH structure to initialize the class.  This way, when
     * you're drawing, you can easily query the pointer in order to gain access to
     * its transformation matrix, which is from now on maintained by Bullet
     * internally.
     */
    objmesh->btrigidbody = new btRigidBody(mass,
                                           btdefaultmotionstate,
                                           btcollisionshape,
                                           localinertia);

    /* Built into the btRigidBody class is a "void *" variable that
     * allows you to associate a user-defined pointer with the rigid
     * body.  By associating the current objmesh pointer to this
     * variable, you will have direct access to the OBJMESH structure
     * at any time inside any Bullet-driven functions and callbacks.
     */
    objmesh->btrigidbody->setUserPointer(objmesh);

    /* Add the new rigid body to your physical world. */
    dynamicsworld->addRigidBody(objmesh->btrigidbody);
}

void free_physic_world(void)
{
    /* Loop while you have some collision objects. */
    while (dynamicsworld->getNumCollisionObjects()) {
        /* Get the first collision object in the list. */
        btCollisionObject *btcollisionobject = dynamicsworld->getCollisionObjectArray()[0];

        /* Try to upcast it to a rigid body. */
        btRigidBody *btrigidbody = btRigidBody::upcast(btcollisionobject);

        /* If the upcast is successful, the pointer will be != NULL,
         * so you know that you are dealing with a valid btRigidBody.
         */
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

void near_callback(btBroadphasePair &btbroadphasepair,
                   btCollisionDispatcher  &btdispatcher,
                   const btDispatcherInfo &btdispatcherinfo)
{
    /* Retrieve the two meshes which are part of the collision. */
    OBJMESH *objmesh0 = (OBJMESH *)((btRigidBody *)
                                    (btbroadphasepair.m_pProxy0->m_clientObject))->getUserPointer();

    OBJMESH *objmesh1 = (OBJMESH *)((btRigidBody *)
                                    (btbroadphasepair.m_pProxy1->m_clientObject))->getUserPointer();

    console_print("Object #0: %s\n", objmesh0->name);
    console_print("Object #1: %s\n", objmesh1->name);
    console_print("%d\n\n", get_milli_time());

    /* Let Bullet continue to deal with the collision by sending the
     * information to the default near-callback function maintained
     * internally.
     */
    // If we comment this out the Cube will pass through the Plane!  8-O
    btdispatcher.defaultNearCallback(btbroadphasepair,
                                     btdispatcher,
                                     btdispatcherinfo);
}

void program_bind_attrib_location(void *ptr) {
    PROGRAM *program = (PROGRAM *)ptr;

    glBindAttribLocation(program->pid, VA_Position, VA_Position_String);
    glBindAttribLocation(program->pid, VA_Normal,   VA_Normal_String);
}

void templateAppInit(int width, int height) {

    atexit(templateAppExit);

    GFX_start();

    init_physic_world();

    dispatcher->setNearCallback(near_callback);

    glViewport(0.0f, 0.0f, width, height);

    GFX_set_matrix_mode(PROJECTION_MATRIX);
    GFX_load_identity();
    GFX_set_perspective(45.0f,
                        (float)width / (float)height,
                        0.1f,
                        100.0f,
                        -90.0f);

    obj = new OBJ(OBJ_FILE, true);

    for (auto objmesh=obj->objmesh.begin();
         objmesh!=obj->objmesh.end(); ++objmesh) {

        objmesh->build();

        /* Test the current mesh name to verify it is the Cube.  If yes,
         * give it a rotation of 35 degrees on the XYZ axis; and then call the
         * add_rigid_body function using the mesh pointer and passing in a mass
         * of 1kg as a parameter.
         */
        if (!strcmp(objmesh->name, "Cube")) {
            objmesh->rotation[0] =
            objmesh->rotation[1] =
            objmesh->rotation[2] = 35.0f;

            add_rigid_body(&(*objmesh), 1.0f);
        } else {
            /* If it's not the Cube, it must be the plane.  Add it as a new
             * rigid body using the mass of 0 since you want it to be a static
             * object.
             */
            add_rigid_body(&(*objmesh), 0.0f);
        }

        objmesh->free_vertex_data();
    }

    program = new PROGRAM((char *)"default",
                          VERTEX_SHADER,
                          FRAGMENT_SHADER,
                          true,
                          false,
                          program_bind_attrib_location,
                          NULL);

    program->draw();
}

void templateAppDraw(void) {
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);


    GFX_set_matrix_mode(MODELVIEW_MATRIX);
    GFX_load_identity();
    {
        vec3    e(10.4f, -9.8f, 5.5f),
                c(-3.4f,  2.8f, 0.0f),
                u( 0.0f,  0.0f, 1.0f);

        GFX_look_at(&e, &c, &u);
    }

    for (auto objmesh=obj->objmesh.begin();
         objmesh!=obj->objmesh.end(); ++objmesh) {

        GFX_push_matrix();

        mat4 mat;
        objmesh->btrigidbody->getWorldTransform().getOpenGLMatrix((float *)&mat);
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

void templateAppExit(void) {
    free_physic_world();
    
    delete program;
    program = NULL;
    
    delete obj;
}
