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
 * - FONT
 * - MEMORY
 * - NAVIGATION
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

#define OBJ_FILE (char *)"maze.obj"

#define PHYSIC_FILE (char *)"maze.bullet"

#define VERTEX_SHADER (char *)"vertex.glsl"

#define FRAGMENT_SHADER (char *)"fragment.glsl"


OBJ *obj = NULL;

PROGRAM *program = NULL;

vec2    view_location,
        view_delta(0.0f, 0.0f);

vec3    eye,
        next_eye,
        center(0.0f, 0.0f, 0.0f),
        up(0.0f, 0.0f, 1.0f);


float rotx	= 45.0f,
      next_rotx = 0.0f, 
      rotz	= 0.0f,
      next_rotz	= -45.0f,
      distance	= 30.0f;	

/* The player object (the blue arrow). */
OBJMESH *player = NULL;
/* The maze itself. */
OBJMESH *maze = NULL;
/* This is an easy-to-use structure that wraps up the Recast and Detour
 * low-level API into one object that can create a navigation mesh and that
 * you can use to query way points.  The code is located inside the
 * navigation.cpp/.h file, which inside the common/ directory of the SDK.
 * The implementation is a bit too big to cover in detail in this chapter, so
 * the focus will just be on the code implementation and the usage of this
 * structure.
 */
NAVIGATION *navigation = NULL;

TEMPLATEAPP templateApp = {
    templateAppInit,
    templateAppDraw,
    templateAppToucheBegan,
    templateAppToucheMoved
};


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

    MEMORY *memory = new MEMORY(PHYSIC_FILE, true);

    btbulletworldimporter->loadFileFromMemory((char *)memory->buffer, memory->size);

    delete memory;

    for (int i=0; i!=btbulletworldimporter->getNumRigidBodies(); ++i) {
        OBJMESH *objmesh = obj->get_mesh(btbulletworldimporter->getNameForPointer(btbulletworldimporter->getRigidBodyByIndex(i)),
                                         false);

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


void templateAppInit(int width, int height) {

    atexit(templateAppExit);

    GFX_start();

    glViewport(0.0f, 0.0f, width, height);

    GFX_set_matrix_mode(PROJECTION_MATRIX);
    GFX_load_identity();


    GFX_set_perspective(80.0f,
                        (float)width / (float)height,
                        1.0f,
                        100.0f,
                        -90.0f);


    obj = new OBJ(OBJ_FILE, true);

    for (int i=0; i!=obj->objmesh.size(); ++i) {
        /* If the current mesh is the maze... */
        if (strstr(obj->objmesh[i].name, "maze")) {
            /* Initialize the NAVIGATION structure. */
            navigation = new NAVIGATION((char *)"maze");
            /* Set up the height of the player, which is basically the same
             * as the Z dimension of the player.
             */
            navigation->navigationconfiguration.agent_height = 2.0f;
            /* Set up the radius of the player (the X dimension of the player
             * divided by 2).  The configuration parameters are really important,
             * because the navigation mesh will be built according to these
             * settings.  Mor tweaks can be made by accessing the navigation->
             * navigationconfiguration parameters to fit your needs.
             */
            navigation->navigationconfiguration.agent_radius = 0.4f;
            /* Build the navigation mesh.  The function will return 1 if
             * successful or 0 if not.  If the generation fails, it might be
             * because the scene is too small for the agent, or if there are no
             * triangles that can be used.  Always make sure that you call this
             * function before building or optimizing the mesh.
             */
            if (navigation->build(&obj->objmesh[i])) {
                console_print("Navigation generated.\n");
            } else {
                console_print("Unable to create the navigation mesh.");
            }
        }

        obj->objmesh[i].optimize(128);

        /* OBJMESH::build2 is another version of the OBJMESH::build that
         * does not use VAO (only pure glDraw calls), at the time of
         * writing this book mixing direct rendering using glDraw
         * commands (as you will do in this chapter) with VAO cause
         * issues on some Android drivers.
         */
        obj->objmesh[i].build2();

        obj->objmesh[i].free_vertex_data();
    }
    
    
    init_physic_world();

    load_physic_world();
    
    /* Get the player mesh pointer. */
    player = obj->get_mesh("player", false);
    /* Set the player to be a dynamic rigid body. */
    player->btrigidbody->setAngularFactor(0.0f);
    /* Get the maze object. */
    maze = obj->get_mesh("maze", false);
    /* Adjust the camera distance so it can frame the maze. */
    distance = maze->radius * 2.0f;

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
}


void templateAppDraw(void) {

    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);


    GFX_set_matrix_mode(MODELVIEW_MATRIX);
    GFX_load_identity();


    if (view_delta->x || view_delta->y) {

        if (view_delta->y) next_rotz -= view_delta->y;

        if (view_delta->x) {
            next_rotx -= view_delta->x;
            next_rotx = CLAMP(next_rotx, 0.0f, 90.0f);
        }

        view_delta->x =
        view_delta->y = 0.0f;
    }

    rotx = rotx * 0.9f + next_rotx * 0.1f;
    rotz = rotz * 0.9f + next_rotz * 0.1f;



    eye.x = center.x +
            distance *
            cosf(rotx * DEG_TO_RAD) *
            sinf(rotz * DEG_TO_RAD);

    eye.y = center.y -
            distance *
            cosf(rotx * DEG_TO_RAD) *
            cosf(rotz * DEG_TO_RAD);


    eye.z = center.z +
            distance *
            sinf(rotx * DEG_TO_RAD);
    
    
    rotx = rotx * 0.9f + next_rotx * 0.1f;
    rotz = rotz * 0.9f + next_rotz * 0.1f;
    
    center.x = maze->location.x;
    center.y = maze->location.y;
    center.z = maze->location.z;

    GFX_look_at(&eye,
                &center,
                &up);


    program->draw();

    glUniform1i(program->get_uniform_location(TM_Diffuse_String), TM_Diffuse);

    for (auto objmesh=obj->objmesh.begin();
         objmesh!=obj->objmesh.end(); ++objmesh) {

        GFX_push_matrix();

        mat4 mat;

        objmesh->btrigidbody->getWorldTransform().getOpenGLMatrix((float *)&mat);

        objmesh->location.x = mat.m[3].x;
        objmesh->location.y = mat.m[3].y;
        objmesh->location.z = mat.m[3].z;
        GFX_multiply_matrix(&mat);

        glUniformMatrix4fv(program->get_uniform_location((char *)"MODELVIEWPROJECTIONMATRIX"),
                           1,
                           GL_FALSE,
                           (float *)GFX_get_modelview_projection_matrix());

        objmesh->draw();
        
        GFX_pop_matrix();
    }
    
    navigation->draw();
    
    dynamicsworld->stepSimulation(1.0f / 60.0f);
}


void templateAppToucheBegan(float x, float y, unsigned int tap_count)
{
	view_location->x = x;
	view_location->y = y;
}


void templateAppToucheMoved(float x, float y, unsigned int tap_count)
{
	view_delta->x = view_delta->x * 0.75f + (x - view_location->x) * 0.25f;
	view_delta->y = view_delta->y * 0.75f + (y - view_location->y) * 0.25f;

	view_location->x = x;
	view_location->y = y;
}


void templateAppExit(void) {
    delete navigation;

    free_physic_world();

    delete program;
    program = NULL;

    delete obj;
}
