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


vec3 location(0.0f, 0.0f, 1.84f);

float   rotz = 0.0f,
        rotx = 90.0f;

vec4 frustum[6];

/* Since you are basically going to split the screen in two in order to
 * have the left and right sides as an independent analog stick, declare the
 * following variable to be able to remember the screen width in landscape_mode.
 */
float screen_size = 0.0f;
/* Declare 2 two-dimensional vectors; one to remember the touch starting
 * location on the right side of the screen, and the other to calculate the
 * delta when the touch is moved.
 */
vec2 view_location,
     view_delta(0.0f, 0.0f);
/* Same as above for the view, but this time for the camera location.
 * In addition, create another variable for the movement delta.  Note that you
 * declare it as a 3D vector because you will be using the Z as the force
 * factor to smooth the movement.
 */
vec3 move_location(0.0f, 0.0f, 0.0f),
     move_delta;

TEMPLATEAPP templateApp = {
    templateAppInit,
    templateAppDraw,
    templateAppToucheBegan,
    templateAppToucheMoved,
    templateAppToucheEnded
};


void program_bind_attrib_location(void *ptr) {
    PROGRAM *program = (PROGRAM *)ptr;

    glBindAttribLocation(program->pid, VA_Position,  VA_Position_String);
    glBindAttribLocation(program->pid, VA_TexCoord0, VA_TexCoord0_String);
}


void templateAppInit(int width, int height) {
    /* Remember the screen height (in landscape mode, the width of the
     * screen).
     */
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

    /* First make sure that either the X or Y view_delta actually has a
     * value, in order to avoid processing movements for nothing.
     */
    if (view_delta->x || view_delta->y) {
        /* If the delta Y is active (!=0), apply it to the Z rotation of
         * the camera.
         */
        if (view_delta->y) rotz -= view_delta->y;
        /* If the delta X is active, apply it to the X rotation.  And since
         * you don't want the view to start flipping, clamp it in the range
         * of 0 to 180.  This way, the user will be restricted to look from
         * straight up to straight down (since forward is 90 degrees).
         */
        if (view_delta->x) {
            rotx += view_delta->x;
            rotx = CLAMP(rotx, 0.0f, 180.0f);
        }

        /* Set the deltas back to 0. */
        view_delta->x =
        view_delta->y = 0.0f;
    }

    /* Check if you have a force (!=0). */
    if (move_delta.z) {
        /* Rotate the move_delta coordinate system by the current Z rotation
         * of the camera.  This way, forward will always be up, backward will
         * always be down, left will always be left, and right will always be
         * right.
         */
        vec3 forward;

        float   r = rotz * DEG_TO_RAD,  /* Convert the rotz to radians. */
                c = cosf(r),    /* Get the cosine for the Z rotation. */
                s = sinf(r);    /* "   "   "      "   "   X rotation. */
        /* Calculate the movement direction rotated on the Z axis; in other
         * words, the forward vector based on the movement direction (delta)
         * using the camera rotation space.
         */
        forward.x = c * move_delta.y - s * move_delta.x;
        forward.y = s * move_delta.y + c * move_delta.x;
        /* Add the vector to the current camera location and multiply it by a
         * factor (basically the camera speed) to regulate the movements.
         */
        location.x += forward.x * move_delta.z * 0.1f;
        location.y += forward.y * move_delta.z * 0.1f;
        /* Get the sine for the rotz and offset it 90 degrees to make sure it
         * fits with the world positive Y axis (the forward vector).
         */
        forward.z = sinf((rotx - 90.0f) * DEG_TO_RAD);
        /* If the movement delta on the X axis (either positive or negative) is
         * almost a fully straight movement (near -1 or 1), take in consideration
         * the Z elevation.
         */
        if (move_delta.x < -0.99f)
            location.z -= forward.z * move_delta.z * 0.1f;

        else if (move_delta.x > 0.99f)
            location.z += forward.z * move_delta.z * 0.1f;
    }

	GFX_rotate(-rotx, 1.0f, 0.0f, 0.0f);

	GFX_rotate(-rotz, 0.0f, 0.0f, 1.0f);

	GFX_translate(-location.x, -location.y, -location.z);

    build_frustum(frustum,
                  GFX_get_modelview_matrix(),
                  GFX_get_projection_matrix());

    for (auto objmesh=obj->objmesh.begin();
         objmesh!=obj->objmesh.end(); ++objmesh) {

        objmesh->distance = sphere_distance_in_frustum(frustum,
                                                       &objmesh->location,
                                                       objmesh->radius);

        if (objmesh->distance > 0.0f) {
            GFX_push_matrix();

            GFX_translate(objmesh->location.x,
                          objmesh->location.y,
                          objmesh->location.z);

            glUniformMatrix4fv(program->uniform_map["MODELVIEWPROJECTIONMATRIX"].location,
                               1,
                               GL_FALSE,
                               (float *)GFX_get_modelview_projection_matrix());
            
            objmesh->draw();
            
            GFX_pop_matrix();
        }
    }
}


void templateAppToucheBegan(float x, float y, unsigned int tap_count)
{
    /* Analyze from which side of the screen the touch was emitted.  And
     * depending on whether it's on the left or the right, remember the
     * starting point of the touch for either the movement or the view.
     */
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
    /* First create a "dead zone" that occupies 10% of the screen size,
     * located at the center of the screen.  This way, if the user is on one side
     * of the screen and swipes all the way to the other side, you can then stop
     * the movement.
     */
    if (y > ((screen_size * 0.5f) - (screen_size * 0.05f)) &&
        y < ((screen_size * 0.5f) + (screen_size * 0.05f))) {
        /* Stop the current movement for the view or if the camera is on the move.
         */
        move_delta.z  =
        view_delta->x =
        view_delta->y = 0.0f;
        /* In order to make things easier for the user, assign the current
         * location of the touch to be either the starting point of the view or the
         * movement, since you never know in which direction the user will move the
         * touch.
         */
        move_location.x = x;
        move_location.y = y;

        view_location->x = x;
        view_location->y = y;
    } else if (y < (screen_size * 0.5f)) {
        /* If the touch start is on the left side of the screen, deal with it
         * as a camera movement.
         */
        vec3 touche(x, y, 0.0f);
        /* Calculate the delta to determine which direction the touch is
         * going.
         */
        move_delta = touche - move_location;
        /* Normalize the delta to have a direction vector in the range of
         * -1 to 1.
         */
        move_delta.safeNormalize();
        /* Calculate the force (basically the distance from the starting
         * movement location to the current touch location) and divide it
         * by a factor in pixels.  This way, the closer to the starting point,
         * the slower the movement will be, and as the touch distance increases,
         * the movement speed will increase up to its maximum.
         */
        move_delta.z = CLAMP((move_location-touche).length() / 128.0f,
                             0.0f,
                             1.0f);
    } else {
        /* Since the touch is on the right side of the screen, simply calculate
         * the delta for the view so you can then use it to manipulate the X
         * and Z rotation of the camera.
         */
        /* Calculate the view delta and linearly interpolate the values to
         * smooth things out a bit.
         */
        view_delta->x = view_delta->x * 0.75f + (x - view_location->x) * 0.25f;
        view_delta->y = view_delta->y * 0.75f + (y - view_location->y) * 0.25f;
        /* Remember the current location as the starting point for the next
         * movement (if any). */
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
