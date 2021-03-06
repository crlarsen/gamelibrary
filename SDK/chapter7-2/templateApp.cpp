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

GFX *gfx = NULL;

OBJ *obj = NULL;

PROGRAM *program = NULL;

TEMPLATEAPP templateApp = {
    templateAppInit,
    templateAppDraw,
    templateAppToucheBegan,
    templateAppToucheMoved,
    templateAppToucheEnded
};

/* To contain the rotation on the Z axis of the camera. */
float rotz = 0.0f;

/* To remember the current touch location. */
vec2 touche_location(0.0f, 0.0f),
/* The touch delta (to use in the touch moved callback.) */
     touche_delta   (0.0f, 0.0f);
/* The current eye location of the camera in world coordinates.  Give a
 * little offset on the Z axis to simulate the position of a "human" eye
 * looking at the scene.
 */
vec3 eye_location (0.0f, 0.0f, 1.84f);

vec4 frustum[6];

void program_bind_attrib_location(void *ptr) {
    PROGRAM *program = (PROGRAM *)ptr;

    glBindAttribLocation(program->pid, VA_Position,  VA_Position_String);
    glBindAttribLocation(program->pid, VA_TexCoord0, VA_TexCoord0_String);
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
    const float originalFovy=80.0f;
    float fovy(originalFovy);
    if (height*iPhoneOriginalWidth > width*iPhoneOriginalHeight) {
        float   h = (iPhoneOriginalHeight*0.5f) / tanf(originalFovy*0.5f*DEG_TO_RAD);
        fovy = 2.0f * atan2f(((float)height)*0.5, h) * RAD_TO_DEG;
    }
    gfx->set_perspective(fovy,
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

    gfx->set_matrix_mode(MODELVIEW_MATRIX);
    gfx->load_identity();

    /* The touch delta Y only affects the rotation, so check if you got a
     * value different than 0 to process the forward/backward movements.
     */
    if (touche_delta->x) {
        /* Declare the forward vector.  In this case, the forward direction is
         * the positive Y axis.
         */
        vec3 forward(0.0f, 1.0f, 0.0f),
        /* Declare the direction vector that you will use to affect the
         * current eye location of the camera.
         */
        direction;

        /* Rotate the current forward vector based on the current Z rotation.
         * By doing this, regardless of the rotation angle Z of the camera, up will
         * always be forward and down will always be backward.
         */
        float   r = rotz * DEG_TO_RAD,
                c = cosf(r),
                s = sinf(r);

        direction->x = c * forward->x - s * forward->y;
        direction->y = s * forward->x + c * forward->y;
        /* You now have a direction vector that is appropriately rotated to
         * the current coordinate system of the camera.  Add the direction
         * vector to the eye_location to make the camera move.  And use the
         * touche_delta->x as the speed factor.
         */
        eye_location->x += direction->x * -touche_delta->x;
        eye_location->y += direction->y * -touche_delta->x;
    }

//    /* Rotate the matrix -90 degrees on the positive X axis to look
//     * forward on the Y axis.
//     */
//    gfx->rotate(-90.0f, 1.0f, 0.0f, 0.0f);
//    /* Then rotate it on the Z axis using the rotation controlled by the
//     * movement of the onscreen touch.
//     */
//    gfx->rotate(-rotz, 0.0f, 0.0f, 1.0f);
    float   alpha(-rotz*DEG_TO_RAD_DIV_2);
    float   cosAlpha(cosf(alpha)), sinAlpha(sinf(alpha));
    gfx->rotate(quaternion(M_SQRT1_2*cosAlpha, -M_SQRT1_2*cosAlpha,
                           M_SQRT1_2*sinAlpha,  M_SQRT1_2*sinAlpha));

    // CRL - In this case we can't push the translation through
    // calculating the frustum, and into the loop to be merged with the
    // other translation operation.
    /* First translate the model view matrix. */
    gfx->translate(-eye_location);

    /* Build the frustum planes.  Always make sure that you call the
     * function after the model view and project matrix have been fully updated.
     * otherwise, the frustum calculation will be wrong.
     */
    build_frustum(frustum,
                  gfx->get_modelview_matrix(),
                  gfx->get_projection_matrix());

    unsigned int n = 0;

    for (auto objmesh=obj->objmesh.begin();
         objmesh!=obj->objmesh.end(); ++objmesh) {

        /* Get the distance of the current mesh in the frustum. */
        objmesh->distance = sphere_distance_in_frustum(frustum,
                                                       &objmesh->location,
                                                       objmesh->radius);
        /* If the distance of the mesh is != 0, it means that the object
         * is visible, so you should draw it onscreen.
         */
        if (objmesh->distance) {
            gfx->push_matrix();

            gfx->translate(objmesh->location);

            glUniformMatrix4fv(program->uniform_map["MODELVIEWPROJECTIONMATRIX"].location,
                               1,
                               GL_FALSE,
                               gfx->get_modelview_projection_matrix().m());

            objmesh->draw();

            gfx->pop_matrix();

            /* Increment the visible object counter. */
            ++n;
        }
    }
    
    /* Report on the console the number of objects currently visible
     * in the frustum.
     */
    console_print("Visible Objects: %d\n", n);
}

void templateAppToucheBegan(float x, float y, unsigned int tap_count)
{
    /* Remember the touch location when the onscreen movement starts. */
    touche_location->x = x;
    touche_location->y = y;
}


void templateAppToucheMoved(float x, float y, unsigned int tap_count)
{
    /* Calculate the XY delta (that you will use as a direction vector)
     * for the current movement onscreen and clamp the range of both the movement
     * and the rotation since you don't want the movement to go wild because the
     * current delta unit is in pixels and the units inside your world are in
     * meters.
     *
     * To insure a consistent movement on all platforms, make sure you use
     * linear interpolation to smooth the values.
     *
     * On iOS, the touch movements are already pretty smooth; however on
     * Android, depending on the type of touch screen you are dealing with, you
     * might get some jaggy results.  You will fix this problem by interpolating
     * the touch location.
     */
    touche_delta->x = touche_delta->x * 0.9f + CLAMP(touche_location->x - x, -
                                                   0.1f, 0.1f) * 0.1f;
    touche_delta->y = touche_delta->y * 0.9f + CLAMP(touche_location->y - y, -
                                                   2.0f, 2.0f) * 0.1f;

    /* Remember the current location for the next touch movement pass. */
    touche_location->x = x;
    touche_location->y = y;

    /* Convert the touch delta Y into a rotation angle as you did previously
     * in your OBJ viewer.  But this time, the rotation will not be affected on
     * the complex geometry but on the camera view matrix.
     */
    rotz += touche_delta->y;
}


void templateAppToucheEnded(float x, float y, unsigned int tap_count)
{
    touche_delta->x =
    touche_delta->y = 0.0f;
}


void templateAppExit(void) {
    delete program;
    program = NULL;

    delete obj;
}
