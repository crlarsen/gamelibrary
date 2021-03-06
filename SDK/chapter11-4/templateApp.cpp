/*

Book:      	Game and Graphics Programming for iOS and Android with OpenGL(R) ES 2.0
Author:    	Romain Marucchi-Foino
ISBN-10: 	1119975913
ISBN-13: 	978-1119975915
Publisher: 	John Wiley & Sons	

Copyright (C) 2011 Romain Marucchi-Foino

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of
this software. Permission is granted to anyone who either own or purchase a copy of the book
specified above, to use this software for any purpose,
including commercial applications subject to the following restrictions:

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
 * - GFX
 * - LIGHT
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

OBJ *obj = NULL;

TEMPLATEAPP templateApp = {
    templateAppInit,
    templateAppDraw
};

std::vector<OBJMESH>::iterator objmesh;

int viewport_matrix[4];

LIGHT *light = NULL;


void program_bind_attrib_location(void *ptr) {
    PROGRAM *program = (PROGRAM *)ptr;

    glBindAttribLocation(program->pid, VA_Position,  VA_Position_String);
    glBindAttribLocation(program->pid, VA_Normal,    VA_Normal_String);
    glBindAttribLocation(program->pid, VA_TexCoord0, VA_TexCoord0_String);
    glBindAttribLocation(program->pid, VA_Tangent0,  VA_Tangent0_String);
}


GFX *gfx = NULL;

void program_draw(void *ptr)
{
    PROGRAM *program = (PROGRAM *)ptr;

    for (auto it=program->uniform_map.begin(); it!=program->uniform_map.end(); ++it) {
        auto    &name = it->first;
        auto    &uniform = it->second;

        if (uniform.constant) {
            continue;
        } else if (name == "MODELVIEWPROJECTIONMATRIX") {
            glUniformMatrix4fv(uniform.location,
                               1,
                               GL_FALSE,
                               gfx->get_modelview_projection_matrix().m());
        } else if (name == TM_Diffuse_String) {
            glUniform1i(uniform.location, TM_Diffuse);

            uniform.constant = true;
        } else if (name == TM_Bump_String) {
            glUniform1i(uniform.location, TM_Bump);

            uniform.constant = true;
        } else if (name == "MODELVIEWMATRIX") {
            // Matrix Data
            glUniformMatrix4fv(uniform.location,
                               1,
                               GL_FALSE,
                               gfx->get_modelview_matrix().m());
        } else if (name == "PROJECTIONMATRIX") {
            glUniformMatrix4fv(uniform.location,
                               1,
                               GL_FALSE,
                               gfx->get_projection_matrix().m());

            uniform.constant = true;
        } else if (name == "NORMALMATRIX") {
            glUniformMatrix3fv(uniform.location,
                               1,
                               GL_FALSE,
                               gfx->get_normal_matrix().m());
        } else if (name == "MATERIAL.ambient") {
            // Material Data
            glUniform4fv(uniform.location,
                         1,
                         objmesh->current_material->ambient.v());
            /* In this scene, all the materials (in this case, there are
             * only two) have the exact same properties, so simply tag the
             * uniforms for the current material to be constant.  This will
             * also allow you to get better performance at runtime, because
             * the data will not be sent over and over for nothing.
             */
            uniform.constant = true;
        } else if (name == "MATERIAL.diffuse") {
            glUniform4fv(uniform.location,
                         1,
                         objmesh->current_material->diffuse.v());
        } else if (name == "MATERIAL.specular") {
            glUniform4fv(uniform.location,
                         1,
                         objmesh->current_material->specular.v());
        } else if (name == "MATERIAL.shininess") {
            glUniform1f(uniform.location,
                        objmesh->current_material->specular_exponent * 0.128f);

            uniform.constant = true;
        }
    }

    // Light Data
    light->push_to_shader(gfx, program);
}


void templateAppInit(int width, int height)
{
    atexit(templateAppExit);

    gfx = new GFX;

    glViewport(0.0f, 0.0f, width, height);

    glGetIntegerv(GL_VIEWPORT, viewport_matrix);

    obj = new OBJ(OBJ_FILE, true);

    for (objmesh=obj->objmesh.begin();
         objmesh != obj->objmesh.end(); ++objmesh) {

        objmesh->optimize(128);

        objmesh->build2();

        objmesh->free_vertex_data();
    }

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

    vec4 color(1.0f, 1.0f, 1.0f, 1.0f);

    vec3 position(0.0f, 0.0f, 5.0f);

    light = new PointLight((char *)"point", color, position);

    obj->get_mesh("sphere", false)->objtrianglelist[0].mode = GL_POINTS;
}


void templateAppDraw(void)
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    gfx->set_matrix_mode(PROJECTION_MATRIX);
    gfx->load_identity();

    // Adjust "Field of View Y" angle for devices which has an aspect
    // ratio which is wider than the origin iPhone (3:2).  Devices which
    // have a narrower aspect ratio (such as iPad) work fine, as is.
    const float iPhoneOriginalWidth =320.0f;
    const float iPhoneOriginalHeight=480.0f;
    const float originalFovy=45.0f;
    float fovy(originalFovy);
    if (viewport_matrix[3]*iPhoneOriginalWidth > viewport_matrix[2]*iPhoneOriginalHeight) {
        float   h = (iPhoneOriginalHeight*0.5f) / tanf(originalFovy*0.5f*DEG_TO_RAD);
        fovy = 2.0f * atan2f(((float)viewport_matrix[3])*0.5, h) * RAD_TO_DEG;
    }
    gfx->set_perspective(fovy,
                         (float)viewport_matrix[2] / (float)viewport_matrix[3],
                           0.1f,
                         100.0f,
                         -90.0f);

    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    gfx->set_matrix_mode(MODELVIEW_MATRIX);
    gfx->load_identity();

    const float   alpha(-72.0f*DEG_TO_RAD_DIV_2);
    const float   cosAlpha(cosf(alpha)), sinAlpha(sinf(alpha));
    const float   beta(-48.5f*DEG_TO_RAD_DIV_2);
    const float   cosBeta(cosf(beta)), sinBeta(sinf(beta));
    gfx->rotate(quaternion( cosAlpha*cosBeta, sinAlpha*cosBeta,
                           -sinAlpha*sinBeta, cosAlpha*sinBeta));

    gfx->translate(-14.0f, 12.0f, -7.0f);

    for (objmesh=obj->objmesh.begin();
         objmesh != obj->objmesh.end(); ++objmesh) {

        gfx->push_matrix();

        gfx->translate(objmesh->location);

        /* If the current object name is the sphere. */
        if (strstr(objmesh->name, "sphere")) {
            /* Enable blending. */
            glEnable(GL_BLEND);

            /* Turn off the depth mask.  This is very important when
             * drawing particles.  This will allow the particle to pass
             * the depth test but will not write the depth buffer.
             * Otherwise, you would have to manually sort every point
             * of the geometry and render them from back to front to
             * insure proper additive alpha sorting.
             */
            glDepthMask(GL_FALSE);

            /* Set additive blending. */
            glBlendEquation(GL_FUNC_ADD);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);

            /* Rotate all the points around the object origin. */
            objmesh->rotation->x += 0.5f;
            objmesh->rotation->y += 0.5f;
            objmesh->rotation->z += 0.5f;
            float   alpha(objmesh->rotation->z*DEG_TO_RAD_DIV_2);
            float   cosAlpha(cosf(alpha)), sinAlpha(sinf(alpha));
            float   beta (objmesh->rotation->y*DEG_TO_RAD_DIV_2);
            float   cosBeta(cosf(beta)), sinBeta(sinf(beta));
            float   gamma(objmesh->rotation->x*DEG_TO_RAD_DIV_2);
            float   cosGamma(cosf(gamma)), sinGamma(sinf(gamma));
            float   cAcB(cosAlpha*cosBeta);
            float   sAsB(sinAlpha*sinBeta);
            float   cAsB(cosAlpha*sinBeta);
            float   sAcB(sinAlpha*cosBeta);
            gfx->rotate(quaternion(cAcB*cosGamma+sAsB*sinGamma,
                                   cAcB*sinGamma-sAsB*cosGamma,
                                   cAsB*cosGamma+sAcB*sinGamma,
                                   sAcB*cosGamma-cAsB*sinGamma));

            /* Draw the mesh.  From here, the particles.gfx shader will be
             * called and it will handle the point sizes, attenuations,
             * and the points' texture coordinates.
             */
            objmesh->draw();

            /* Disable blending and re-enable the depth mask writing. */
            glDisable(GL_BLEND);
            glDepthMask(GL_TRUE);
        } else {
            /* If the current object is not the sphere, draw it normally. */
            objmesh->draw();
        }
        
        gfx->pop_matrix();
    }
}


void templateAppExit(void) {
    
    delete light;
    light = NULL;

    delete obj;
}
