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
 * - SOUND
 * - TEXTURE
 * - THREAD
 */

#include "templateApp.h"

#define OBJ_FILE (char *)"Scene.obj"

#define PHYSIC_FILE (char *)"Scene.bullet"


OBJ *obj = NULL;

vec3    eye,
        next_eye,
        up(0.0f, 0.0f, 1.0f);

OBJMESH *player = NULL;

THREAD *thread = NULL;

vec4 frustum[6];

int viewport_matrix[4];

FONT    *font_small = NULL,
        *font_big   = NULL;

float   rotx	 = 40.0f,
        rotz	 = 0.0f,
        distance = 5.0f;

vec2 accelerometer(0.0f, 0.0f),
     next_accelerometer(0.0f, 0.0f);

float ball_speed  = 6.7f,
      sensitivity = 3.0f;
	  
unsigned char game_state = 0;


float game_time  = 0.0f,
      gem_factor = 20.0f;

unsigned int gem_points = 0;

/* The background music. */
SOUNDBUFFER *background_soundbuffer = NULL;
SOUND *background_sound = NULL;
/* To use with the four gem colors:  red, green, blue, and yellow, in this
 * specific order.
 */
SOUNDBUFFER *gems_soundbuffer[4];
SOUND *gems_sound[4];
/* To handle the water sound under the first bridge. */
SOUNDBUFFER *water_soundbuffer = NULL;
SOUND *water_sound = NULL;
/* To handle the lava sound under the second bridge. */
SOUNDBUFFER *lava_soundbuffer = NULL;
SOUND *lava_sound = NULL;
/* To handle the toxic sound under the last bridge. */
SOUNDBUFFER *toxic_soundbuffer = NULL;
SOUND *toxic_sound = NULL;


TEMPLATEAPP templateApp = {
    templateAppInit,
    templateAppDraw,
    templateAppToucheBegan,
    NULL,
    NULL,
    templateAppAccelerometer
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
    while(dynamicsworld->getNumCollisionObjects()) {
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


void program_draw(void *ptr)
{
    PROGRAM *program = (PROGRAM *)ptr;

    for (auto it=program->uniform_map.begin(); it!=program->uniform_map.end(); ++it){
        auto    &name = it->first;
        auto    &uniform = it->second;

        if(!uniform.constant && (name == TM_Diffuse_String)) {
            glUniform1i(uniform.location, TM_Diffuse);

            uniform.constant = true;
        } else if(name == "MODELVIEWPROJECTIONMATRIX") {
            glUniformMatrix4fv(uniform.location,
                               1,
                               GL_FALSE,
                               (float *)GFX_get_modelview_projection_matrix());
        } else if(name == "TEXTUREMATRIX") {
            static vec2 scroll(0.0f, 0.0f);

            GFX_set_matrix_mode(TEXTURE_MATRIX);

            GFX_push_matrix();

            scroll->x += 0.0025f;
            scroll->y += 0.0025f;

            GFX_translate(scroll->x, scroll->y, 0.0f);

            glUniformMatrix4fv(uniform.location,
                               1,
                               GL_FALSE,
                               (float *)GFX_get_texture_matrix());
            
            GFX_pop_matrix();
            
            GFX_set_matrix_mode(MODELVIEW_MATRIX); 
        }		
    }
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
		if (localray.m_collisionObject == m_btRigidBody) {
            return 1.0f;
        }

		return ClosestRayResultCallback::addSingleResult(localray, normalinworldspace);
	}

protected:
    btRigidBody *m_btRigidBody;
};


bool contact_added_callback(btManifoldPoint &btmanifoldpoint,
                            const btCollisionObject *btcollisionobject0,
                            int part_0, int index_0,
                            const btCollisionObject *btcollisionobject1,
                            int part_1, int index_1) {

    OBJMESH *objmesh0 = (OBJMESH *)((btRigidBody *)btcollisionobject0)->getUserPointer();

    OBJMESH *objmesh1 = (OBJMESH *)((btRigidBody *)btcollisionobject1)->getUserPointer();


    if ((strstr(objmesh0->name, "level_clear") ||
         strstr(objmesh1->name, "level_clear"))) {
        game_state = 1;
    } else if ((strstr(objmesh0->name, "player") ||
                strstr(objmesh1->name, "player"))
               &&
               (strstr(objmesh0->name, "gem") ||
                strstr(objmesh1->name, "gem"))) {

        OBJMESH *objmesh = NULL;

        btCollisionObject *btcollisionobject = NULL;

        if (strstr(objmesh0->name, "gem")) {

            objmesh = objmesh0;
            btcollisionobject = (btCollisionObject *)btcollisionobject0;
        } else {

            objmesh = objmesh1;
            btcollisionobject = (btCollisionObject *)btcollisionobject1;
        }
                   
        unsigned char index = 0;

        if (strstr(objmesh->name, "red")) {

            gem_points += 1;
            index = 0;
        } else if (strstr(objmesh->name, "green")) {

            gem_points += 2;
            index = 1;
        } else if (strstr(objmesh->name, "blue")) {

            gem_points += 3;
            index = 2;
        } else if (strstr(objmesh->name, "yellow")) {

            gem_points += 4;
            index = 3;
        }

        gems_sound[index]->set_location(objmesh->location,
                                        objmesh->radius * gem_factor);

        gems_sound[index]->play(0);

        objmesh->visible = false;

        delete objmesh->btrigidbody->getCollisionShape();

        delete objmesh->btrigidbody->getMotionState();

        dynamicsworld->removeRigidBody(objmesh->btrigidbody);

        dynamicsworld->removeCollisionObject(btcollisionobject);
                 
        delete objmesh->btrigidbody;
                 
        objmesh->btrigidbody = NULL;
    }
    
    return false;
}


void decompress_stream(void *ptr)
{
    background_sound->update_queue();
}


void load_level(void)
{
    obj = new OBJ(OBJ_FILE, true);

    for (auto objmesh=obj->objmesh.begin();
         objmesh!=obj->objmesh.end(); ++objmesh) {
        objmesh->optimize(128);

        objmesh->build();

        objmesh->free_vertex_data();
    }

    init_physic_world();

    load_physic_world();
    
    gContactAddedCallback = contact_added_callback;
    
    
    OBJMESH *level_clear = obj->get_mesh("level_clear", false);

    level_clear->btrigidbody->setCollisionFlags(level_clear->btrigidbody->getCollisionFlags()  |
                                                btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK |
                                                btCollisionObject::CF_NO_CONTACT_RESPONSE);
    level_clear->visible = false;

    for (auto objmesh=obj->objmesh.begin();
         objmesh!=obj->objmesh.end(); ++objmesh) {

        if (strstr(objmesh->name, "gem")) {
            objmesh->rotation->z = (float)(random() % 360);

            objmesh->btrigidbody->setCollisionFlags(objmesh->btrigidbody->getCollisionFlags() |
                                                    btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
        }
    }

    player = obj->get_mesh("player", false);

    player->btrigidbody->setFriction(10.0f);

    eye = player->location;
    
    for (auto texture=obj->texture.begin();
         texture!=obj->texture.end(); ++texture) {
        (*texture)->build(obj->texture_path,
                          TEXTURE_MIPMAP | TEXTURE_16_BITS,
                          TEXTURE_FILTER_2X,
                          0.0f);
    }

    for (auto program=obj->program.begin();
         program != obj->program.end(); ++program) {
        (*program)->build(program_bind_attrib_location,
                          program_draw,
                          true,
                          obj->program_path);
    }

    for (auto objmaterial=obj->objmaterial.begin();
         objmaterial != obj->objmaterial.end(); ++objmaterial) {
        objmaterial->build(NULL);
    }

    font_small = new FONT((char *)"foo.ttf");

    font_small->load(font_small->name,
                     true,
                     24.0f,
                     512,
                     512,
                     32,
                     96);


    font_big = new FONT((char *)"foo.ttf");

    font_big->load(font_big->name,
                   true,
                   48.0f,
                   512,
                   512,
                   32,
                   96);

    /* Declare a memory structure that you will use (and reuse) to load
     * each sound buffer.
     */
    MEMORY *memory = NULL;

    for (int i=0; i!=4; ++i) {

        switch(i) {

            case 0: {

                memory = new MEMORY((char *)"red.ogg", true);
                break;
            }
            case 1: {

                memory = new MEMORY((char *)"green.ogg", true);
                break;
            }
            case 2: {

                memory = new MEMORY((char *)"blue.ogg", true);
                break;
            }
            case 3: {

                memory = new MEMORY((char *)"yellow.ogg", true);
                break;
            }
        }
        /* For the current gem buffer index, create a sound buffer using the
         * contents of the memory structure that you have loaded.
         */
        gems_soundbuffer[i] = new SOUNDBUFFER((char *)"gem", memory);

        delete memory;
        /* Create a new sound source for the current index and link the
         * current sound buffer.
         */
        gems_sound[i] =  new SOUND((char *)"gem", gems_soundbuffer[i]);
        /* Set the volume for the source but do not start playing it yet.
         * You will handle the playback code inside the contact_added_callback
         * only when the player collides with a gem.
         */
        gems_sound[i]->set_volume(1.0f);
    }
    
    
    OBJMESH *objmesh = NULL;

    memory = new MEMORY((char *)"water.ogg", true);

    water_soundbuffer = new SOUNDBUFFER((char *)"water", memory);

    delete memory;

    water_sound = new SOUND((char *)"water", water_soundbuffer);

    objmesh = obj->get_mesh("water", false);

    water_sound->set_location(objmesh->location,
                              objmesh->radius);

    water_sound->set_volume(0.5f);

    water_sound->play(1);


    memory = new MEMORY((char *)"lava.ogg", true);

    lava_soundbuffer = new SOUNDBUFFER((char *)"lava", memory);

    delete memory;

    lava_sound = new SOUND((char *)"lava", lava_soundbuffer);

    objmesh = obj->get_mesh("lava", false);

    lava_sound->set_location(objmesh->location,
                             objmesh->radius);

    lava_sound->set_volume(0.5f);

    lava_sound->play(1);


    memory = new MEMORY((char *)"toxic.ogg", true);

    toxic_soundbuffer = new SOUNDBUFFER((char *)"toxic", memory);

    delete memory;

    toxic_sound = new SOUND((char *)"toxic", toxic_soundbuffer);

    objmesh = obj->get_mesh("toxic", false);
    
    toxic_sound->set_location(objmesh->location,
                              objmesh->radius);

    toxic_sound->set_volume(0.5f);

    toxic_sound->play(1);


    memory = new MEMORY((char *)"background.ogg", true);

    background_soundbuffer = new SOUNDBUFFERSTREAM((char *)"background", memory);

    background_sound = new SOUND((char *)"background", background_soundbuffer);

    background_sound->set_volume(0.5f);

    background_sound->play(1);

    thread->play();
}


void free_level(void)
{
    gem_points =
    game_state = 0;
    game_time  = 0.0f;

    thread->pause();

    delete background_sound;
    background_sound = NULL;

    delete background_soundbuffer->memory;
    background_soundbuffer->memory = NULL;

    delete background_soundbuffer;
    background_soundbuffer = NULL;

    for (int i=0; i!=4; ++i) {
        delete gems_sound[i];
        gems_sound[i] = NULL;
        delete gems_soundbuffer[i];
        gems_soundbuffer[i] = NULL;
    }

    delete water_sound;
    water_sound = NULL;
    delete water_soundbuffer;
    water_soundbuffer = NULL;

    delete lava_sound;
    lava_sound = NULL;
    delete lava_soundbuffer;
    lava_soundbuffer = NULL;

    delete toxic_sound;
    toxic_sound = NULL;
    delete toxic_soundbuffer;
    toxic_soundbuffer = NULL;

    player = NULL;

    thread->pause();
    
    delete font_small;
    font_small = NULL;

    delete font_big;
    font_big = NULL;

    free_physic_world();
    
    delete obj;
}


void templateAppInit(int width, int height) {

    atexit(templateAppExit);

    GFX_start();

    AUDIO_start();

    thread = new THREAD(decompress_stream, NULL, THREAD_PRIORITY_NORMAL, 1);

    glViewport(0.0f, 0.0f, width, height);

    glGetIntegerv(GL_VIEWPORT, viewport_matrix);

    srandom(get_milli_time());
    
    load_level();
}


void templateAppDraw(void) {

    if (game_state == 2) {
        free_level();
        load_level();
    }

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    GFX_set_matrix_mode(PROJECTION_MATRIX);
    GFX_load_identity();

    GFX_set_perspective(80.0f,
                        (float)viewport_matrix[2] / (float)viewport_matrix[3],
                        0.1f,
                        50.0f,
                        -90.0f);

    GFX_set_matrix_mode(MODELVIEW_MATRIX);
    GFX_load_identity();


    next_accelerometer->x = accelerometer->x * 0.1f + next_accelerometer->x * 0.9f;
    next_accelerometer->y = accelerometer->y * 0.1f + next_accelerometer->y * 0.9f;
    
    rotz += next_accelerometer->y * sensitivity;
    
    vec3    forward(0.0f, 1.0f, 0.0f),
            direction;

    if (!game_state) {
        float   r = rotz * DEG_TO_RAD,
                c = cosf(r),
                s = sinf(r);

        direction->x = c * forward->y - s * forward->x;
        direction->y = s * forward->y + c * forward->x;

        float speed = CLAMP((-next_accelerometer->x * sensitivity) * ball_speed,
                            -ball_speed,
                            ball_speed);

        player->btrigidbody->setAngularVelocity(btVector3(direction->x * speed,
                                                          direction->y * speed,
                                                          0.0f));
        
        player->btrigidbody->setActivationState(ACTIVE_TAG);
    }

    next_eye->x = player->location->x +
                  distance *
                  cosf(rotx * DEG_TO_RAD) *
                  sinf(rotz * DEG_TO_RAD);

    next_eye->y = player->location->y -
                  distance *
                  cosf(rotx * DEG_TO_RAD) *
                  cosf(rotz * DEG_TO_RAD);

    next_eye->z = player->location->z +
                  distance *
                  sinf(rotx * DEG_TO_RAD);

    player->location->z += player->dimension->z;

    btVector3 p1(player->location->x,
                 player->location->y,
                 player->location->z),

    p2(next_eye->x,
       next_eye->y,
       next_eye->z);

    ClosestNotMeRayResultCallback back_ray(player->btrigidbody,
                                           p1,
                                           p2);

    dynamicsworld->rayTest(p1,
                           p2,
                           back_ray);

    if (back_ray.hasHit()) {
        back_ray.m_hitNormalWorld.normalize();

        next_eye->x = back_ray.m_hitPointWorld.x() +
        (back_ray.m_hitNormalWorld.x() * 0.1f);

        next_eye->y = back_ray.m_hitPointWorld.y() +
        (back_ray.m_hitNormalWorld.y()* 0.1f);

        next_eye->z = back_ray.m_hitPointWorld.z() +
        (back_ray.m_hitNormalWorld.z()* 0.1f);
    }
    
    eye = next_eye * 0.05f + eye * 0.95f;

    direction = player->location - eye;

    direction.safeNormalize();

    AUDIO_set_listener(eye, direction, up);

    GFX_look_at(&eye,
                &player->location,
                &up);

    build_frustum(frustum,
                  *GFX_get_modelview_matrix(),
                  *GFX_get_projection_matrix());
    
    
    for (auto objmesh=obj->objmesh.begin();
         objmesh!=obj->objmesh.end(); ++objmesh) {

        objmesh->distance = sphere_distance_in_frustum(frustum,
                                                       &objmesh->location,
                                                       objmesh->radius);

        if (objmesh->distance && objmesh->visible) {
            GFX_push_matrix();

            if (strstr(objmesh->name, "gem")) {
                GFX_translate(objmesh->location->x,
                              objmesh->location->y,
                              objmesh->location->z);

                objmesh->rotation->z += 1.0f;

                GFX_rotate(objmesh->rotation->z, 0.0f, 0.0f, 1.0f);
            } else if (objmesh->btrigidbody) {
                mat4 mat;

                objmesh->btrigidbody->getWorldTransform().getOpenGLMatrix((float *)&mat);

                objmesh->location = vec3(mat[3], true);

                GFX_multiply_matrix(&mat);
            } else {
                GFX_translate(objmesh->location->x,
                              objmesh->location->y,
                              objmesh->location->z);
            }
            
            objmesh->draw();
            
            GFX_pop_matrix();
        }
    }
    
    dynamicsworld->stepSimulation(1.0f / 60.0f);
    
    
    GFX_set_matrix_mode(PROJECTION_MATRIX);
    GFX_load_identity();
    
    float   half_width  = (float)viewport_matrix[2] * 0.5f,
            half_height = (float)viewport_matrix[3] * 0.5f;

    GFX_set_orthographic_2d(-half_width,
                            half_width,
                            -half_height,
                            half_height);

    GFX_rotate(-90.0f, 0.0f, 0.0f, 1.0f);

    GFX_translate(-half_height, -half_width, 0.0f);

    GFX_set_matrix_mode(MODELVIEW_MATRIX);

    GFX_load_identity();

    vec4 font_color(0.0f, 0.0f, 0.0f, 1.0f);
    
    char    gem_str  [MAX_CHAR] = {""},
            time_str [MAX_CHAR] = {""},
            level_str[MAX_CHAR] = {""};

    if (game_state) {
        sprintf(level_str, "Level Clear!");

        font_big->print(viewport_matrix[3] * 0.5f - font_big->length(level_str) * 0.5f + 4.0f,
                        viewport_matrix[2] - font_big->font_size * 1.5f - 4.0f,
                        level_str,
                        &font_color);

        /* Yellow. */
        font_color->x = 1.0f;
        font_color->y = 1.0f;
        font_color->z = 0.0f;

        font_big->print(viewport_matrix[3] * 0.5f - font_big->length(level_str) * 0.5f,
                        viewport_matrix[2] - font_big->font_size * 1.5f,
                        level_str,
                        &font_color);
    }
    
    font_color->x = 0.0f;
    font_color->y = 0.0f;
    font_color->z = 0.0f;

    sprintf(gem_str, "Gem Points:%02d", gem_points);
    sprintf(time_str, "Game Time:%02.2f", game_time * 0.1f);

    font_small->print(viewport_matrix[3] - font_small->length(gem_str) - 6.0f,
                      (font_small->font_size * 0.5f),
                      gem_str,
                      &font_color);

    font_small->print(8.0f,
                      (font_small->font_size * 0.5f),
                      time_str,
                      &font_color);

    font_color->x = 1.0f;
    font_color->y = 1.0f;
    font_color->z = 0.0f;

    font_small->print(viewport_matrix[3] - font_small->length(gem_str) - 8.0f,
                      (font_small->font_size * 0.5f),
                      gem_str,
                      &font_color);

    font_small->print(6.0f,
                      (font_small->font_size * 0.5f),
                      time_str,
                      &font_color);

    if (!game_state) game_time += background_sound->get_time();
}


void templateAppToucheBegan(float x, float y, unsigned int tap_count)
{
    if (game_state == 1 && tap_count >= 2) game_state = 2;
}


void templateAppAccelerometer(float x, float y, float z)
{
    vec3 tmp(x, y, z);

    tmp.safeNormalize();

    accelerometer->x = tmp->x + 0.35f;

#ifndef __IPHONE_4_0

    accelerometer->y = tmp->y + 0.35f;
#else

    accelerometer->y = tmp->y;
#endif
}


void templateAppExit(void) {

    free_level();

    delete thread;
    thread = NULL;

    AUDIO_stop();
}
