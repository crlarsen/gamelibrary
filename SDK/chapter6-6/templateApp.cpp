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

#define OBJ_FILE (char *)"pinball.obj"

#define PHYSIC_FILE (char *)"pinball.bullet"

#define VERTEX_SHADER (char *)"vertex.glsl"

#define FRAGMENT_SHADER (char *)"fragment.glsl"

GFX *gfx = NULL;

OBJ *obj = NULL;

PROGRAM *program = NULL;

TEMPLATEAPP templateApp = {
    templateAppInit,
    templateAppDraw,
    templateAppToucheBegan
};

btSoftBodyRigidBodyCollisionConfiguration *collisionconfiguration = NULL;

btCollisionDispatcher *dispatcher = NULL;

btBroadphaseInterface *broadphase = NULL;

btConstraintSolver *solver = NULL;

btSoftRigidDynamicsWorld *dynamicsworld = NULL;

/* Represent the current ball index.  Since the ball objects are named
 * ball1, ball2, ball3, and so on, you will use this variable to implement
 * a mechanism that is similar to the one you used in the previous section
 * with Momo faces.
 */
unsigned char   ball_index = 0;
bool            restart_game = false;   // Flag used to restart the game when all
                                        // the balls are depleted.

/* Variable to hold the current ball mesh. */
OBJMESH *ball = NULL;

void get_next_ball(void)
{
    char tmp[MAX_CHAR] = {""};

    ++ball_index;

    sprintf(tmp, "ball%d", ball_index);

    /* Retrieve the mesh with this ball name. */
    ball = obj->get_mesh(tmp, false);
}

void near_callback(btBroadphasePair &btbroadphasepair,
                   btCollisionDispatcher  &btdispatcher,
                   const btDispatcherInfo &btdispatcherinfo)
{
    OBJMESH *objmesh0 = (OBJMESH *)((btRigidBody *)
                                    (btbroadphasepair.m_pProxy0->m_clientObject))->getUserPointer();

    OBJMESH *objmesh1 = (OBJMESH *)((btRigidBody *)
                                    (btbroadphasepair.m_pProxy1->m_clientObject))->getUserPointer();

    char tmp[MAX_CHAR] = {""};
    sprintf(tmp, "ball%d", ball_index);

    /* Check if the current collision involves the current ball and the
     * "out of bound" object.
     */
    if (ball &&

        (strstr(objmesh0->name, "out_of_bound") ||
         strstr(objmesh0->name, tmp                  ))

        &&

        (strstr(objmesh1->name, "out_of_bound") ||
         strstr(objmesh1->name, tmp                  ))) {
            get_next_ball();
            /* Restore the ball index back to normal, since get_next_ball
             * will increase it by 1.
             */
            --ball_index;
            /* If there is no ball pointer (ball == NULL), it means that the
             * player ran out of balls.  In this case, display the "game over"
             * object.
             */
            if (!ball) obj->get_mesh("game_over", false)->visible = true;

            ball = NULL;
        }

    btdispatcher.defaultNearCallback(btbroadphasepair,
                                     btdispatcher,
                                     btdispatcherinfo);
}


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

    MEMORY *memory = new MEMORY(PHYSIC_FILE, true);

    btbulletworldimporter->loadFileFromMemory((char *)memory->buffer, memory->size);

    delete memory;

    /* At this point all the rigid bodies and constraints have been re-created
     * and are now present inside your dynamic world.  All you have to do now
     * is loop through the rigid bodies and link their pointers to the appropriate
     * mesh.  To do this start off by creating a new counter.
     */
    for (int i=0; i!=btbulletworldimporter->getNumRigidBodies(); ++i) {
        /* Get a OBJMESH pointer based on the name of the current rigid body.
         * Since you have exported your .obj and .bullet files from Blender
         * (presumably) each mesh and rigid body has the same name.
         */
        OBJMESH *objmesh = obj->get_mesh(btbulletworldimporter->getNameForPointer(btbulletworldimporter->getRigidBodyByIndex(i)), false);

        /* If you get a valid pointer it means that you have a match. */
        if (objmesh) {
            /* Link the btRigidBody pointer to the mesh. */
            objmesh->btrigidbody = (btRigidBody *)btbulletworldimporter->getRigidBodyByIndex(i);
            /* Set the user pointer so you can et back access to the OBJMESH
             * structure within any Bullet function callback (in this case the
             * near callback).
             */
            objmesh->btrigidbody->setUserPointer(objmesh);
            /* Teak the restitution of the current rigid body.  Since you are
             * working on a pinball machine, everything should be very bouncy.
             */
            objmesh->btrigidbody->setRestitution(0.75f);
        }
    }

    /* At this point everything has been loaded and linked.  Get rid of the
     * world importer in the memory.
     */
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


void load_game(void)
{
    obj = new OBJ(OBJ_FILE, true);

    for (auto objmesh=obj->objmesh.begin();
         objmesh!=obj->objmesh.end(); ++objmesh) {
        objmesh->build();

        objmesh->free_vertex_data();
    }


    init_physic_world();

    dispatcher->setNearCallback(near_callback);

    load_physic_world();

    obj->get_mesh("game_over", false)->visible = false;

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



void templateAppInit(int width, int height) {
    atexit(templateAppExit);

    gfx = new GFX;

    glViewport(0.0f, 0.0f, width, height);

    gfx->set_matrix_mode(PROJECTION_MATRIX);

    gfx->load_identity();
    // Adjust "Field of View Y" angle for devices which has an aspect
    // ratio which is wider than the origin iPhone (3:2).  Devices which
    // have a narrower aspect ratio (such as iPad) work fine, as is.
    const float iPhoneOriginalWidth =320.0f;
    const float iPhoneOriginalHeight=480.0f;
    const float originalFovy=45.0f;
    float fovy(originalFovy);
    if (height*iPhoneOriginalWidth > width*iPhoneOriginalHeight) {
        float   h = (iPhoneOriginalHeight*0.5f) / tanf(originalFovy*0.5f*DEG_TO_RAD);
        fovy = 2.0f * atan2f(((float)height)*0.5, h) * RAD_TO_DEG;
    }
    gfx->set_perspective(fovy,
                         (float)width / (float)height,
                           1.0f,
                         100.0f,
                           0.0f);

    load_game();
}


void templateAppDraw(void) {
    /* If you need to restart the game ... */
    if (restart_game) {
        /* 1. Free everything from memory. */
        templateAppExit();
        /* 2. Reload the game. */
        load_game();
        /* 3. Reset the ball index. */
        ball_index = 0;
        /* 4. Reset the restart flag. */
        restart_game = false;
    }

    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);


    gfx->set_matrix_mode(MODELVIEW_MATRIX);
    gfx->load_identity(); {

        vec3    e(0.0f, -21.36f, 19.64f),
                c(0.0f, -20.36f, 19.22f),
                u(0.0f,    0.0f,  1.0f );

        gfx->look_at(e, c, u);
    }

    for (auto objmesh=obj->objmesh.begin();
         objmesh!=obj->objmesh.end(); ++objmesh) {

        gfx->push_matrix();

        if (objmesh->btrigidbody) {
            mat4 mat;

            objmesh->btrigidbody->getWorldTransform().getOpenGLMatrix(mat.m());

            gfx->multiply_matrix(mat);
        } else {
            gfx->translate(objmesh->location);
        }

        glUniformMatrix4fv(program->uniform_map["MODELVIEWPROJECTIONMATRIX"].location,
                           1,
                           GL_FALSE,
                           gfx->get_modelview_projection_matrix().m());

        objmesh->draw();

        gfx->pop_matrix();
    }

    dynamicsworld->stepSimulation(1.0f / 60.0f);
}

void templateAppToucheBegan(float x, float y, unsigned int tap_count)
{
    /* Check if the "game over" is visible.  If yes, toggle the
     * restart flag and exit the callback.
     */
    if (obj->get_mesh("game_over", false)->visible) {
        restart_game = true;
        return;
    }

    if (!ball) {
        get_next_ball();

        if (ball) {
            /* Activate the rigid body */
            ball->btrigidbody->setActivationState(ACTIVE_TAG);
            /* Give it a boost on the Y axis to launch the ball */
            ball->btrigidbody->setLinearVelocity(btVector3(0.0f, 30.0f, 0.0f));
        }
    } else {
        OBJMESH *objmesh = obj->get_mesh("flipper1", false);
        /* Activate the rigid body.  Since you are about to assign a new
         * angular velocity to it, it has to be alive first.
         */
        objmesh->btrigidbody->setActivationState(ACTIVE_TAG);
        /* Set a negative angular velocity (since the first flipper is
         * located on the right side and will respond to a negative
         * rotation on the Z axis.
         */
        objmesh->btrigidbody->setAngularVelocity(btVector3(0.0f, 0.0f, -30.0f));
        /* Do the same as above, but this time on the left flipper, assigning
         * a positive angular velocity on the Z axis.
         */
        objmesh = obj->get_mesh("flipper2", false);

        objmesh->btrigidbody->setActivationState(ACTIVE_TAG);
        
        objmesh->btrigidbody->setAngularVelocity(btVector3(0.0f, 0.0f, 30.0f));
    }
}

void templateAppExit(void) {
    
    free_physic_world();
    
    delete program;
    program = NULL;
    
    delete obj;
}
