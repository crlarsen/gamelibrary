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


static void vec3_rotate_quat(vec3 &dst, const vec3 &v0, const quaternion &v1)
{
    float   w(-v1.v.dotProduct(v0));
    vec3    v(v1.w*v0 + v1.v.crossProduct(v0));

    dst = v*v1.w - w*v1.v + v1.v.crossProduct(v);
}


static void quat_build_r(quaternion &q)
{
    float l = 1.0f - (q->i * q->i) -
    (q->j * q->j) -
    (q->k * q->k);

    q->r = ( l < 0.0f ) ? 0.0f : -sqrtf( l );
}


MD5::MD5(char *filename, const bool relative_path) :
    visible(true), location(0,0,0), rotation(0,0,0), scale(1,1,1),
    min(FLT_MAX,FLT_MAX,FLT_MAX), max(-FLT_MAX,-FLT_MAX,-FLT_MAX),
    dimension(0,0,0), radius(0.0f), distance(1.0f), btrigidbody(NULL)
{
    MEMORY  *m = new MEMORY(filename, relative_path);

    if (!m) return;

    get_file_name(filename, this->name);

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
        } else if (!strncmp(line, "joints {", 8)) {
            unsigned int i = 0;

            line = strtok(NULL, "\n");

            while (line[0] != '}') {
                if (sscanf(line,
                           "%s %d ( %f %f %f ) ( %f %f %f )",
                            this->bind_pose[i].name,
                           &this->bind_pose[i].parent,
                           &this->bind_pose[i].location->x,
                           &this->bind_pose[i].location->y,
                           &this->bind_pose[i].location->z,
                           &this->bind_pose[i].rotation->i,
                           &this->bind_pose[i].rotation->j,
                           &this->bind_pose[i].rotation->k) == 8) {
                    quat_build_r(this->bind_pose[i].rotation);

                    ++i;
                }

                line = strtok(NULL, "\n");
            }
        } else if (!strncmp(line, "mesh {", 6)) {
            MD5VERTEX md5vertex;

            MD5TRIANGLE md5triangle;

            static MD5WEIGHT md5weight;

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
                                  &md5vertex.uv->x,
                                  &md5vertex.uv->y,
                                  &md5vertex.start,
                                  &md5vertex.count) == 5) {
                    this->md5mesh[mesh_index].md5vertex[int_val] = md5vertex;
                } else if (sscanf(line, " numtris %d", &n_triangle) == 1) {
                    this->md5mesh[mesh_index].n_indice = n_triangle * 3;

                    this->md5mesh[mesh_index].md5triangle.resize(n_triangle);
                } else if (sscanf(line,
                                  " tri %d %hu %hu %hu",
                                  &int_val,
                                  &md5triangle.indice[2],
                                  &md5triangle.indice[1],
                                  &md5triangle.indice[0]) == 4) {
                    this->md5mesh[mesh_index].md5triangle[int_val] = md5triangle;
                } else if (sscanf(line, " numweights %d", &n_weight) == 1) {
                    this->md5mesh[mesh_index].md5weight.resize(n_weight);
                } else if (sscanf(line,
                                  " weight %d %d %f ( %f %f %f )",
                                  &int_val,
                                  &md5weight.joint,
                                  &md5weight.bias,
                                  &md5weight.location->x,
                                  &md5weight.location->y,
                                  &md5weight.location->z) == 6) {
                    this->md5mesh[mesh_index].md5weight[int_val] = md5weight;

                }

            next_mesh_line:
                line = strtok(NULL, "\n");
            }

            // The code doesn't need to resize indice to 0 (zero) because
            // the constructor for MD5MESH already created it with 0
            // elements.  Here the code uses reserve() to short circuit
            // the need to do multiple reallocations, copies, and frees.
            this->md5mesh[mesh_index].indice.reserve(this->md5mesh[mesh_index].n_indice);

            auto    &md5mesh = this->md5mesh[mesh_index];
            for (int j=0; j!=md5mesh.md5triangle.size(); ++j) {
                // CRL -- When this code is converted to C++ 11 and
                // md5triangle.indice is declared as "array<unsigned short,3>"
                // investigate replacing this
                md5mesh.indice.push_back(md5mesh.md5triangle[j].indice[0]);
                md5mesh.indice.push_back(md5mesh.md5triangle[j].indice[1]);
                md5mesh.indice.push_back(md5mesh.md5triangle[j].indice[2]);
                // with
                //md5mesh.indice.insert(md5mesh.indice.end(),
                //                      md5mesh.md5triangle[j].begin(),
                //                      md5mesh.md5triangle[j].end());
            }
            
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

    this->md5action.push_back(MD5ACTION(name));

    md5action = &this->md5action.back();


    char *line = strtok((char *)m->buffer, "\n");

    int int_val = 0;

    while (line) {
        unsigned int    n_frame;
        if (sscanf(line, "MD5Version %d", &int_val) == 1) {
            if (int_val != 10) goto cleanup;
        } else if (sscanf(line, "numFrames %d", &n_frame) == 1) {
            md5action->frame.resize(n_frame);

            for (int i=0; i != n_frame; ++i)
                md5action->frame[i].resize(this->bind_pose.size());
        } else if (sscanf(line, "numJoints %d", &int_val) == 1) {
            if (this->bind_pose.size() != int_val) goto cleanup;

            md5action->pose.resize(this->bind_pose.size());

            for (int i=0; i != this->bind_pose.size(); ++i)
                strcpy(md5action->pose[i].name, this->bind_pose[i].name);
        } else if (sscanf(line, "frameRate %d", &int_val) == 1) {
            md5action->fps = 1.0f / (float)int_val;
        } else if (sscanf(line, "frame %d", &int_val)) {
            auto &md5joint = md5action->frame[int_val];

            line = strtok(NULL, "\n");

            for (int i=0; i != this->bind_pose.size(); ++i) {
                if (sscanf(line,
                           " %f %f %f %f %f %f",
                           &md5joint[i].location->x,
                           &md5joint[i].location->y,
                           &md5joint[i].location->z,
                           &md5joint[i].rotation->i,
                           &md5joint[i].rotation->j,
                           &md5joint[i].rotation->k) == 6) {
                    strcpy(md5joint[i].name, this->bind_pose[i].name);

                    quat_build_r(md5joint[i].rotation);
                }

                line = strtok(NULL, "\n");
            }


            vec3 location;

            quaternion rotation;

            for (int i=0; i != this->bind_pose.size(); ++i) {
                if (this->bind_pose[i].parent > -1) {
                    MD5JOINT *md5joint = &md5action->frame[int_val][this->bind_pose[i].parent];

                    vec3_rotate_quat(location,
                                     md5action->frame[int_val][i].location,
                                     md5joint->rotation);

                    md5action->frame[int_val][i].location = location + md5joint->location;
                    
                    md5action->frame[int_val][i].rotation =
                        (md5joint->rotation * md5action->frame[int_val][i].rotation).normalize();
                }
            }
        }
        
    next_line:
        
        line = strtok(NULL, "\n");	
    }
    
    delete m;
    
    return (this->md5action.size() - 1);
    
    
cleanup:
    
    
    this->md5action.resize(this->md5action.size() - 1);
    delete m;
    
    return -1;
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
    if (exact_name) {
        for (int i=0; i != this->md5action.size(); ++i) {
            if (!strcmp(this->md5action[i].name, name))
                return &this->md5action[i];
        }
    } else {
        for (int i=0; i != this->md5action.size(); ++i) {
            if (strstr(this->md5action[i].name, name))
                return &this->md5action[i];
        }
    }

    return NULL;
}


MD5MESH *MD5::get_mesh(char *name, const bool exact_name)
{
    if (exact_name) {
        for (auto md5mesh=this->md5mesh.begin();
             md5mesh != this->md5mesh.end(); ++md5mesh) {
            if (!strcmp(md5mesh->shader, name))
                return &(*md5mesh);
        }
    } else {
        for (auto md5mesh=this->md5mesh.begin();
             md5mesh != this->md5mesh.end(); ++md5mesh) {
            if (strstr(md5mesh->shader, name))
                return &(*md5mesh);
        }
    }

    return NULL;
}


MD5ACTION::MD5ACTION(const char *name) : curr_frame(0), next_frame(1),
                                         state(STOP), method(MD5_METHOD_FRAME),
                                         loop(false), frame_time(0), fps(0)
{
    assert(name==NULL || strlen(name)<sizeof(this->name));
    strcpy(this->name, name ? name : "");
}

void MD5ACTION::action_play(const MD5Method frame_interpolation_method,
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

MD5MESH::MD5MESH(const char *name) :
    vbo(0), size(0), stride(0), vertex_data(NULL),
    mode(GL_TRIANGLES), n_indice(0), vbo_indice(0),
    vao(0), visible(true), objmaterial(NULL)
{
    assert(name==NULL || strlen(name)<sizeof(shader));
    strcpy(shader, name ? name : "");
}

MD5MESH::MD5MESH(const MD5MESH &src) :
    md5vertex(src.md5vertex), vbo(src.vbo), size(src.size),
    stride(src.stride), md5triangle(src.md5triangle), mode(src.mode),
    n_indice(src.n_indice), indice(src.indice),
    vbo_indice(src.vbo_indice), md5weight(src.md5weight), vao(src.vao),
    visible(src.visible), objmaterial(src.objmaterial)
{
    strcpy(shader, src.shader);

    vertex_data = (unsigned char *) calloc(1, size);
    memcpy(vertex_data, src.vertex_data, size);

    memcpy(offset, src.offset, sizeof(offset));
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

void MD5MESH::build_vao()
{
    glGenVertexArraysOES(1, &this->vao);

    glBindVertexArrayOES(this->vao);

    this->set_mesh_attributes();

    glBindVertexArrayOES(0);
}

void MD5::build_bind_pose_weighted_normals_tangents()
{
    static vec3 zero(0, 0, 0);

    for (auto md5mesh=this->md5mesh.begin();
         md5mesh != this->md5mesh.end(); ++md5mesh) {

        vec3 *vertex_array = (vec3 *)md5mesh->vertex_data;

        for (auto md5vertex=md5mesh->md5vertex.begin();
             md5vertex!=md5mesh->md5vertex.end(); ++md5vertex) {
            md5vertex->normal  =
            md5vertex->tangent = zero;
        }

        for (auto md5triangle=md5mesh->md5triangle.begin();
             md5triangle!=md5mesh->md5triangle.end(); ++md5triangle) {
            auto &indice = md5triangle->indice;

            vec3    v1,
                    v2,
                    normal;

            v1 = vertex_array[indice[0]] - vertex_array[indice[1]];

            v2 = vertex_array[indice[0]] - vertex_array[indice[2]];


            normal = v1.crossProduct(v2);

            normal.safeNormalize();

            // Flat normals
            /*
             md5mesh->md5vertex[md5triangle->indice[0]].normal =
             md5mesh->md5vertex[md5triangle->indice[1]].normal =
             md5mesh->md5vertex[md5triangle->indice[2]].normal = normal;
             */

            // Smooth normals
            md5mesh->md5vertex[indice[0]].normal += normal;

            md5mesh->md5vertex[indice[1]].normal += normal;

            md5mesh->md5vertex[indice[2]].normal += normal;

            vec3 tangent;

            vec2 uv1(md5mesh->md5vertex[indice[1]].uv);
            uv1 -= md5mesh->md5vertex[indice[0]].uv;

            vec2 uv2(md5mesh->md5vertex[indice[2]].uv);
            uv2 -= md5mesh->md5vertex[indice[0]].uv;

            float c = 1.0f / (uv1->x * uv2->y - uv2->x * uv1->y);

            tangent = (v1 * uv2->y + v2 * uv1->y) * c;


            md5mesh->md5vertex[indice[0]].tangent += tangent;

            md5mesh->md5vertex[indice[1]].tangent += tangent;

            md5mesh->md5vertex[indice[2]].tangent += tangent;
        }


        for (auto md5vertex=md5mesh->md5vertex.begin();
             md5vertex!=md5mesh->md5vertex.end(); ++md5vertex) {
            // Average normals
            md5vertex->normal.safeNormalize();

            md5vertex->tangent.safeNormalize();
        }

        for (auto md5weight=md5mesh->md5weight.begin();
             md5weight!=md5mesh->md5weight.end(); ++md5weight) {
            md5weight->normal  =
            md5weight->tangent = zero;
        }


        for (auto md5vertex=md5mesh->md5vertex.begin();
             md5vertex!=md5mesh->md5vertex.end(); ++md5vertex) {

            for (int k=0; k != md5vertex->count; ++k) {
                MD5WEIGHT &md5weight = md5mesh->md5weight[md5vertex->start + k];

                MD5JOINT &md5joint = this->bind_pose[md5weight.joint];

                vec3    normal(md5vertex->normal),
                        tangent(md5vertex->tangent);
                
                quaternion    rotation(md5joint.rotation.conjugate());
                
                
                vec3_rotate_quat(normal,
                                 normal,
                                 rotation);
                
                vec3_rotate_quat(tangent,
                                 tangent,
                                 rotation);
                
                
                md5weight.normal += normal;

                md5weight.tangent += tangent;
            }
        }
        
        
        for (auto md5weight=md5mesh->md5weight.begin();
             md5weight!=md5mesh->md5weight.end(); ++md5weight) {
            md5weight->normal.safeNormalize();
            
            md5weight->tangent.safeNormalize();
        }
    }
}


void MD5::set_pose(std::vector<MD5JOINT> &pose)
{
    for (auto md5mesh=this->md5mesh.begin(); md5mesh != this->md5mesh.end(); ++md5mesh) {
        vec3    *vertex_array  = (vec3 *)md5mesh->vertex_data,
                *normal_array  = (vec3 *)&md5mesh->vertex_data[md5mesh->offset[1]],
                *tangent_array = (vec3 *)&md5mesh->vertex_data[md5mesh->offset[3]];

        vec2    *uv_array      = (vec2 *)&md5mesh->vertex_data[md5mesh->offset[2]];

        memset(vertex_array,  0, md5mesh->offset[1]);
        memset(normal_array,  0, md5mesh->offset[1]);
        memset(tangent_array, 0, md5mesh->offset[1]);


        for (int j=0; j != md5mesh->md5vertex.size(); ++j) {
            MD5VERTEX *md5vertex = &md5mesh->md5vertex[j];

            for (int k=0; k != md5vertex->count; ++k) {
                vec3    location(0.0f, 0.0f, 0.0f),
                        normal  (0.0f, 0.0f, 0.0f),
                        tangent (0.0f, 0.0f, 0.0f);


                MD5WEIGHT *md5weight = &md5mesh->md5weight[md5vertex->start + k];

                MD5JOINT *md5joint = &pose[md5weight->joint];

                vec3_rotate_quat(location,
                                 md5weight->location,
                                 md5joint->rotation);

                vec3_rotate_quat(normal,
                                 md5weight->normal,
                                 md5joint->rotation);

                vec3_rotate_quat(tangent,
                                 md5weight->tangent,
                                 md5joint->rotation);

                vertex_array[j] += (md5joint->location + location) * md5weight->bias;

                normal_array[j] += normal * md5weight->bias;

                tangent_array[j] += tangent * md5weight->bias;
            }
            
            uv_array[j] = md5vertex->uv;
        }
        
        
        glBindBuffer(GL_ARRAY_BUFFER, md5mesh->vbo);
        
        glBufferSubData(GL_ARRAY_BUFFER,
                        0,
                        md5mesh->size,
                        md5mesh->vertex_data);		
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void MD5::blend_pose(std::vector<MD5JOINT> &final_pose,
                     const std::vector<MD5JOINT> &pose0,
                     const std::vector<MD5JOINT> &pose1,
                     const MD5Method joint_interpolation_method,
                     const float blend)
{
    for (int i=0; i != this->bind_pose.size(); ++i) {
        final_pose[i].location = linterp(pose0[i].location,
                                         pose1[i].location,
                                         blend);

        switch(joint_interpolation_method) {
            case MD5_METHOD_FRAME:
            case MD5_METHOD_LERP:
            {
                final_pose[i].rotation = rinterp(pose0[i].rotation,
                                                 pose1[i].rotation,
                                                 blend);
                break;
            }


            case MD5_METHOD_SLERP:
            {
                final_pose[i].rotation = slerp(pose0[i].rotation,
                                               pose1[i].rotation,
                                               blend);

                break;
            }
        }
    }
}


void MD5::add_pose(std::vector<MD5JOINT> &final_pose,
                   const MD5ACTION &action0,
                   const MD5ACTION &action1,
                   const MD5Method joint_interpolation_method,
                   const float action_weight)
{
    for (int i=0; i != this->bind_pose.size(); ++i) {
        if ((action1.frame[action1.curr_frame][i].location != action1.frame[action1.next_frame][i].location) ||
            (action1.frame[action1.curr_frame][i].rotation != action1.frame[action1.next_frame][i].rotation))
        {
            final_pose[i].location = linterp(action0.pose[i].location,
                                             action1.pose[i].location,
                                             action_weight);

            switch(joint_interpolation_method)
            {
                case MD5_METHOD_FRAME:
                case MD5_METHOD_LERP:
                {
                    final_pose[i].rotation = rinterp(action0.pose[i].rotation,
                                                     action1.pose[i].rotation,
                                                     action_weight);
                    break;
                }


                case MD5_METHOD_SLERP:
                {
                    final_pose[i].rotation = slerp(action0.pose[i].rotation,
                                                   action1.pose[i].rotation,
                                                   action_weight);
                    break;
                }
            }
        } else {
            final_pose[i].location = action0.pose[i].location;
            final_pose[i].rotation = action0.pose[i].rotation;
        }
    }
}


void MD5::update_bound_mesh()
{
    // Get the mesh min and max.

    // min, and max vectors are already set using FLT_MAX, or -FLT_MAX
    // by the constructor


    for (int i=0; i != this->md5mesh.size(); ++i) {
        MD5MESH *md5mesh = &this->md5mesh[i];

        vec3 *vertex_array = (vec3 *)md5mesh->vertex_data;

        for (int j=0; j != md5mesh->md5vertex.size(); ++j) {
            if (vertex_array[j]->x < this->min->x) this->min->x = vertex_array[j]->x;
            if (vertex_array[j]->y < this->min->y) this->min->y = vertex_array[j]->y;
            if (vertex_array[j]->z < this->min->z) this->min->z = vertex_array[j]->z;

            if (vertex_array[j]->x > this->max->x) this->max->x = vertex_array[j]->x;
            if (vertex_array[j]->y > this->max->y) this->max->y = vertex_array[j]->y;
            if (vertex_array[j]->z > this->max->z) this->max->z = vertex_array[j]->z;
        }
    }


    // Mesh dimension
    this->dimension = this->max - this->min;
    
    
    // Bounding sphere radius
    this->radius = (this->max - this->min).length() * 0.5f;
}


void MD5::build()
{
    for (auto md5mesh=this->md5mesh.begin();
         md5mesh!=this->md5mesh.end(); ++md5mesh) {
        md5mesh->build_vbo();
        
        md5mesh->build_vao();
    }

    this->set_pose(this->bind_pose);

    this->build_bind_pose_weighted_normals_tangents();
    
    this->set_pose(this->bind_pose);
    
    this->update_bound_mesh();
}


void MD5::build2()
{
    for (auto md5mesh=this->md5mesh.begin();
         md5mesh != this->md5mesh.end(); ++md5mesh)
        md5mesh->build_vbo();

    this->set_pose(this->bind_pose);

    this->build_bind_pose_weighted_normals_tangents();

    this->set_pose(this->bind_pose);

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
                        md5action->pose =
                            md5action->frame[md5action->curr_frame];

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
