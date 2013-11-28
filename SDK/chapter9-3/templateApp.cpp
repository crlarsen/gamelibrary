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

#define OBJ_FILE (char *)"piano.obj"

#define VERTEX_SHADER (char *)"vertex.glsl"

#define FRAGMENT_SHADER (char *)"fragment.glsl"

#define MAX_PIANO_KEY 13

/* To handle the sounds for each piano key. */
SOUNDBUFFER *soundbuffer[MAX_PIANO_KEY];

SOUND *soundsource[MAX_PIANO_KEY];

/* The sound buffer and the sound source for playing the "wrong" sound when
 * the player misses a key in the sequence.
 */
SOUNDBUFFER *wrongbuffer;

SOUND *wrong;

SOUNDBUFFER *ambientbuffer;

SOUND *ambient;

THREAD *thread = NULL;

OBJ *obj = NULL;

PROGRAM *program = NULL;

int viewport_matrix[4];

vec2 touche;
/* Flag to determine if the player tries to pick something onscreen. */
unsigned char pick = 0;
/* Temporary variable to store the color used for picking. */
vec4 color;
/* Index of the sound associated with the object that has been picked. */
unsigned int sound_index = 0;

/* The maximum amount of levels. */
#define MAX_LEVEL 50
/* Flag to control "game over" state. */
unsigned char game_over = 0;
/* The current level index. */
unsigned int cur_level = 0,
             /* The current sound of the level that has to be played
              * automatically when a new sequence is generated.
              */
             cur_level_sound,
             /* Array to contain the auto-generated piano key index for
              * the levels.
              */
             level[MAX_LEVEL],
             /* The current sound played by the user.  This value will
              * increment as the player tries to reproduce the current
              * sequence used in the level.
              */
             cur_player_sound;

FONT *font_small = NULL,
     *font_big   = NULL;

TEMPLATEAPP templateApp = { templateAppInit,
							templateAppDraw,
							templateAppToucheBegan };

void program_bind_attrib_location(void *ptr) {
    PROGRAM *program = (PROGRAM *)ptr;

    glBindAttribLocation(program->pid, VA_Position,  VA_Position_String);
    glBindAttribLocation(program->pid, VA_TexCoord0, VA_TexCoord0_String);
}

/* The thread function callback.  Note that the void *ptr parameter is the
 * userdata pointer thatmight have been set when you called the
 * THREAD_create function.  It is up to you to cast it back to its original
 * type before being able to the variable.
 */
void decompress_stream(void *ptr) {
    /* Update the sound source queue for the ambient sound.  By calling this
     * function, the buffer chunks that have been process will be un-queued
     * and filled with fresh new data decompress directly from the OGG sound
     * buffer in memory.
     */
    SOUND_update_queue(ambient);
}

void next_level(void) {
    /* Increase the current level number. */
	++cur_level;

    /* Randomly generate a piano keys index based on the current level
     * number.
     */
	for (int i=0; i!=cur_level; ++i)
		level[i] = rand() % MAX_PIANO_KEY;

    /* Reset the current sound level so the player can see and hear the
     * sequence to reproduce for the current_level.
     */
	cur_level_sound = 0;
}

void templateAppInit(int width, int height) {

	atexit(templateAppExit);

	GFX_start();
	
    /* Helper function to initialize the device and context as you did at the
     * beginning of this chapter.
     */
    AUDIO_start();
	
	glViewport(0.0f, 0.0f, width, height);
	
	glGetIntegerv(GL_VIEWPORT, &viewport_matrix[0]);

	obj = new OBJ(OBJ_FILE, true);
	
	for (auto objmesh=obj->objmesh.begin();
         objmesh!=obj->objmesh.end(); ++objmesh) {
		objmesh->optimize(128);

		objmesh->build();
		
		objmesh->free_vertex_data();
	}

    /* Declare an empty memory pointer to store the sound buffers. */
    MEMORY *memory = NULL;
    /* Reset the counter. */
    /* Loop until the maximum number of piano keys is reached.  Basically,
     * all piano keys have an object name that corresponds to an OGG file.
     * In this example, the 00.ogg sound file will be associated to the
     * mesh name 00 and so on.
     */
    for (int i=0; i!=MAX_PIANO_KEY; ++i) {
        /* Generate a sound filename based on the current loop counter. */
        char soundfile[MAX_CHAR] = {""};
        sprintf(soundfile, "%02d.ogg", i);
        /* Load the sound file into memory. */
        memory = new MEMORY(soundfile, true);
        /* Create a new sound buffer pointer and associate the content loaded
         * from disk to it.  Note that the OGG decompresiion is automatically
         * handled inside the SOUNDBUFFER_load function.
         */
        soundbuffer[i] = SOUNDBUFFER_load(soundfile, memory);
        /* Free the memory.  At this stage, the sound buffer has been sent
         * to the audio memory, so there is no need to keep the sound file
         * alive in local memory.  The buffer is ready to be used.
         */
        delete memory;
        /* Create a new sound source and link the sound buffer you just
         * created to it.
         */
        soundsource[i] = SOUND_add(obj->objmesh[i].name, soundbuffer[i]);
    }

    /* Next, load the sound to play if the user makes a mistake. */
    memory = new MEMORY((char *)"wrong.ogg", true);
    wrongbuffer = SOUNDBUFFER_load((char *)"wrong", memory);
    delete memory;
    wrong = SOUND_add((char *)"wrong", wrongbuffer);

    memory = new MEMORY((char *)"lounge.ogg", true);
    /* Create the sound buffer using the SOUNDBUFFER_load_stream API.  This
     * function will initialize multiple sound buffer IDs internally and
     * will fill them with uncompressed chunks of the OGG stream.  The
     * function will also automatically queue them in sequence for
     * real-time playback.
     */
    ambientbuffer = SOUNDBUFFER_load_stream((char *)"lounge", memory);
    ambient = SOUND_add((char *)"name", ambientbuffer);
    /* The sound buffer has to be continuously streamed from memory, so
     * you will have to decompress small pieces of the OGG file and queue
     * these chunks.  To make sure that this operation will not affect
     * the performance of the main thread (the one that is drawing),
     * create a new thread that will be used strictly for decompression
     * and queuing.
     */
    thread = THREAD_create(decompress_stream,       // The thread callback
                                                    // function.
                           NULL,    // Use data pointer used to pass
                                    // whatever information you want to
                                    // make to make available inside the
                                    // new process created by the thread.
                           THREAD_PRIORITY_NORMAL,
                           1);  // Thread timeout, or sleep time if you prefer,
                                // in milliseconds.  By setting this parameter,
                                // you can control the update frequency of the
                                // thread on top of its priority.
    THREAD_play(thread);

    SOUND_set_volume(ambient, 0.5f);
    /* Play sound in a loop. */
    SOUND_play(ambient, 1);
    /* Contrary to what you did previously, do not free the sound buffer
     * from the local memory.  The buffer needs to be available for
     * real-time decompression.
     */

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

	srandom(get_milli_time());

	next_level();

	font_small = FONT_init((char *)"foo.ttf");

	FONT_load(font_small,
              font_small->name,
              true,
              32.0f,
              512,
              512,
              32,
              96);

	font_big = FONT_init((char *)"foo.ttf");

	FONT_load(font_big,
              font_big->name,
              true,
              64.0f,
              512,
              512,
              32,
              96);	
}


void templateAppDraw(void) {

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	GFX_set_matrix_mode(PROJECTION_MATRIX);
	GFX_load_identity();
	
	GFX_set_perspective(50.0f,
                        (float)viewport_matrix[2] / (float)viewport_matrix[3],
                        1.0f,
                        100.0f,
                        -90.0f);

	GFX_set_matrix_mode(MODELVIEW_MATRIX);
	GFX_load_identity();


	vec3 e = { 0.0f, -8.0f, 7.5f },
		 c = { 0.0f,  2.0f, 0.0f },
		 u = { 0.0f,  0.0f, 1.0f };
		 
	GFX_look_at(&e, &c, &u);
	
	program->draw();

    /* If the current preview sound is different than the current level,
     * it means that the whole generated sequence has not been played back
     * to the user yet.
     */
    if (cur_level_sound != cur_level) {
        /* Static variable to remember the time. */
        static unsigned int start = get_milli_time();
        /* Wait one second between each sound preview, because playing all
         * of the sounds one after another without any pause between each
         * sound would be too hard for the player to remember.
         */
        if (get_milli_time() - start > 1000) {
            if (SOUND_get_state(soundsource[level[cur_level_sound]]) != AL_PLAYING) {

                SOUND_set_volume(soundsource[level[cur_level_sound]], 1.0f);

                SOUND_play(soundsource[level[cur_level_sound]], 0);

                ++cur_level_sound;
            }

            start = get_milli_time();
        }

        cur_player_sound = 0;
    } else if (pick) {  // If you receive a signal that the user wants to
                        // pick something.
        /* Change the DIFFUSE texture channel to be a channel that you do
         * not use, so the texture color will be black.  With the
         * modifications you've made to the fragment shader, you will be
         * able to simple affect the fragment color with a uniform color
         * passed to the shader, and because black + color = color, your
         * object will be drawn using this unique color.
         */
        glUniform1i(program->get_uniform_location(TM_Diffuse_String), TM_Unused);
        /* Loop for the maximum amount of piano key avoiding to draw the
         * curtain (which is the last object recorded in the OBJ file).
         */
        for (int i=0; i!=obj->objmesh.size(); ++i) {
            /* Get the current OBJMESH structure pointer, and adjust the
             * current modelview matrix to render the object onscreen.
             */
            OBJMESH *objmesh = &obj->objmesh[i];
            GFX_push_matrix();
            GFX_translate(objmesh->location.x,
                          objmesh->location.y,
                          objmesh->location.z);
            glUniformMatrix4fv(program->get_uniform_location((char *)"MODELVIEWPROJECTIONMATRIX"),
                               1,
                               GL_FALSE,
                               (float *)GFX_get_modelview_projection_matrix());
            /* Use the following helper function to generate a unique RGBA
             * value for the current loop index.
             */
            generate_color_from_index(i, &color);
            /* Send the color to the fragment shader. */
            glUniform4fv(program->get_uniform_location((char *)"COLOR"),
                         1,
                         (float *)&color);
            /* Draw the object using the unique color that you have generated
             * above.
             */
            objmesh->draw();
            GFX_pop_matrix();
        }

        /* Now that you've fully rendered the current scene so that each object
         * has its own unique color, it's time to ask GLES to identify the
         * color under the user touch onscreen.  The RGBA result that you
         * will be requesting will be returned in an unsigned byte form.
         * You can then easily extract the numbers in their pure form and
         * associate them with a piano key number.
         */
        ucol4 ucolor;
        /* Be careful with this function, because it requires OpenGLES to
         * fully process all the commands sent down the pipeline and will
         * cause the server (in other words, the driver) to stall until the
         * final color is calculated for the pixels you will be requesting.
         */
        glReadPixels(touche.x,  // X coordinate of the touch.
                     viewport_matrix[3]-touche.y,   // Y coordinate transformed to
                                                    // OpenGLES coordinates.
                     1, 1,      // width x height of request pixel rectangle.
                     GL_RGBA,   // The requested pixel format.
                     GL_UNSIGNED_BYTE,  // The requested pixel type.
                     &ucolor);  // Result.

        /* By default, you have 13 objects that have been rendered onscreen
         * (each piano key).  And because the colors generated have been
         * incremented depending on the current index of the loop counter,
         * the result (if a piano key is picked) shown never exceeds the
         * maximum number of the key index (0 to 12).  This means that the
         * value of the blue (b) component contains the corresponding index
         * of the piano key.
         */
//        if (1<=ucolor.b && ucolor.b<=MAX_PIANO_KEY) {
//            /* Convert the name of the OBJMESH indexed by the b color
//             * component to an unsigned int value, which will correspond to
//             * the index of the piano key sound source in the array.
//             */
//            sscanf(obj->objmesh[ucolor.b].name, "%d", &sound_index);
//
//            /* Set the volume for the key at the top. */
//            SOUND_set_volume(soundsource[sound_index], 1.0f);
//            /* Play the piano key sound. */
//            SOUND_play(soundsource[sound_index], 0);
//        }
        if (1<=ucolor.b && ucolor.b<=MAX_PIANO_KEY) {
            sscanf(obj->objmesh[ucolor.b].name, "%d", &sound_index);

            if (level[cur_player_sound] != sound_index) {
                SOUND_set_volume(wrong, 1.0f);

                SOUND_play(wrong, 0);

                game_over = 1;
            } else {
                SOUND_set_volume(soundsource[sound_index], 1.0f);

                SOUND_play(soundsource[sound_index], 0);

                ++cur_player_sound;
            }
        }

        /* Clear the depth buffer and color buffer, because you are about
         * to draw the scene again.
         */
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    }

    pick = 0;

    unsigned char source_playing = 0;

	glUniform1i(program->get_uniform_location(TM_Diffuse_String), TM_Diffuse);

	for (auto objmesh=obj->objmesh.begin();
         objmesh!=obj->objmesh.end(); ++objmesh) {

		GFX_push_matrix();

		GFX_translate(objmesh->location.x,
                      objmesh->location.y,
                      objmesh->location.z);

		glUniformMatrix4fv(program->get_uniform_location((char *)"MODELVIEWPROJECTIONMATRIX"),
                           1,
                           GL_FALSE,
                           (float *)GFX_get_modelview_projection_matrix());

        /* Convert the current mesh name to an index that corresponds to
         * the sound source index of the piano key sound source array.
         */
        sscanf(objmesh->name, "%d", &sound_index);
        /* Check the sound source is currently playing. */
        if (!strstr(objmesh->name, "curtain") &&
           SOUND_get_state(soundsource[sound_index]) == AL_PLAYING) {
            source_playing = 1;
            /* Set full brightness as the color to use for the piano key
             * that is about to be drawn onscreen (since you fragment
             * shader now supports color additions on the final
             * gl_FragColor value.
             */
            color.x = 1.0f;
            color.y = 1.0f;
            color.z = 1.0f;
            color.w = 1.0f;
        } else {
            /* Set the color be fully black so the color won't affect
             * the texture color of the piano key (because black + color
             * = color).
             */
            color.x =
            color.y =
            color.z = 0.0f;
            color.w = 1.0f;
        }

        glUniform4fv(program->get_uniform_location((char *)"COLOR"),
                     1,
                     (float *)&color);

		objmesh->draw();
		
		GFX_pop_matrix();
	}

    /* If the current player sound index is equal to the current level
     * number, it means that the player has cleared the level and entered
     * all the piano keys in the right order.  Time to move on to the
     * next level!
     */
	if (cur_player_sound == cur_level && !source_playing) next_level();

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

	char str[MAX_CHAR] = {""};

	if (game_over) {

		strcpy(str, "GAME OVER");

		/* Yellow. */
		color.x = 1.0f;
		color.y = 1.0f;
		color.z = 0.0f;
		color.w = 1.0f;

		FONT_print(font_big,
			   viewport_matrix[3] * 0.5f -
			   FONT_length(font_big, str) * 0.5f,
			   viewport_matrix[2] -
			   font_big->font_size * 1.5f,
			   str,
			   &color);
}


	sprintf(str, "Level:%d", cur_level);

	/* Green. */
	color.x = 0.0f;
	color.y = 1.0f;
	color.z = 0.0f;
	color.w = 1.0f;
	
	FONT_print(font_small,
		   5.0f,
		   viewport_matrix[2] - font_small->font_size,
		   str,
		   &color);
}


void templateAppToucheBegan(float x, float y, unsigned int tap_count)
{
    /* If the game is not over, allow the player to pick a piano key. */
    if (!game_over) {
        pick = 1;
    } else if (game_over && tap_count >= 2) {
        /* Otherwise, if the player double-taps the screen ...
         * reset the "game over" state and set the current level back to 0.
         */
        game_over =
        cur_level = 0;

        next_level();
    }

	touche.x = x;
	touche.y = y;
}


void templateAppExit(void) {
	FONT_free(font_small);

	FONT_free(font_big);

    /* Stop and free the decompression thread. */
    THREAD_free(thread);

    /* Loop while until the maximum number of piano keys is reached. */
    for (int i=0; i!=MAX_PIANO_KEY; ++i) {
        /* Stop and free the sound sources for each piano key as well as
         * their associated buffer.
         */
        SOUND_free(soundsource[i]);
        SOUNDBUFFER_free(soundbuffer[i]);
    }

    /* Same as above but for "wrong" sound. */
    SOUND_free(wrong);
    SOUNDBUFFER_free(wrongbuffer);

    /* Free the ambient source. */
    SOUND_free(ambient);

    /* Now it is time to free the ambient buffer memory.  Since no more
     * streaming will take place, you can now freely dispose of it.  As
     * you can see, the memory pointer has been stored within the
     * SOUNDBUFFER structure, so you can free it when the application
     * exits.
     */
    delete ambientbuffer->memory;
    /* Free the sound buffer structure for the ambient music. */
    SOUNDBUFFER_free(ambientbuffer);
    /* Stop OpenAL, and free the device and its associated context. */
    AUDIO_stop();

    delete program;
    program = NULL;

    delete obj;
}
