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

#define PHYSIC_FILE (char *)"Scene.bullet"

#define VERTEX_SHADER (char *)"vertex.glsl"

#define FRAGMENT_SHADER (char *)"fragment.glsl"

OBJ *obj = NULL;

PROGRAM *program = NULL;


float   rotz = 0.0f,
        rotx = 90.0f;

float screen_size = 0.0f;

vec2    view_location,
        view_delta;
	 
vec3    move_location(0.0f, 0.0f, 0.0f),
        move_delta;

OBJMESH *camera = NULL;

TEMPLATEAPP templateApp = {
    templateAppInit,
    templateAppDraw,
    templateAppToucheBegan,
    templateAppToucheMoved,
    templateAppToucheEnded
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
        OBJMESH *objmesh = obj->get_mesh(btbulletworldimporter->getNameForPointer(btbulletworldimporter->getRigidBodyByIndex(i)), false);

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
    screen_size = (width > height) ? width : height;

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
    
    /* Query the camera mesh pointer. */
    camera = obj->get_mesh("camera", false);
    /* Set the rigid body to be a dynamic body. */
    camera->btrigidbody->setAngularFactor(0.0f);
    /* Make the object invisible at render time. */
    camera->visible = true;

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


    if (view_delta->x || view_delta->y) {

        if (view_delta->y) rotz -= view_delta->y;

        if (view_delta->x) {
            rotx += view_delta->x;
            rotx = CLAMP(rotx, 0.0f, 180.0f);
        }
        
        view_delta->x =
        view_delta->y = 0.0f;
    }
    
    
    if (move_delta.z) {
        vec3 forward;

        float   r = rotz * DEG_TO_RAD,
                c = cosf(r),
                s = sinf(r);

        forward.x = c * move_delta.y - s * move_delta.x;
        forward.y = s * move_delta.y + c * move_delta.x;
        /* Assign the linear velocity of the collision object and multiply
         * the delta by 6.7m/s (the average velocity that human's achieve while
         * running.
         */
        camera->btrigidbody->setLinearVelocity(btVector3(forward.x * move_delta.z * 6.7f,
                                                         forward.y * move_delta.z * 6.7f,
                                                         0.0f));
        /* Make sure that the rigid body is activated; otherwise the
         * setLinearVelocity call above will have no effect, because the body
         * might be deactivated.
         */
        camera->btrigidbody->setActivationState(ACTIVE_TAG);
    } else {
        camera->btrigidbody->setActivationState(ISLAND_SLEEPING);
    }

    GFX_rotate(-rotx, 1.0f, 0.0f, 0.0f);

    GFX_rotate(-rotz, 0.0f, 0.0f, 1.0f);

    GFX_translate(-camera->location.x,
                  -camera->location.y,
                  /* Give an offset on the Z axis since the location represents
                   * the position of the object pivot point.  You need to
                   * simulate a real human eye looking at the scene, so add to
                   * the current Z value half of the Z dimension of the object
                   * bounding box, to simulate that the eye position is located
                   * at the top of the collision object.
                   */
                  -camera->location.z - (camera->dimension.z * 0.5f));

    for (auto objmesh=obj->objmesh.begin();
         objmesh!=obj->objmesh.end(); ++objmesh) {

        GFX_push_matrix();

        mat4 mat;
        /* Ask Bullet to return the OpenGL matrix for the current mesh and store
         * it inside a 4x4 matrix.
         */
        objmesh->btrigidbody->getWorldTransform().getOpenGLMatrix((float *)&mat);
        /* Update the mesh location by copying the last row of the matrix,
         * to make sure the latest location will be used by the clipping
         * method (if any).
         */
        objmesh->location.x = mat.m[3].x;
        objmesh->location.y = mat.m[3].y;
        objmesh->location.z = mat.m[3].z;
        /* Multiply the matrix by the current modelview matrix. */
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
        view_location->x = x;
        view_location->y = y;
    }
}


void templateAppToucheMoved(float x, float y, unsigned int tap_count)
{
    if (y > ((screen_size * 0.5f) - (screen_size * 0.05f)) &&
        y < ((screen_size * 0.5f) + (screen_size * 0.05f))) {
        move_delta.z =
        view_delta->x =
        view_delta->y = 0.0f;

        move_location.x = x;
        move_location.y = y;

        view_location->x = x;
        view_location->y = y;
    } else if (y < (screen_size * 0.5f)) {
        vec3 touche(x, y, 0.0f);

        move_delta = touche - move_location;

        move_delta.safeNormalize();

        move_delta.z = CLAMP((move_location-touche).length() / 128.0f,
                             0.0f,
                             1.0f);
    } else {
        view_delta->x = view_delta->x * 0.75f + (x - view_location->x) * 0.25f;
        view_delta->y = view_delta->y * 0.75f + (y - view_location->y) * 0.25f;

        view_location->x = x;
        view_location->y = y;
    }
}


void templateAppToucheEnded(float x, float y, unsigned int tap_count)
{
    move_delta.z = 0.0f;
}


void templateAppExit(void) {
    delete program;
    program = NULL;
    
    delete obj;
}
