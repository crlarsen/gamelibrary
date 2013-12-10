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

PROGRAM *path_point = NULL;

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

unsigned char double_tap = 0;
/* This navigation path query.  This structure will be used to construct the
 * query to be processed by Detour.
 */
NAVIGATIONPATH navigationpath_player;
/* The result of the query.  If this structure is successful, it will be
 * filled by the way points (along with other data) that form the path.
 */
NAVIGATIONPATHDATA navigationpathdata_player;

int viewport_matrix[4];

int player_next_point = -1;

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
    
    /* Query OpenGLES to return the current viewport matrix (which is
     * basically set with the glViewport command).
     */
    glGetIntegerv(GL_VIEWPORT, viewport_matrix);

    GFX_set_matrix_mode(PROJECTION_MATRIX);
    GFX_load_identity();

    GFX_set_perspective(80.0f,
                        (float)width / (float)height,
                        1.0f,
                        1000.0f,
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

void draw_navigation_points(NAVIGATIONPATHDATA *navigationpathdata, vec3 *color)
{
    /* Adjust the Z location of all points to make it easier to see them on
     * the floor of the maze.
     */
    for (int i=0; i!= navigationpathdata->path_point_count + 1; ++i)
        navigationpathdata->path_point_array[i].z = 1.0f;

    /* Reset the VAO and VBO indices since you are about to draw using
     * glDrawArrays.  The data is always dynamic, so there's no need to
     * create a VBO and VAO for this.
     */
    glBindVertexArrayOES(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    /* Enable alpha blending just to get fancy. */
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    /* If the path_point shader program does not exist, simply create it
     * when it's the first time the execution pointer reaches this location.
     */
    if (!path_point) {
        path_point = new PROGRAM((char *)"path_point",
                                 (char *)"point_vert.glsl",
                                 (char *)"point_frag.glsl",
                                 true,
                                 false,
                                 program_bind_attrib_location,
                                 NULL);
    }

    /* Set the shader program for drawing. */
    path_point->draw();

    glUniformMatrix4fv(path_point->get_uniform_location((char *)"MODELVIEWPROJECTIONMATRIX"),
                       1,
                       GL_FALSE,
                       (float *)GFX_get_modelview_projection_matrix());

    glUniform3fv(path_point->get_uniform_location((char *)"COLOR"),
                 1,
                 (float *)color);

    glEnableVertexAttribArray(0);

    glVertexAttribPointer(0,
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          0,
                          navigationpathdata->path_point_array);

    glDrawArrays(GL_POINTS,
                 0,
                 navigationpathdata->path_point_count + 1);

    glDrawArrays(GL_LINE_STRIP,
                 0,
                 navigationpathdata->path_point_count + 1);

    glDisable(GL_BLEND);
}

void move_entity(OBJMESH *objmesh,
                 NAVIGATIONPATHDATA *navigationpathdata,
                 int *next_point,
                 float speed) {
    /* Set the Z location of the mesh to be 0.  (You are only interested in
     * the X and Y position to compute the forward direction vector.)
     */
    objmesh->location.z =
    navigationpathdata->path_point_array[*next_point].z = 0.0f;
    /* Calculate the distance between the mesh location and the way point. */
    float distance =
        (objmesh->location-navigationpathdata->path_point_array[*next_point]).length();
    /* If the distance is less than 10cm, it means that the mesh is close
     * enough to its destination and the next index should be incremented to the
     * next way point.
     */
    if (distance < 0.1f) {
        ++*next_point;
        /* If the next_point is equal to the path point pointer +1 (the end
         * location), it means that the mesh has reached its destination.
         * Set the next_point back to -1 to stop the movement.
         */
        if (*next_point == (navigationpathdata->path_point_count + 1)) {
            *next_point = -1;
        }
    }

    /* If the next point is not -1, it means that the mesh is on the move. */
    if (*next_point != -1) {
        vec3 direction;
        /* Calculate the forward direction vector. */
        direction =
            navigationpathdata->path_point_array[*next_point] - objmesh->location;

        direction.safeNormalize();
        /* Assign the direction vector to the linear velocity of the rigid
         * body pointer attached to the mesh multiplied by the speed passed
         * in the parameter to this function.
         */
        objmesh->btrigidbody->setLinearVelocity(btVector3(direction.x * speed,
                                                          direction.y * speed,
                                                          0.0f));
        /* Activate the rigid body so that linear velocity will be affected. */
        objmesh->btrigidbody->setActivationState(ACTIVE_TAG);
    } else {
        /* The next_point value is -1, which means that the mesh should stop
         * moving.  To do this, simply specify that the rigid body wants
         * deactivation, and to keep things clean, reset the number of way
         * points.
         */
        objmesh->btrigidbody->setActivationState(WANTS_DEACTIVATION);
        navigationpathdata->path_point_count = 0;
    }
}

void templateAppDraw(void) {

    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    GFX_set_matrix_mode(MODELVIEW_MATRIX);
    GFX_load_identity();

    if (view_delta.x || view_delta.y) {
        if (view_delta.y) next_rotz -= view_delta.y;

        if (view_delta.x) {
            next_rotx -= view_delta.x;
            next_rotx = CLAMP(next_rotx, 0.0f, 90.0f);
        }

        view_delta.x =
        view_delta.y = 0.0f;
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
    
    if (double_tap) {
        /* Variable to hold the 3D location on the far plane of the frustum. */
        vec3 location;
        /* This function converts a 2D point from the screen coordinates
         * to a 3D point in space.  The return value is 1 or 0, depending on
         * whether or not the query is successful.  It's a GFX helper, but built
         * basically the same way as the standard gluUnproject
         * (http://www.opengl.org/sdk/docs/man/xhtml/gluUnproject.xml)
         * function.
         */
        if (GFX_unproject(view_location.x,
                          /* The origin of the OpenGLES color buffer is down
                           * left, but its location for iOS and Android is up
                           * left.  To handle this situation, simply use the
                           * viewport matrix height data (viewport_matrix[3])
                           * to readjust the Y location of the picking point
                           * onscreen.
                           */
                          viewport_matrix[3] - view_location.y,
                          /* This parameter represents the depth that you want
                           * to query, with 1 representing the far clipping
                           * plane and 0 representing the near clipping plane.
                           * In this case, you are only interested in the far
                           * clipping plane value, which explains the value 1.
                           */
                          1.0f,
                          GFX_get_modelview_matrix(),
                          GFX_get_projection_matrix(),
                          viewport_matrix,
                          &location.x,
                          &location.y,
                          &location.z)) {

            /* Now that you have the XYZ location on the far plane, you can
             * create the collision ray.  Begin by creating the starting point,
             * which is basically the current camera eye position.
             */
            btVector3 ray_from(eye.x,
                               eye.y,
                               eye.z),
            /* Translate the resulting location of GFX_unproject based on the
             * current eye location to make sure that the coordinate system
             * will fit with what the player currently sees onscreen.
             */
            ray_to(location.x + eye.x,
                   location.y + eye.y,
                   location.z + eye.z);
            /* Create the collision ray. */
            btCollisionWorld::ClosestRayResultCallback collision_ray(ray_from,
                                                                     ray_to);
            /* Launch the ray in space. */
            dynamicsworld->rayTest(ray_from,
                                   ray_to,
                                   collision_ray);
            /* Check if the collision ray gets a hit, and check if the
             * collision object involved is the maze btRigidBody.
             */
            if (collision_ray.hasHit() &&
                collision_ray.m_collisionObject == maze->btrigidbody) {

                collision_ray.m_hitNormalWorld.normalize();
                /* Check if the normal Z is pointing upward to make sure
                 * the hit is on the floor of the maze.
                 */
                if (collision_ray.m_hitNormalWorld.z() == 1.0f) {
                    /* Since you got a valid hit, it is time to create the
                     * pathfinding query to send to Detour.  First, assign
                     * the current player location as the starting point of
                     * the query.
                     */
                    navigationpath_player.start_location.x = player->location.x;
                    navigationpath_player.start_location.y = player->location.y;
                    navigationpath_player.start_location.z = player->location.z;
                    /* Then simply use the collision ray hit position XYZ as
                     * the end point of the path query.
                     */
                    navigationpath_player.end_location.x = collision_ray.m_hitPointWorld.x();
                    navigationpath_player.end_location.y = collision_ray.m_hitPointWorld.y();
                    navigationpath_player.end_location.z = collision_ray.m_hitPointWorld.z();
                    /* The query is ready to be sent to Detour, so send it over.
                     * If Detour was able to find a path, the function will
                     * return 1 and will store the way points information
                     * inside the navigationpathdata_player variable; othewise
                     * the function will return 0.
                     */
                    if (navigation->get_path(&navigationpath_player,
                                             &navigationpathdata_player)) {
                        player_next_point = 1;

                        /* Loop while you've got some way points.  Please note
                         * that by default, the function will assign the number
                         * of path_point_count to be the way points returned by
                         * Detour.  However, the function implementation added
                         * another point which is the exact same end location
                         * that you specified in your query.  The reason is
                         * that, most of the time, the ending point not exactly
                         * on the mavigation mesh, so the library will return
                         * the closts pont.  Depending on what you want to
                         * achieve, you may or may not want to use this extra
                         * way point.  But for this tutorial, you are going to
                         * take it into consideration.
                         */
                        for (int i=0; i!=navigationpathdata_player.path_point_count + 1; ++i)
                            console_print("%d: %f %f %f\n",
                                          i,
                                          navigationpathdata_player.path_point_array[i].x,
                                          navigationpathdata_player.path_point_array[i].y,
                                          navigationpathdata_player.path_point_array[i].z);
                        printf("\n");
                    }
                }
            }
        }

        double_tap = 0;
    }

    if (navigationpathdata_player.path_point_count) {
        vec3 color(0.0f, 0.0f, 1.0f);

        draw_navigation_points(&navigationpathdata_player, &color);

        move_entity(player,                     // The player OBJMESH pointer.
                    &navigationpathdata_player, // The player navigation path
                                                // structure.
                    &player_next_point,         // The way point the player is
                                                // moving towards.
                    3.0f);                      // The speed to use as factor
                                                // to the linear velocity of
                                                // the btRigidBody.
    }

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
    /* Check if the screen received a double-tap. */
    if (tap_count == 2) double_tap = 1;

    view_location.x = x;
    view_location.y = y;
}


void templateAppToucheMoved(float x, float y, unsigned int tap_count)
{
    view_delta.x = view_delta.x * 0.75f + (x - view_location.x) * 0.25f;
    view_delta.y = view_delta.y * 0.75f + (y - view_location.y) * 0.25f;

    view_location.x = x;
    view_location.y = y;
}


void templateAppExit(void) {
    if (path_point) {
        delete path_point;
        path_point = NULL;
    }
    delete navigation;

    free_physic_world();

    delete program;
    program = NULL;
    
    delete obj;
}
