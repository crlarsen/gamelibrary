/*

GFX Lightweight OpenGLES 2.0 Game and Graphics Engine

Copyright (C) 2011 Romain Marucchi-Foino http://gfx.sio2interactive.com

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of
this software. Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it freely,
subject to the following restrictions:

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
 * - LIGHT
 * - MD5
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

#include "gfx.h"


MD5::MD5(char *filename, const bool relative_path)
{
    MEMORY  *m = new MEMORY(filename, relative_path);

    if (!m) return;

    memset(name, 0, sizeof(name));

    memset(&location, 0, sizeof(vec3));
    memset(&rotation, 0, sizeof(vec3));
    memset(&min, 0, sizeof(vec3));
    memset(&max, 0, sizeof(vec3));
    memset(&dimension, 0, sizeof(vec3));

    radius = 0.0;

    btrigidbody = NULL;

    get_file_name(filename, this->name);

    this->distance =
    this->scale.x  =
    this->scale.y  =
    this->scale.z  = 1.0f;
    this->visible  = true;

    char *line = strtok((char *)m->buffer, "\n");

    int int_val = 0;

    unsigned int mesh_index = 0;

    while (line) {
        int n_joint, n_mesh;
        if (sscanf(line, "MD5Version %d", &int_val) == 1) {
            if (int_val != 10) goto cleanup;
        } else if (sscanf(line, "numJoints %d", &n_joint) == 1) {
            this->bind_pose.resize(n_joint);
        } else if (sscanf(line, "numMeshes %d", &n_mesh) == 1) {
            this->md5mesh.resize(n_mesh);

            for (auto md5mesh=this->md5mesh.begin();
                 md5mesh != this->md5mesh.end(); ++md5mesh) {
                md5mesh->mode = GL_TRIANGLES;
                md5mesh->visible = true;
            }
        } else if (!strncmp(line, "joints {", 8)) {
            unsigned int i = 0;

            line = strtok(NULL, "\n");

            while (line[0] != '}') {
                if (sscanf(line,
                           "%s %d ( %f %f %f ) ( %f %f %f )",
                           this->bind_pose[i].name,
                           &this->bind_pose[i].parent,
                           &this->bind_pose[i].location.x,
                           &this->bind_pose[i].location.y,
                           &this->bind_pose[i].location.z,
                           &this->bind_pose[i].rotation.x,
                           &this->bind_pose[i].rotation.y,
                           &this->bind_pose[i].rotation.z) == 8) {
                    vec4_build_w(&this->bind_pose[i].rotation);

                    ++i;
                }

                line = strtok(NULL, "\n");
            }
        } else if (!strncmp(line, "mesh {", 6)) {
            MD5VERTEX md5vertex;

            MD5TRIANGLE md5triangle;

            MD5WEIGHT md5weight;

            line = strtok(NULL, "\n");

            unsigned int    n_vertex;

            unsigned int    n_weight;

            unsigned int    n_triangle;

            while (line[0] != '}') {
                if (sscanf(line, " shader \"%[^\"]", this->md5mesh[mesh_index].shader) == 1) {
                    goto next_mesh_line;
                } else if (sscanf(line, " numverts %d", &n_vertex) == 1) {
                    this->md5mesh[mesh_index].md5vertex.resize(n_vertex);
                } else if (sscanf(line,
                                  " vert %d ( %f %f ) %d %d",
                                  &int_val,
                                  &md5vertex.uv.x,
                                  &md5vertex.uv.y,
                                  &md5vertex.start,
                                  &md5vertex.count) == 5) {
                    memcpy(&this->md5mesh[mesh_index].md5vertex[int_val],
                           &md5vertex,
                           sizeof(MD5VERTEX));
                } else if (sscanf(line, " numtris %d", &n_triangle) == 1) {
                    this->md5mesh[mesh_index].n_indice = n_triangle * 3;

                    this->md5mesh[mesh_index].md5triangle.resize(n_triangle);
                } else if (sscanf(line,
                                  " tri %d %hu %hu %hu",
                                  &int_val,
                                  &md5triangle.indice[2],
                                  &md5triangle.indice[1],
                                  &md5triangle.indice[0]) == 4) {
                    memcpy(&this->md5mesh[mesh_index].md5triangle[int_val],
                           &md5triangle,
                           sizeof(MD5TRIANGLE));
                } else if (sscanf(line, " numweights %d", &n_weight) == 1) {
                    this->md5mesh[mesh_index].md5weight.resize(n_weight);
                } else if (sscanf(line,
                                  " weight %d %d %f ( %f %f %f )",
                                  &int_val,
                                  &md5weight.joint,
                                  &md5weight.bias,
                                  &md5weight.location.x,
                                  &md5weight.location.y,
                                  &md5weight.location.z) == 6) {
                    memcpy(&this->md5mesh[mesh_index].md5weight[int_val],
                           &md5weight,
                           sizeof(MD5WEIGHT));
                }
                
            next_mesh_line:
                line = strtok(NULL, "\n");
            }
            
            unsigned int s = this->md5mesh[mesh_index].n_indice * sizeof(unsigned short);
            
            this->md5mesh[mesh_index].indice.resize(this->md5mesh[mesh_index].n_indice);

            memcpy(&this->md5mesh[mesh_index].indice[0],
                   &this->md5mesh[mesh_index].md5triangle[0],
                   s);
            
            ++mesh_index;
        }
        
        
    next_line:
        
        line = strtok(NULL, "\n");
    }
    
cleanup:
    
    delete m;
}


int MD5::load_action(char *name, char *filename, const bool relative_path)
{
    MEMORY *m = new MEMORY(filename, relative_path);

    if (!m) return -1;

    MD5ACTION *md5action;

    this->md5action.resize(this->md5action.size()+1);

    md5action = &this->md5action[this->md5action.size() - 1];

    memset(md5action, 0, sizeof(MD5ACTION));

    strcpy(md5action->name, name);

    md5action->curr_frame = 0;
    md5action->next_frame = 1;


    char *line = strtok((char *)m->buffer, "\n");

    int int_val = 0;

    while (line) {
        unsigned int    n_frame;
        if (sscanf(line, "MD5Version %d", &int_val) == 1) {
            if (int_val != 10) goto cleanup;
        } else if (sscanf(line, "numFrames %d", &n_frame) == 1) {
            md5action->frame.resize(n_frame);
            //            md5action->frame.resize(0);

            for (int i=0; i != n_frame; ++i) {
                md5action->frame[i] = (MD5JOINT *) malloc(this->bind_pose.size() *
                                                          sizeof(MD5JOINT));
            }
        } else if (sscanf(line, "numJoints %d", &int_val) == 1) {
            if (this->bind_pose.size() != int_val) goto cleanup;

            md5action->pose = (MD5JOINT *) malloc(this->bind_pose.size() *
                                                  sizeof(MD5JOINT));

            for (int i=0; i != this->bind_pose.size(); ++i) {
                strcpy(md5action->pose[i].name, this->bind_pose[i].name);
            }
        } else if (sscanf(line, "frameRate %d", &int_val) == 1) {
            md5action->fps = 1.0f / (float)int_val;
        } else if (sscanf(line, "frame %d", &int_val)) {
            MD5JOINT *md5joint = md5action->frame[int_val];

            line = strtok(NULL, "\n");

            for (int i=0; i != this->bind_pose.size(); ++i) {
                if (sscanf(line,
                           " %f %f %f %f %f %f",
                           &md5joint[i].location.x,
                           &md5joint[i].location.y,
                           &md5joint[i].location.z,
                           &md5joint[i].rotation.x,
                           &md5joint[i].rotation.y,
                           &md5joint[i].rotation.z) == 6) {
                    strcpy(md5joint[i].name, this->bind_pose[i].name);

                    vec4_build_w(&md5joint[i].rotation);
                }

                line = strtok(NULL, "\n");
            }


            vec3 location;

            vec4 rotation;

            for (int i=0; i != this->bind_pose.size(); ++i) {
                if (this->bind_pose[i].parent > -1) {
                    MD5JOINT *md5joint = &md5action->frame[int_val][this->bind_pose[i].parent];

                    vec3_rotate_vec4(&location,
                                     &md5action->frame[int_val][i].location,
                                     &md5joint->rotation);

                    md5action->frame[int_val][i].location.x = location.x + md5joint->location.x;
                    md5action->frame[int_val][i].location.y = location.y + md5joint->location.y;
                    md5action->frame[int_val][i].location.z = location.z + md5joint->location.z;
                    
                    vec4_multiply_vec4(&rotation,
                                       &md5joint->rotation,
                                       &md5action->frame[int_val][i].rotation);
                    
                    vec4_normalize(&md5action->frame[int_val][i].rotation,
                                   &rotation);
                }
            }
        }
        
    next_line:
        
        line = strtok(NULL, "\n");	
    }
    
    delete m;
    
    return (this->md5action.size() - 1);
    
    
cleanup:
    
    
    this->md5action.resize(this->md5action.size()-1);
    delete m;
    
    return -1;
}


MD5::~MD5()
{
    this->free_mesh_data();

    for (auto md5mesh=this->md5mesh.begin();
         md5mesh != this->md5mesh.end(); ++md5mesh) {

        md5mesh->md5vertex.clear();

        if (md5mesh->vertex_data) free(md5mesh->vertex_data);

        md5mesh->md5triangle.clear();

        md5mesh->md5weight.clear();

        if (md5mesh->vbo) glDeleteBuffers(1, &md5mesh->vbo);

        if (md5mesh->vbo_indice) glDeleteBuffers(1, &md5mesh->vbo_indice);

        if (md5mesh->vao) glDeleteVertexArraysOES(1, &md5mesh->vao);
    }

    this->md5mesh.resize(0);


    for (auto md5action=this->md5action.begin();
         md5action != this->md5action.end(); ++md5action) {

        for (auto frame=md5action->frame.begin();
             frame != md5action->frame.end(); ++frame) {
            free(*frame);
        }

        if (md5action->frame.size()) md5action->frame.resize(0);
        
        if (md5action->pose) free(md5action->pose);
    }
    
    this->md5action.resize(0);
}


void MD5::free_mesh_data()
{
    for (auto md5mesh=this->md5mesh.begin();
         md5mesh != this->md5mesh.end(); ++md5mesh) {

        md5mesh->indice.clear();

        md5mesh->md5triangle.clear();
    }
}


MD5ACTION *MD5::get_action(char *name, const bool exact_name)
{
    for (int i=0; i != this->md5action.size(); ++i) {
        if (exact_name) {
            if (!strcmp(this->md5action[i].name, name))
                return &this->md5action[i];
        } else {
            if (strstr(this->md5action[i].name, name))
                return &this->md5action[i];
        }
    }

    return NULL;
}


MD5MESH *MD5::get_mesh(char *name, const bool exact_name)
{
    for (auto md5mesh=this->md5mesh.begin();
         md5mesh != this->md5mesh.end(); ++md5mesh) {
        if (exact_name) {
            if (!strcmp(md5mesh->shader, name))
                return &(*md5mesh);
        } else {
            if (strstr(md5mesh->shader, name))
                return &(*md5mesh);
        }
    }

    return NULL;
}



void MD5ACTION::action_play(const unsigned char frame_interpolation_method,
                            const bool loop)
{
    this->method = frame_interpolation_method;
    this->loop   = loop;
    this->state  = PLAY;

    if (!this->frame_time && frame_interpolation_method == MD5_METHOD_FRAME) {
        this->frame_time =
        this->fps;
    }
}


void MD5ACTION::action_pause()
{
    this->state = PAUSE;
}


void MD5ACTION::action_stop()
{
    this->curr_frame = 0;
    this->next_frame = 1;
    this->state = STOP;
    this->frame_time = 0.0f;
}


void MD5ACTION::set_action_fps(float fps)
{
    this->fps = 1.0f / fps;
}


void MD5MESH::set_mesh_attributes()
{
    glBindBuffer(GL_ARRAY_BUFFER, this->vbo);

    glEnableVertexAttribArray(0);

    glVertexAttribPointer(0,
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          0,
                          (void *)NULL);


    glEnableVertexAttribArray(1);

    glVertexAttribPointer(1,
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          0,
                          BUFFER_OFFSET(this->offset[1]));


    glEnableVertexAttribArray(2);

    glVertexAttribPointer(2,
                          2,
                          GL_FLOAT,
                          GL_FALSE,
                          0,
                          BUFFER_OFFSET(this->offset[2]));


    glEnableVertexAttribArray(3);

    glVertexAttribPointer(3,
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          0,
                          BUFFER_OFFSET(this->offset[3]));
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vbo_indice);
}


void MD5MESH::set_mesh_visibility(const bool visible)
{
    this->visible = visible;
}


void MD5MESH::set_mesh_material(OBJMATERIAL *objmaterial)
{
    this->objmaterial = objmaterial;
}


void MD5::optimize(unsigned int vertex_cache_size)
{
    unsigned int s;

    unsigned short n_group = 0;

    if (vertex_cache_size) SetCacheSize(vertex_cache_size);

    for (auto md5mesh=this->md5mesh.begin(); md5mesh != this->md5mesh.end(); ++md5mesh) {

        PrimitiveGroup *primitivegroup;

        if (GenerateStrips((unsigned short *)&md5mesh->md5triangle[0],
                           md5mesh->n_indice,
                           &primitivegroup,
                           &n_group,
                           true)) {
            if (primitivegroup[0].numIndices < md5mesh->n_indice) {
                md5mesh->mode = GL_TRIANGLE_STRIP;
                md5mesh->n_indice = primitivegroup[0].numIndices;

                s = primitivegroup[0].numIndices * sizeof(unsigned short);

                md5mesh->indice.resize(primitivegroup[0].numIndices);

                memcpy(&md5mesh->indice[0],
                       &primitivegroup[0].indices[0],
                       s);
            }
            
            delete[] primitivegroup;
        }
    }
}


void MD5::build_vbo(unsigned int mesh_index)
{
    MD5MESH *md5mesh = &this->md5mesh[mesh_index];

    md5mesh->size = md5mesh->md5vertex.size() * (sizeof(vec3) +  // Vertex
                                                 sizeof(vec3) +  // Normals
                                                 sizeof(vec2) +  // Texcoord0
                                                 sizeof(vec3));  // Tangent0

    md5mesh->vertex_data = (unsigned char *) calloc(1, md5mesh->size);

    md5mesh->offset[0] = 0;

    md5mesh->offset[1] = md5mesh->md5vertex.size() * sizeof(vec3);

    md5mesh->offset[2] =   md5mesh->offset[1] +
    (md5mesh->md5vertex.size() * sizeof(vec3));

    md5mesh->offset[3] =   md5mesh->offset[2] +
    (md5mesh->md5vertex.size() * sizeof(vec2));


    glGenBuffers(1, &md5mesh->vbo);

    glBindBuffer(GL_ARRAY_BUFFER, md5mesh->vbo);

    glBufferData(GL_ARRAY_BUFFER,
                 md5mesh->size,
                 md5mesh->vertex_data,
                 GL_DYNAMIC_DRAW);


    glGenBuffers(1, &md5mesh->vbo_indice);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, md5mesh->vbo_indice);

    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 md5mesh->n_indice * sizeof(unsigned short),
                 &md5mesh->indice[0],
                 GL_STATIC_DRAW);
}

void MD5MESH::build_vbo()
{
    this->size = this->md5vertex.size() * (sizeof(vec3) +  // Vertex
                                           sizeof(vec3) +  // Normals
                                           sizeof(vec2) +  // Texcoord0
                                           sizeof(vec3));  // Tangent0

    this->vertex_data = (unsigned char *) calloc(1, this->size);

    this->offset[0] = 0;

    this->offset[1] = this->md5vertex.size() * sizeof(vec3);

    this->offset[2] = this->offset[1] + (this->md5vertex.size() * sizeof(vec3));

    this->offset[3] = this->offset[2] + (this->md5vertex.size() * sizeof(vec2));


    glGenBuffers(1, &this->vbo);

    glBindBuffer(GL_ARRAY_BUFFER, this->vbo);

    glBufferData(GL_ARRAY_BUFFER,
                 this->size,
                 this->vertex_data,
                 GL_DYNAMIC_DRAW);


    glGenBuffers(1, &this->vbo_indice);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vbo_indice);

    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 this->n_indice * sizeof(unsigned short),
                 &this->indice[0],
                 GL_STATIC_DRAW);
}

void MD5::build_bind_pose_weighted_normals_tangents()
{
    for (auto md5mesh=this->md5mesh.begin(); md5mesh != this->md5mesh.end(); ++md5mesh) {

        vec3 *vertex_array = (vec3 *)md5mesh->vertex_data;

        for (auto md5vertex=md5mesh->md5vertex.begin();
             md5vertex!=md5mesh->md5vertex.end(); ++md5vertex) {
            memset(&md5vertex->normal,
                   0,
                   sizeof(vec3));

            memset(&md5vertex->tangent,
                   0,
                   sizeof(vec3));
        }

        for (int j=0; j != md5mesh->md5triangle.size(); ++j) {
            auto indice = md5mesh->md5triangle[j].indice;

            vec3    v1,
                    v2,
                    normal;

            vec3_diff(&v1,
                      &vertex_array[indice[0]],
                      &vertex_array[indice[1]]);

            vec3_diff(&v2,
                      &vertex_array[indice[0]],
                      &vertex_array[indice[2]]);


            vec3_cross(&normal, &v1, &v2);

            vec3_normalize(&normal, &normal);

            // Flat normals
            /*
             memcpy(&md5mesh->md5vertex[md5triangle->indice[0]].normal,
             &normal,
             sizeof(vec3));

             memcpy(&md5mesh->md5vertex[md5triangle->indice[1]].normal,
             &normal,
             sizeof(vec3));

             memcpy(&md5mesh->md5vertex[md5triangle->indice[2]].normal,
             &normal,
             sizeof(vec3));
             */

            // Smooth normals
            vec3_add(&md5mesh->md5vertex[indice[0]].normal,
                     &md5mesh->md5vertex[indice[0]].normal,
                     &normal);

            vec3_add(&md5mesh->md5vertex[indice[1]].normal,
                     &md5mesh->md5vertex[indice[1]].normal,
                     &normal);

            vec3_add(&md5mesh->md5vertex[indice[2]].normal,
                     &md5mesh->md5vertex[indice[2]].normal,
                     &normal);

            vec3 tangent;

            vec2 uv1, uv2;

            float c;

            vec2_diff(&uv1,
                      &md5mesh->md5vertex[indice[1]].uv,
                      &md5mesh->md5vertex[indice[0]].uv);

            vec2_diff(&uv2,
                      &md5mesh->md5vertex[indice[2]].uv,
                      &md5mesh->md5vertex[indice[0]].uv);


            c = 1.0f / (uv1.x * uv2.y - uv2.x * uv1.y);

            tangent.x = (v1.x * uv2.y + v2.x * uv1.y) * c;
            tangent.y = (v1.y * uv2.y + v2.y * uv1.y) * c;
            tangent.z = (v1.z * uv2.y + v2.z * uv1.y) * c;


            vec3_add(&md5mesh->md5vertex[indice[0]].tangent,
                     &md5mesh->md5vertex[indice[0]].tangent,
                     &tangent);

            vec3_add(&md5mesh->md5vertex[indice[1]].tangent,
                     &md5mesh->md5vertex[indice[1]].tangent,
                     &tangent);

            vec3_add(&md5mesh->md5vertex[indice[2]].tangent,
                     &md5mesh->md5vertex[indice[2]].tangent,
                     &tangent);
        }


        for (int j=0; j != md5mesh->md5vertex.size(); ++j) {
            auto normal = md5mesh->md5vertex[j].normal;
            auto tangent = md5mesh->md5vertex[j].tangent;

            // Average normals
            vec3_normalize(&normal, &normal);

            vec3_normalize(&tangent, &tangent);
        }


        for (auto md5weight=md5mesh->md5weight.begin();
             md5weight!=md5mesh->md5weight.end(); ++md5weight) {
            memset(&md5weight->normal,
                   0,
                   sizeof(vec3));

            memset(&md5weight->tangent,
                   0,
                   sizeof(vec3));
        }


        for (auto md5vertex=md5mesh->md5vertex.begin();
             md5vertex!=md5mesh->md5vertex.end(); ++md5vertex) {

            for (int k=0; k != md5vertex->count; ++k) {
                MD5WEIGHT *md5weight = &md5mesh->md5weight[md5vertex->start + k];

                MD5JOINT *md5joint = &this->bind_pose[md5weight->joint];

                vec3 normal = { md5vertex->normal.x,
                    md5vertex->normal.y,
                    md5vertex->normal.z },

                tangent = { md5vertex->tangent.x,
                    md5vertex->tangent.y,
                    md5vertex->tangent.z };
                
                vec4 rotation = { md5joint->rotation.x,
                    md5joint->rotation.y,
                    md5joint->rotation.z,
                    md5joint->rotation.w };
                
                
                vec4_conjugate(&rotation, &rotation);
                
                
                vec3_rotate_vec4(&normal,
                                 &normal,
                                 &rotation);
                
                vec3_rotate_vec4(&tangent,
                                 &tangent,
                                 &rotation);
                
                
                vec3_add(&md5weight->normal,
                         &md5weight->normal,
                         &normal);
                
                vec3_add(&md5weight->tangent,
                         &md5weight->tangent,
                         &tangent);
            }
        }
        
        
        for (auto md5weight=md5mesh->md5weight.begin();
             md5weight!=md5mesh->md5weight.end(); ++md5weight) {
            vec3_normalize(&md5weight->normal,
                           &md5weight->normal);
            
            vec3_normalize(&md5weight->tangent,
                           &md5weight->tangent);
        }
    }
}


void MD5::set_pose(MD5JOINT *pose)
{
    for (auto md5mesh=this->md5mesh.begin(); md5mesh != this->md5mesh.end(); ++md5mesh) {
        vec3    *vertex_array  = (vec3 *)md5mesh->vertex_data,
                *normal_array  = (vec3 *)&md5mesh->vertex_data[md5mesh->offset[1]],
                *tangent_array = (vec3 *)&md5mesh->vertex_data[md5mesh->offset[3]];

        vec2 *uv_array = (vec2 *)&md5mesh->vertex_data[md5mesh->offset[2]];

        memset(vertex_array , 0, md5mesh->offset[1]);
        memset(normal_array , 0, md5mesh->offset[1]);
        memset(tangent_array, 0, md5mesh->offset[1]);


        for (int j=0; j != md5mesh->md5vertex.size(); ++j) {
            MD5VERTEX *md5vertex = &md5mesh->md5vertex[j];

            for (int k=0; k != md5vertex->count; ++k) {
                vec3 location = { 0.0f, 0.0f, 0.0f },
                normal   = { 0.0f, 0.0f, 0.0f },
                tangent  = { 0.0f, 0.0f, 0.0f };


                MD5WEIGHT *md5weight = &md5mesh->md5weight[md5vertex->start + k];

                MD5JOINT *md5joint = &pose[md5weight->joint];

                vec3_rotate_vec4(&location,
                                 &md5weight->location,
                                 &md5joint->rotation);

                vec3_rotate_vec4(&normal,
                                 &md5weight->normal,
                                 &md5joint->rotation);

                vec3_rotate_vec4(&tangent,
                                 &md5weight->tangent,
                                 &md5joint->rotation);

                vertex_array[j].x += (md5joint->location.x + location.x) * md5weight->bias;
                vertex_array[j].y += (md5joint->location.y + location.y) * md5weight->bias;
                vertex_array[j].z += (md5joint->location.z + location.z) * md5weight->bias;

                normal_array[j].x += normal.x * md5weight->bias;
                normal_array[j].y += normal.y * md5weight->bias;
                normal_array[j].z += normal.z * md5weight->bias;

                tangent_array[j].x += tangent.x * md5weight->bias;
                tangent_array[j].y += tangent.y * md5weight->bias;
                tangent_array[j].z += tangent.z * md5weight->bias;
            }
            
            uv_array[j].x = md5vertex->uv.x;
            uv_array[j].y = md5vertex->uv.y;
        }
        
        
        glBindBuffer(GL_ARRAY_BUFFER, md5mesh->vbo);
        
        glBufferSubData(GL_ARRAY_BUFFER,
                        0,
                        md5mesh->size,
                        md5mesh->vertex_data);		
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void MD5::blend_pose(MD5JOINT *final_pose, MD5JOINT *pose0, MD5JOINT *pose1,
                     unsigned char joint_interpolation_method, float blend)
{
    for (int i=0; i != this->bind_pose.size(); ++i) {
        vec3_lerp(&final_pose[i].location,
                  &pose0[i].location,
                  &pose1[i].location,
                  blend);

        switch(joint_interpolation_method) {
            case MD5_METHOD_FRAME:
            case MD5_METHOD_LERP:
            {
                vec4_lerp(&final_pose[i].rotation,
                          &pose0[i].rotation,
                          &pose1[i].rotation,
                          blend);
                break;
            }


            case MD5_METHOD_SLERP:
            {
                vec4_slerp(&final_pose[i].rotation,
                           &pose0[i].rotation,
                           &pose1[i].rotation,
                           blend);
                
                break;
            }
        }
    }
}


void MD5::add_pose(MD5JOINT *final_pose, MD5ACTION *action0,
                   MD5ACTION *action1,
                   unsigned char joint_interpolation_method,
                   float action_weight)
{
    for (int i=0; i != this->bind_pose.size(); ++i) {
        if (memcmp(&action1->frame[action1->curr_frame][i].location, &action1->frame[action1->next_frame][i].location, sizeof(vec3)) ||
            memcmp(&action1->frame[action1->curr_frame][i].rotation, &action1->frame[action1->next_frame][i].rotation, sizeof(vec4)))
        {
            vec3_lerp(&final_pose[i].location,
                      &action0->pose[i].location,
                      &action1->pose[i].location,
                      action_weight);

            switch(joint_interpolation_method)
            {
                case MD5_METHOD_FRAME:
                case MD5_METHOD_LERP:
                {
                    vec4_lerp(&final_pose[i].rotation,
                              &action0->pose[i].rotation,
                              &action1->pose[i].rotation,
                              action_weight);
                    break;
                }


                case MD5_METHOD_SLERP:
                {
                    vec4_slerp(&final_pose[i].rotation,
                               &action0->pose[i].rotation,
                               &action1->pose[i].rotation,
                               action_weight);
                    break;
                }
            }
        }
        else
        {
            memcpy(&final_pose[i].location, &action0->pose[i].location, sizeof(vec3));
            memcpy(&final_pose[i].rotation, &action0->pose[i].rotation, sizeof(vec4));
        }
    }
}


void MD5::update_bound_mesh()
{
    // Get the mesh min and max.
    this->min.x =
    this->min.y =
    this->min.z = FLT_MAX;

    this->max.x =
    this->max.y =
    this->max.z = -FLT_MAX;


    for (int i=0; i != this->md5mesh.size(); ++i) {
        MD5MESH *md5mesh = &this->md5mesh[i];

        vec3 *vertex_array = (vec3 *)md5mesh->vertex_data;

        for (int j=0; j != md5mesh->md5vertex.size(); ++j) {
            if (vertex_array[j].x < this->min.x) this->min.x = vertex_array[j].x;
            if (vertex_array[j].y < this->min.y) this->min.y = vertex_array[j].y;
            if (vertex_array[j].z < this->min.z) this->min.z = vertex_array[j].z;

            if (vertex_array[j].x > this->max.x) this->max.x = vertex_array[j].x;
            if (vertex_array[j].y > this->max.y) this->max.y = vertex_array[j].y;
            if (vertex_array[j].z > this->max.z) this->max.z = vertex_array[j].z;
        }
    }


    // Mesh dimension
    vec3_diff(&this->dimension,
              &this->max,
              &this->min);
    
    
    // Bounding sphere radius
    this->radius = vec3_dist(&this->min,
                             &this->max) * 0.5f;
}


void MD5::build()
{
    for (auto md5mesh=this->md5mesh.begin();
         md5mesh != this->md5mesh.end(); ++md5mesh) {

        md5mesh->build_vbo();

        glGenVertexArraysOES(1, &md5mesh->vao);

        glBindVertexArrayOES(md5mesh->vao);

        md5mesh->set_mesh_attributes();

        glBindVertexArrayOES(0);
    }

    this->set_pose(&this->bind_pose[0]);

    this->build_bind_pose_weighted_normals_tangents();
    
    this->set_pose(&this->bind_pose[0]);
    
    this->update_bound_mesh();
}


void MD5::build2()
{
    for (auto md5mesh=this->md5mesh.begin();
         md5mesh != this->md5mesh.end(); ++md5mesh)
        md5mesh->build_vbo();

    this->set_pose(&this->bind_pose[0]);

    this->build_bind_pose_weighted_normals_tangents();

    this->set_pose(&this->bind_pose[0]);

    this->update_bound_mesh();
}


bool MD5::draw_action(float time_step)
{
    bool    update = false;

    for (auto md5action = this->md5action.begin();
         md5action != this->md5action.end(); ++md5action) {

        if (md5action->state == PLAY) {
            md5action->frame_time += time_step;

            switch(md5action->method)
            {
                case MD5_METHOD_FRAME:
                {
                    if (md5action->frame_time >= md5action->fps) {
                        memcpy(md5action->pose,
                               md5action->frame[md5action->curr_frame],
                               this->bind_pose.size() * sizeof(MD5JOINT));

                        ++md5action->curr_frame;

                        if (md5action->curr_frame == md5action->frame.size()) {
                            if (md5action->loop)
                                md5action->curr_frame = 0;
                            else {
                                md5action->action_stop();
                                break;
                            }
                        }

                        md5action->next_frame = md5action->curr_frame + 1;

                        if (md5action->next_frame == md5action->frame.size()) {
                            md5action->next_frame = 0;
                        }

                        md5action->frame_time -= md5action->fps;

                        update = true;
                    }

                    break;
                }

                case MD5_METHOD_LERP:
                case MD5_METHOD_SLERP:
                {
                    float t = CLAMP(md5action->frame_time / md5action->fps, 0.0f, 1.0f);

                    this->blend_pose(md5action->pose,
                                     md5action->frame[md5action->curr_frame],
                                     md5action->frame[md5action->next_frame],
                                     md5action->method,
                                     t);

                    if (t >= 1.0f) {
                        ++md5action->curr_frame;
                        
                        md5action->next_frame = (md5action->curr_frame + 1);
                        
                        if (md5action->loop) {
                            if (md5action->curr_frame == md5action->frame.size()) {
                                md5action->curr_frame = 0;
                                md5action->next_frame = 1;
                            }								
                            
                            if (md5action->next_frame == md5action->frame.size()) {
                                md5action->next_frame = 0;
                            }
                        } else {
                            if (md5action->next_frame == md5action->frame.size()) {
                                md5action->action_stop();
                                break;
                            }
                        }
                        
                        md5action->frame_time -= md5action->fps;
                    }
                    
                    update = true;
                    
                    break;
                }
            }
        }
    }
    
    return update;
}


void MD5::draw()
{
    if (this->visible && this->distance) {
        for (auto md5mesh=this->md5mesh.begin();
             md5mesh != this->md5mesh.end(); ++md5mesh) {

            if (md5mesh->visible) {
                if (md5mesh->objmaterial)
                    md5mesh->objmaterial->draw();

                if (md5mesh->vao)
                    glBindVertexArrayOES(md5mesh->vao);
                else
                    md5mesh->set_mesh_attributes();

                glDrawElements(md5mesh->mode,
                               md5mesh->n_indice,
                               GL_UNSIGNED_SHORT,
                               (void *)NULL);
            }
        }
    }	
}
