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
 */

#include "templateApp.h"

#define OBJ_FILE (char *)"Scene.obj"

#define VERTEX_SHADER (char *)"vertex.glsl"

#define FRAGMENT_SHADER (char *)"fragment.glsl"

OBJ *obj = NULL;

PROGRAM *program = NULL;

TEMPLATEAPP templateApp = { templateAppInit,
							templateAppDraw,
							templateAppToucheBegan,
							templateAppToucheMoved,
							templateAppToucheEnded };

/* To contain the rotation on the Z axis of the camera. */
float rotz = 0.0f;

/* To remember the current touch location. */
vec2 touche_location = { 0.0f, 0.0f },
/* The touch delta (to use in the touch moved callback.) */
     touche_delta    = { 0.0f, 0.0f };
/* The current eye location of the camera in world coordinates.  Give a
 * little offset on the Z axis to simulate the position of a "human" eye
 * looking at the scene.
 */
vec3 eye_location  = { 0.0f, 0.0f, 1.84f };

vec4 frustum[6];

void program_bind_attrib_location(void *ptr) {
	PROGRAM *program = (PROGRAM *)ptr;

	glBindAttribLocation(program->pid, 0, "POSITION");
	glBindAttribLocation(program->pid, 2, "TEXCOORD0");
}


void templateAppInit(int width, int height) {
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
	
	
	for (int i=0; i!=obj->texture.size(); ++i)
		OBJ_build_texture(obj,
                          i,
                          obj->texture_path,
                          TEXTURE_MIPMAP | TEXTURE_16_BITS,
                          TEXTURE_FILTER_2X,
                          0.0f);


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
	
	glUniform1i(program->get_uniform_location((char *)"DIFFUSE"), 1);
}


void templateAppDraw(void) {

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	GFX_set_matrix_mode(MODELVIEW_MATRIX);
	GFX_load_identity();

    /* The touch delta Y only affects the rotation, so check if you got a
     * value different than 0 to process the forward/backward movements.
     */
    if (touche_delta.x) {
        /* Declare the forward vector.  In this case, the forward direction is
         * the positive Y axis.
         */
        vec3 forward = { 0.0f, 1.0f, 0.0f },
             /* Declare the direction vector that you will use to affect the
              * current eye location of the camera.
              */
             direction;

        /* Rotate the current forward vector based on the current Z rotation.
         * By doing this, regardless of the rotation angle Z of the camera, up will
         * always be forward and down will always be backward.
         */
        float r = rotz * DEG_TO_RAD,
              c = cosf(r),
              s = sinf(r);

        direction.x = c * forward.x - s * forward.y;
        direction.y = s * forward.x + c * forward.y;
        /* You now have direction vector that is appropriately rotated to
         * the current coordinate system of the camera.  Add the direction
         * vector to the eye_location to make the camera move.  And use the
         * touche_delta.x as the speed factor.
         */
        eye_location.x += direction.x * -touche_delta.x;
        eye_location.y += direction.y * -touche_delta.x;
    }

    /* First translate the model view matrix. */
    GFX_translate(eye_location.x,
                  eye_location.y,
                  eye_location.z);
    /* Then rotate it on the Z axis using the rotation controlled by the
     * movement of the onscreen touch.
     */
    GFX_rotate(rotz, 0.0f, 0.0f, 1.0f);
    /* Next, rotate the matrix of 90 degrees on the positive X axis to look
     * forward on the Y axis.
     */
    GFX_rotate(90.0f, 1.0f, 0.0f, 0.0f);
    /* Invert the current model view matrix to create a camera view matrix. */
    mat4_invert(GFX_get_modelview_matrix());

    /* Build the frustum planes.  Always make sure that you call the
     * function after the model view and project matrix have been fully updated.
     * otherwise, the frustum calculation will be wrong.
     */
    build_frustum(frustum,
                  GFX_get_modelview_matrix(),
                  GFX_get_projection_matrix());

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
    touche_location.x = x;
    touche_location.y = y;
}


void templateAppToucheMoved(float x, float y, unsigned int tap_count)
{
    /* Calculate the XY delta (thatyou will use as a direction vector)
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
    touche_delta.x = touche_delta.x * 0.9f + CLAMP(touche_location.x - x, -
                                                   0.1f, 0.1f) * 0.1f;
    touche_delta.y = touche_delta.y * 0.9f + CLAMP(touche_location.y - y, -
                                                   2.0f, 2.0f) * 0.1f;

    /* Remember the current location for the next touch movement pass. */
    touche_location.x = x;
    touche_location.y = y;

    /* Convert the touch delta Y into a rotation angle as you did previously
     * in your OBJ viewer.  But this time, the rotation will not be affected on
     * the complex geometry but on the camera view matrix.
     */
    rotz += touche_delta.y;
}


void templateAppToucheEnded(float x, float y, unsigned int tap_count)
{
    touche_delta.x =
    touche_delta.y = 0.0f;
}


void templateAppExit(void) {
    delete program;
    program = NULL;

    delete obj;
}
