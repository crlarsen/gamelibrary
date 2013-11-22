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
 * - TEXTURE
 */

#include "templateApp.h"
#define OBJ_FILE (char *)"scene.obj"
PROGRAM *program = NULL;
OBJ *obj = NULL;
TEMPLATEAPP templateApp = {
    templateAppInit,
    templateAppDraw
};

void program_bind_attrib_location(void *ptr) {
    PROGRAM *program = (PROGRAM *)ptr;
    glBindAttribLocation(program->pid, VA_Position,  VA_Position_String);
    glBindAttribLocation(program->pid, VA_TexCoord0, VA_TexCoord0_String);
}

/* This time you will use the material draw callback instead of the
 * program draw callback, since in this chapter you will work on a
 * material basis, not on a shader program basis.
 */
void material_draw_callback(void *ptr)
{
    OBJMATERIAL *objmaterial = (OBJMATERIAL *)ptr;
    PROGRAM *program = objmaterial->program;

    for (auto it=program->uniform_map.begin(); it!=program->uniform_map.end(); ++it) {
        auto    &name = it->first;
        auto    &uniform = it->second;
        if (name == TM_Diffuse_String) {
            /* If a diffuse texture is specified inside the MTL file, it
             * will always be bound to the second texture channel
             * (GL_TEXTURE1).
             */
            glUniform1i(uniform.location, TM_Diffuse);
        } else if (name == "MODELVIEWPROJECTIONMATRIX") {
            /* Send over the current model view matrix multiplied by the
             * projection matrix.
             */
            glUniformMatrix4fv(uniform.location, 1, GL_FALSE, (float *)GFX_get_modelview_projection_matrix());
        }
    }
}

void templateAppInit(int width, int height)
{
    atexit(templateAppExit);

    GFX_start();
    
    glViewport(0.0f, 0.0f, width, height);
    
    GFX_set_matrix_mode(PROJECTION_MATRIX);
    GFX_load_identity();
    GFX_set_perspective(45.0f,
                        (float)width / (float)height,
                        0.1f,
                        100.0f,
                        -90.0f);    // This time you will use a landscape
                                    // view, so rotate the projection
                                    // matrix 90 degrees.

    obj = new OBJ(OBJ_FILE, true);

    /* While there are some objects. */
    for (auto objmesh=obj->objmesh.begin(); objmesh!=obj->objmesh.end(); ++objmesh) {
        /* Generate the VBOs and VAO for the current object. */
        objmesh->build();  // The object index inside the OBJ structure.

        /* Free all the vertex data related arrays. At this point, they
         * have all been transferred to the video memory by the
         * OBJMESH::build call.
         */
        objmesh->free_vertex_data();
    } /* Move to the next object. */

    for (auto texture=obj->texture.begin();
         texture!=obj->texture.end(); ++texture) {
        (*texture)->build(obj->texture_path,    // By default the same as where the .mtl is located.
                          TEXTURE_MIPMAP,
                          TEXTURE_FILTER_2X,
                          0.0f);
    }

    /* Load the global vertex shader that you are going to use for all
     * the material shader programs.
     */
    MEMORY *vertex_shader = mopen((char *)"Uber.vs", 1);

    for (auto objmaterial=obj->objmaterial.begin();
         objmaterial!=obj->objmaterial.end(); ++objmaterial) {
        MEMORY *fragment_shader = mopen((char *)"Uber.fs", 1);

        objmaterial->build(NULL);
        /* The material has no alpha, so it is considered a solid
         * object. In addition, please note that the current and
         * following conditions are strictly based on the dissolve
         * value. However, in a real-world scenario, an extra check has
         * to be made on the number of bits of the texture to analyze if
         * it contains an alpha channel. And modifications in the
         * shaders are required to handle this type of scenario.
         */
        /* If dissolve is equal to one it means that the material you
         * are dealing with will be applied on solid objects.
         */
        if (objmaterial->dissolve == 1.0f)
            /* This function will insert the appropriate #define code
             * at an arbitrary position in the current fragment shader
             * memory stream that you loaded in the previous step.
             */
            minsert(/* The memory stream. */   fragment_shader,
                    /* The code to insert. */   (char *)"#define SOLID_OBJECT\n",
                    /* The character position in the stream to insert
                     * the code. In this case, it's 0, which represents
                     * the beginning of the stream. This way, only the
                     * block of the marked the #ifdef SOLID_OBJECT will
                     * be enabled.
                     */
                    0);
        /* There's no real way in an OBJ material file to tag an object
         * for alpha test. Simply use the 0.0 value on the dissolve
         * parameter to tag the object as an alpha-tested object.
         */
        else if (!objmaterial->dissolve)
            /* Insert the necessary code to enable the
             * ALPHA_TESTED_OBJECT code block of the shader by manually
             * inserting the definition.
             */
            minsert(fragment_shader,
                    (char *)"#define ALPHA_TESTED_OBJECT\n",
                    0);
        /* Same as above except that if the object does not fall between
         * the two previous conditions, you have to treat it as a
         * transparent object.
         */
        else {
            minsert(fragment_shader, (char *)"#define TRANSPARENT_OBJECT\n", 0);
        }

        /* Use the objmaterial program pointer to initialize the shader program. */
        objmaterial->program = new PROGRAM(objmaterial->name);
        /* Create the vertex shader. */
        objmaterial->program->vertex_shader = new SHADER((char *)"vertex",
                                                         GL_VERTEX_SHADER);

        /* Create the fragment shader. */
        objmaterial->program->fragment_shader = new SHADER((char *)"fragment",
                                                           GL_FRAGMENT_SHADER);
        
        /* Compile both the vertex and fragment programs. */
        objmaterial->program->vertex_shader->compile((char *)vertex_shader->buffer, true);
        
        objmaterial->program->fragment_shader->compile((char *)fragment_shader->buffer, true);
        
        /* Link the bind attribute location callback BEFORE the linking
         * phase of the shader program to insure that the location of
         * the attribute that you specify will be taken into
         * consideration.
         */
        objmaterial->program->set_bind_attrib_location_callback(program_bind_attrib_location);
        
        /* Link the shader program so you can use it for drawing. */
        objmaterial->program->link(true);

        /* Assign the draw callback to the material so you can receive
         * live feedback before drawing in order to update your uniform
         * variables based on the current material data.
         */
        objmaterial->set_draw_callback(material_draw_callback);

        /* Close and free the memory stream. */
        mclose(fragment_shader);
    }

    mclose(vertex_shader);
}

void templateAppDraw(void)
{
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    GFX_set_matrix_mode(MODELVIEW_MATRIX);
    GFX_load_identity();
    vec3 e = { 0.0f, -6.0f, 1.35f }, /* The location of the camera. */
         c = { 0.0f, -5.0f, 1.35f }, /* Where the camera is looking. */
         u = { 0.0f, 0.0f, 1.0f };
    
     GFX_look_at(&e, &c, &u);

    /* Solid Objects */
    for (int i=0; i != obj->objmesh.size(); ++i) {
        /* Get the material pointer of the first triangle list of the
         * current mesh. All your objects are using a single material,
         * so only one triangle list is available. By getting access to
         * the first triangle list, you can now gain access to the
         * material used by your current mesh. You can then use the
         * material dissolve property as you did earlier to classify
         * your object at render time.
         */
        OBJMATERIAL *objmaterial = obj->objmesh[i].objtrianglelist[0].objmaterial;
        /* Is it a solid object? */
        if (objmaterial->dissolve == 1.0f) {
            GFX_push_matrix();
            GFX_translate(obj->objmesh[i].location.x,
                          obj->objmesh[i].location.y,
                          obj->objmesh[i].location.z);
            obj->objmesh[i].draw();
            GFX_pop_matrix();
        }
    }

    /* Alpha-Tested Objects */
    for (auto objmesh=obj->objmesh.begin();
         objmesh!=obj->objmesh.end(); ++objmesh) {
        OBJMATERIAL *objmaterial = objmesh->objtrianglelist[0].objmaterial;
        if (!objmaterial->dissolve) {
            GFX_push_matrix();
            GFX_translate(objmesh->location.x,
                          objmesh->location.y,
                          objmesh->location.z);
            objmesh->draw();
            GFX_pop_matrix();
        }
    }

    /* Tell the GPU to enable blending. */
    glEnable(GL_BLEND);
    /* Specify which source and destination function to use for
     * blending. In this case, all you want is the alpha value of the
     * fragment to be used by the blending operation to make your object
     * semitransparent.
     */
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    /* Transparent Objects */
    for (auto objmesh=obj->objmesh.begin();
         objmesh!=obj->objmesh.end(); ++objmesh) {
        OBJMATERIAL *objmaterial = objmesh->objtrianglelist[0].objmaterial;
        /* If the current dissolve value doesn't fit the conditions of
         * the solid or alpha tested objects, the current object has to
         * be transparent, so draw it onscreen.
         */
        if (objmaterial->dissolve>0.0f && objmaterial->dissolve<1.0f) {
            GFX_push_matrix();
            GFX_translate(objmesh->location.x,
                          objmesh->location.y,
                          objmesh->location.z);
            objmesh->draw();
            GFX_pop_matrix();
        }
    }

    /* Every time you enable a machine state, remember to turn it back
     * OFF when you don't need it. This way, you won't have any
     * surprises when drawing the next frame.
     */
    glDisable(GL_BLEND);
}

void templateAppExit(void)
{
    delete obj;
}
