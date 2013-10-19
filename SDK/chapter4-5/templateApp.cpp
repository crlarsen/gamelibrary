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
    glBindAttribLocation(program->pid, 0, "POSITION");
    glBindAttribLocation(program->pid, 1, "NORMAL");
    glBindAttribLocation(program->pid, 2, "TEXCOORD0");
}

/* This time you will use the material draw callback instead of the
 * program draw callback, since in this chapter you will work on a
 * material basis, not on a shader program basis.
 */
void material_draw_callback(void *ptr)
{
    OBJMATERIAL *objmaterial = (OBJMATERIAL *)ptr;
    PROGRAM *program = objmaterial->program;
    unsigned int i = 0;
    while (i != program->uniform_count) {
        if (!strcmp(program->uniform_array[i].name, "DIFFUSE")) {
            /* If a diffuse texture is specified inside the MTL file, it
             * will always be bound to the second texture channel
             * (GL_TEXTURE1).
             */
              glUniform1i(program->uniform_array[i].location, 1);
        } else if (!strcmp(program->uniform_array[i].name, "MODELVIEWPROJECTIONMATRIX")) {
            /* Send over the current model view matrix multiplied by the
             * projection matrix.
             */
            glUniformMatrix4fv(program->uniform_array[i].location, 1, GL_FALSE,
                               (float *)GFX_get_modelview_projection_matrix());
        } else if (!strcmp(program->uniform_array[i].name, "DISSOLVE")) {
            glUniform1f(program->uniform_array[i].location, objmaterial->dissolve);
        } else if (!strcmp(program->uniform_array[i].name, "AMBIENT_COLOR")) {
            glUniform3fv(program->uniform_array[i].location, 1,
                         (float *)&objmaterial->ambient);
        } else if (!strcmp(program->uniform_array[i].name, "DIFFUSE_COLOR")) {
            glUniform3fv(program->uniform_array[i].location, 1,
                         (float *)&objmaterial->diffuse);
        } else if (!strcmp(program->uniform_array[i].name, "SPECULAR_COLOR")) {
            glUniform3fv(program->uniform_array[i].location, 1,
                         (float *)&objmaterial->specular);
        } else if (!strcmp(program->uniform_array[i].name, "SHININESS")) {
            glUniform1f(program->uniform_array[i].location,
                        objmaterial->specular_exponent * 0.128f);
        } else if (!strcmp(program->uniform_array[i].name, "MODELVIEWMATRIX")) {
            glUniformMatrix4fv(program->uniform_array[i].location, 1, GL_FALSE,
                               (float *)GFX_get_modelview_matrix());
        } else if (!strcmp(program->uniform_array[i].name, "PROJECTIONMATRIX")) {
            glUniformMatrix4fv(program->uniform_array[i].location, 1, GL_FALSE,
                               (float *)GFX_get_projection_matrix());
        } else if (!strcmp(program->uniform_array[i].name, "NORMALMATRIX")) {
            glUniformMatrix3fv(program->uniform_array[i].location, 1, GL_FALSE,
                               (float *)GFX_get_normal_matrix());
        } else if (!strcmp(program->uniform_array[i].name, "LIGHTPOSITION")) {
            /* The light position in world space coordinates. */
            vec3 position    = { 0.0f, -3.0f, 10.0f };
            vec3 eyeposition = { 0.0f,  0.0f,  0.0f };

            /* Convert the light position to eye space. */
            vec3_multiply_mat4(&eyeposition, &position,
                               &gfx.modelview_matrix[gfx.modelview_matrix_index-1]);
            glUniform3fv(program->uniform_array[i].location, 1,
                         (float *)&eyeposition);
        }

        ++i;
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

    obj = OBJ_load(OBJ_FILE, 1);

    /* Initialize the counter. */
    unsigned int i = 0;
    /* While there are some objects. */
    while (i != obj->n_objmesh) {
        /* Generate the VBOs and VAO for the current object. */
        OBJ_build_mesh(obj, // The OBJ structure to use.
                       i);  // The object index inside the OBJ structure.

        /* Free all the vertex data related arrays. At this point, they
         * have all been transferred to the video memory by the
         * OBJ_build_mesh call.
         */
        OBJ_free_mesh_vertex_data(obj, i);
        ++i;
    } /* Move to the next object. */

    i = 0;
    while (i != obj->n_texture) {
        OBJ_build_texture(obj,  i,  /* By default the same as where the .mtl is located. */
                          obj->texture_path,  TEXTURE_MIPMAP,  TEXTURE_FILTER_2X,  0.0f);
        /* Next texture. */
        ++i;
    }

    i = 0;
    /* Load the global vertex shader that you are going to use for all
     * the material shader programs.
     */
    while (i != obj->n_objmaterial) {
        MEMORY *fragment_shader = mopen((char *)"Uber.fs", 1);
        MEMORY *vertex_shader = mopen((char *)"Uber.vs", 1);

        OBJMATERIAL *objmaterial = &obj->objmaterial[i];
        OBJ_build_material(obj, i, NULL);
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
        else if (!objmaterial->dissolve) {
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
        } else {
            minsert(fragment_shader, (char *)"#define TRANSPARENT_OBJECT\n", 0);
        }

        if (objmaterial->illumination_model) {
            minsert(vertex_shader, (char *)"#define LIGHTING_SHADER\n", 0);
            minsert(fragment_shader, (char *)"#define LIGHTING_SHADER\n", 0);
        }

        /* Use the objmaterial program pointer to initialize the shader program. */
        objmaterial->program = PROGRAM_init(objmaterial->name);
        /* Create the vertex shader. */
        objmaterial->program->vertex_shader = SHADER_init((char *)"vertex",
                                                          GL_VERTEX_SHADER);

        /* Create the fragment shader. */
        objmaterial->program->fragment_shader = SHADER_init((char *)"fragment",
                                                            GL_FRAGMENT_SHADER);
        
        /* Compile both the vertex and fragment programs. */
        SHADER_compile(objmaterial->program->vertex_shader,
                       (char *)vertex_shader->buffer,
                       1);
        
        SHADER_compile(objmaterial->program->fragment_shader,
                       (char *)fragment_shader->buffer,
                       1);
        
        /* Link the bind attribute location callback BEFORE the linking phase of the shader program to insure that the location of the attribute that you specify will be taken into consideration. */
        PROGRAM_set_bind_attrib_location_callback(objmaterial->program,
                                                  program_bind_attrib_location);
        
        /* Link the shader program so you can use it for drawing. */
        PROGRAM_link(objmaterial->program, 1);

        /* Assign the draw callback to the material so you can receive
         * live feedback before drawing in order to update your uniform
         * variables based on the current material data.
         */
        OBJ_set_draw_callback_material(obj, i, material_draw_callback);

        /* Close and free the memory stream. */
        mclose(fragment_shader);
        mclose(vertex_shader);

        ++i;
    }
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

    /* Initialize a counter. */
    unsigned int i = 0;
    /* Solid Objects */
    while (i != obj->n_objmesh) {
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
            OBJ_draw_mesh(obj, i);
            GFX_pop_matrix();
        }
        ++i;
    }

//    /* Alpha-Tested Objects */
//    i = 0;
//    while (i != obj->n_objmesh) {
//        OBJMATERIAL *objmaterial = obj->objmesh[i].objtrianglelist[0].objmaterial;
//        if (!objmaterial->dissolve) {
//            GFX_push_matrix();
//            GFX_translate(obj->objmesh[i].location.x,
//                          obj->objmesh[i].location.y,
//                          obj->objmesh[i].location.z);
//            glCullFace(GL_FRONT);
//            OBJ_draw_mesh(obj, i);
//            glCullFace(GL_BACK);
//            OBJ_draw_mesh(obj, i);
//            GFX_pop_matrix();
//        }
//        ++i;
//    }

    /* Tell the GPU to enable blending. */
    glEnable(GL_BLEND);
    /* Specify which source and destination function to use for
     * blending. In this case, all you want is the alpha value of the
     * fragment to be used by the blending operation to make your object
     * semitransparent.
     */
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    /* Transparent Objects */
    i = 0;
    while (i != obj->n_objmesh) {
        OBJMATERIAL *objmaterial = obj->objmesh[i].objtrianglelist[0].objmaterial;
        /* If the current dissolve value doesn't fit the conditions of
         * the solid or alpha tested objects, the current object has to
         * be transparent, so draw it onscreen.
         */
        if (objmaterial->dissolve != 1.0f) {
            GFX_push_matrix();
            GFX_translate(obj->objmesh[i].location.x,
                          obj->objmesh[i].location.y,
                          obj->objmesh[i].location.z);
            glCullFace(GL_FRONT);
            OBJ_draw_mesh(obj, i);
            glCullFace(GL_BACK);
            OBJ_draw_mesh(obj, i);
            GFX_pop_matrix();
        }
        ++i;
    }
    /* Every time you enable a machine state, remember to turn it back
     * OFF when you don't need it. This way, you won't have any
     * surprises when drawing the next frame.
     */
    glDisable(GL_BLEND);
}

void templateAppExit(void)
{
    unsigned i = 0;
    while (i != obj->n_objmaterial) {
        SHADER_free(obj->objmaterial[i].program->vertex_shader);
        SHADER_free(obj->objmaterial[i].program->fragment_shader);
        PROGRAM_free(obj->objmaterial[i].program);   ++i;
    }

    OBJ_free(obj);
}
