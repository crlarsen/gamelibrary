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
 * - OBJ
 * - OBJMATERIAL
 * - OBJMESH
 * - OBJTRIANGLEINDEX
 * - OBJTRIANGLELIST
 * - OBJVERTEXDATA
 * - PROGRAM
 * - SHADER
 */

#include "gfx.h"

OBJTRIANGLELIST::OBJTRIANGLELIST() :
    useuvs(false),
    n_indice_array(0),
    indice_array(NULL),
    objmaterial(NULL),
    mode(0),
    vbo(0)
{
}

OBJTRIANGLELIST::OBJTRIANGLELIST(int mode, bool useuvs,
                                 OBJMATERIAL *objmaterial) :
    useuvs(useuvs),
    n_indice_array(0),
    indice_array(NULL),
    objmaterial(objmaterial),
    mode(mode),
    vbo(0)
{
}

OBJVERTEXDATA::OBJVERTEXDATA(const int vi, const int uvi) :
    vertex_index(vi),
    uv_index(uvi)
{
}

OBJMESH::OBJMESH(OBJ *parent) : name(""), visible(false), group(""),
                                current_material(NULL), radius(0.0f),
                                distance(0.0f), vbo(0), stride(0),
                                size(0), vao(0), btrigidbody(NULL),
                                use_smooth_normals(false), parent(parent)
{
    memset(&location, 0, sizeof(vec3));
    memset(&rotation, 0, sizeof(vec3));
    memset(&scale, 0, sizeof(vec3));
    memset(&min, 0, sizeof(vec3));
    memset(&max, 0, sizeof(vec3));
    memset(&dimension, 0, sizeof(vec3));
    memset(offset, ~0, sizeof(offset));
}

OBJMESH::OBJMESH(char *name, bool visible, char *group, float scale_x,
                 float scale_y, float scale_z, float distance,
                 bool use_smooth_normals, OBJ *parent) :
    visible(visible), current_material(NULL), radius(0.0f),
    distance(distance), vbo(0), stride(0), size(0), vao(0),
    btrigidbody(NULL), use_smooth_normals(use_smooth_normals),
    parent(parent)
{
    assert(name==NULL || strlen(name) < sizeof(this->name));
    if (name) {
        strcpy(this->name, name);
    } else {
        memset(this->name, 0, sizeof(this->name));
    }

    assert(strlen(group) < sizeof(this->group));
    if (group) {
        strcpy(this->group, group);
    } else {
        memset(this->group, 0, sizeof(this->group));
    }

    scale.x = scale_x;
    scale.y = scale_y;
    scale.z = scale_z;

    memset(&location, 0, sizeof(vec3));
    memset(&rotation, 0, sizeof(vec3));
    memset(&min, 0, sizeof(vec3));
    memset(&max, 0, sizeof(vec3));
    memset(&dimension, 0, sizeof(vec3));
    memset(offset, ~0, sizeof(offset));
}

OBJMESH::~OBJMESH()
{
    objvertexdata.clear();
    objtrianglelist.clear();
}

OBJMESH::OBJMESH(const OBJMESH &src)
{
    strcpy(name, src.name);
    visible = src.visible;
    strcpy(group, src.group);

    objvertexdata = src.objvertexdata;

    objtrianglelist = src.objtrianglelist;

    current_material = src.current_material;
    memcpy(&location, &src.location, sizeof(vec3));
    memcpy(&rotation, &src.rotation, sizeof(vec3));
    memcpy(&scale, &src.scale, sizeof(vec3));
    memcpy(&min, &src.min, sizeof(vec3));
    memcpy(&max, &src.max, sizeof(vec3));
    memcpy(&dimension, &src.dimension, sizeof(vec3));

    radius = src.radius;
    distance = src.distance;
    vbo = src.vbo;
    stride = src.stride;
    size = src.size;

    memcpy(offset, src.offset, sizeof(offset));

    vao = src.vao;
    btrigidbody = src.btrigidbody;
    use_smooth_normals = src.use_smooth_normals;
    parent = src.parent;
}

OBJMESH &OBJMESH::operator=(const OBJMESH &rhs)
{
    if (this != &rhs) {
        strcpy(name, rhs.name);
        visible = rhs.visible;
        strcpy(group, rhs.group);

        objvertexdata = rhs.objvertexdata;

        objtrianglelist = rhs.objtrianglelist;

        current_material = rhs.current_material;
        memcpy(&location, &rhs.location, sizeof(vec3));
        memcpy(&rotation, &rhs.rotation, sizeof(vec3));
        memcpy(&scale, &rhs.scale, sizeof(vec3));
        memcpy(&min, &rhs.min, sizeof(vec3));
        memcpy(&max, &rhs.max, sizeof(vec3));
        memcpy(&dimension, &rhs.dimension, sizeof(vec3));

        radius = rhs.radius;
        distance = rhs.distance;
        vbo = rhs.vbo;
        stride = rhs.stride;
        size = rhs.size;

        memcpy(offset, rhs.offset, sizeof(offset));

        vao = rhs.vao;
        btrigidbody = rhs.btrigidbody;
        use_smooth_normals = rhs.use_smooth_normals;
        
        parent = rhs.parent;
    }
    return *this;
}

OBJTRIANGLELIST::~OBJTRIANGLELIST()
{
    objtriangleindex.clear();

    indice_array.clear();
}

OBJTRIANGLELIST::OBJTRIANGLELIST(const OBJTRIANGLELIST &src)
{
    objtriangleindex = src.objtriangleindex;

    useuvs = src.useuvs;

    n_indice_array = src.n_indice_array;

    indice_array = src.indice_array;

    objmaterial = src.objmaterial;

    mode = src.mode;

    vbo = src.vbo;
}

OBJTRIANGLELIST &OBJTRIANGLELIST::operator=(const OBJTRIANGLELIST &rhs)
{
    if (this != &rhs) {
        objtriangleindex = rhs.objtriangleindex;

        useuvs = rhs.useuvs;

        n_indice_array = rhs.n_indice_array;

        indice_array = rhs.indice_array;

        objmaterial = rhs.objmaterial;
        
        mode = rhs.mode;
        
        vbo = rhs.vbo;
    }
    
    return *this;
}

void OBJMESH::add_vertex_data(OBJTRIANGLELIST   *objtrianglelist,
							  int               vertex_index,
							  int               uv_index)
{
	unsigned short index = 0;
	
	for ( ; index != this->objvertexdata.size(); ++index) {
		if (vertex_index == this->objvertexdata[index].vertex_index) {
			if (uv_index == -1)
                goto add_index_to_triangle_list;
			else if (uv_index == this->objvertexdata[index].uv_index)
                goto add_index_to_triangle_list;
		}
	}
	
	index = this->objvertexdata.size();
	
    this->objvertexdata.push_back(OBJVERTEXDATA(vertex_index, uv_index));


add_index_to_triangle_list:
	
	++objtrianglelist->n_indice_array;
	
	objtrianglelist->indice_array.push_back(index);
}


int OBJ::get_texture_index(char *filename)
{
	for (int i=0; i!=this->texture.size(); ++i) {
		if (!strcmp(filename, this->texture[i]->name)) return i;
	}
	
	return -1;
}


void OBJ::add_texture(char *filename)
{
	if (this->get_texture_index(filename) != -1) return;

	this->texture.push_back(TEXTURE_init(filename));
}


int OBJ::get_program_index(char *filename)
{
	for (int i=0; i!=this->program.size(); ++i) {
		if (!strcmp(filename, this->program[i]->name)) return i;
	}
	
	return -1;
}


void OBJ::add_program(char *filename)
{
	if (this->get_program_index(filename) != -1) return;

    this->program.push_back(new PROGRAM(filename));
}


void OBJ_build_texture(OBJ              *obj,
                       unsigned int     texture_index,
                       char             *texture_path,
                       unsigned int     flags,
                       unsigned char    filter,
                       float            anisotropic_filter)
{
	TEXTURE *texture = obj->texture[texture_index];

	MEMORY *m = NULL;
	
	char filename[MAX_PATH] = {""};
	
	sprintf(filename, "%s%s", texture_path, texture->name);
	
	m = mopen(filename, 0);
	
	if (m) {
		TEXTURE_load(texture, m);
		
		TEXTURE_generate_id(texture,
                            flags,
                            filter,
                            anisotropic_filter);

		TEXTURE_free_texel_array(texture);
		
		mclose(m);
	}
}


void OBJMATERIAL::build(PROGRAM *program)
{
	int index;
	
	char ext[MAX_CHAR] = {""};

	if (this->map_ambient[0]) {
		get_file_extension(this->map_ambient, ext, 1);

		if (!strcmp(ext, "GFX")) {
			index = this->parent->get_program_index(this->map_ambient);
			
			if (index != -1) this->program = this->parent->program[index];
		} else {
			index = this->parent->get_texture_index(this->map_ambient);
			
			if (index != -1) this->texture_ambient = this->parent->texture[index];
		}
	}


	if (this->map_diffuse[0]) {
		get_file_extension(this->map_diffuse, ext, 1);

		if (!strcmp(ext, "GFX")) {
			index = this->parent->get_program_index(this->map_diffuse);
			
			if (index != -1) this->program = this->parent->program[index];
		} else {
			index = this->parent->get_texture_index(this->map_diffuse);
			
			if (index != -1) this->texture_diffuse = this->parent->texture[index];
		}
	}


	if (this->map_specular[0]) {
		get_file_extension(this->map_specular, ext, 1);

		if (!strcmp(ext, "GFX")) {
			index = this->parent->get_program_index(this->map_specular);
			
			if (index != -1) this->program = this->parent->program[index];
		} else {
			index = this->parent->get_texture_index(this->map_specular);
			
			if (index != -1) this->texture_specular = this->parent->texture[index];
		}
	}


	if (this->map_translucency[0]) {
		get_file_extension(this->map_translucency, ext, 1);

		if (!strcmp(ext, "GFX")) {
			index = this->parent->get_program_index(this->map_translucency);
			
			if (index != -1) this->program = this->parent->program[index];
		} else {
			index = this->parent->get_texture_index(this->map_translucency);
			
			if (index != -1) this->texture_translucency = this->parent->texture[index];
		}
	}


	if (this->map_disp[0]) {
		get_file_extension(this->map_disp, ext, 1);

		if (!strcmp(ext, "GFX")) {
			index = this->parent->get_program_index(this->map_disp);
			
			if (index != -1) this->program = this->parent->program[index];
		} else {
			index = this->parent->get_texture_index(this->map_disp);
			
			if (index != -1) this->texture_disp = this->parent->texture[index];
		}
	}


	if (this->map_bump[0]) {
		get_file_extension(this->map_bump, ext, 1);

		if (!strcmp(ext, "GFX")) {
			index = this->parent->get_program_index(this->map_bump);
			
			if (index != -1) this->program = this->parent->program[index];
		} else {
			index = this->parent->get_texture_index(this->map_bump);
			
			if (index != -1) this->texture_bump = this->parent->texture[index];
		}
	}
	

	if (program) this->program = program;
}


void OBJMATERIAL::set_draw_callback(MATERIALDRAWCALLBACK *materialdrawcallback)
{
    this->materialdrawcallback = materialdrawcallback;
}


void OBJMESH::update_bounds()
{
	unsigned int index;
	
	// Get the mesh min and max.
	this->min.x =
	this->min.y =
	this->min.z = FLT_MAX;

	this->max.x =
	this->max.y =
	this->max.z = -FLT_MAX;
	
	
	for (int i=0; i!=this->objvertexdata.size(); ++i) {
		index = this->objvertexdata[i].vertex_index;
		
		if (this->parent->indexed_vertex[index].x < this->min.x) this->min.x = this->parent->indexed_vertex[index].x;
		if (this->parent->indexed_vertex[index].y < this->min.y) this->min.y = this->parent->indexed_vertex[index].y;
		if (this->parent->indexed_vertex[index].z < this->min.z) this->min.z = this->parent->indexed_vertex[index].z;

		if (this->parent->indexed_vertex[index].x > this->max.x) this->max.x = this->parent->indexed_vertex[index].x;
		if (this->parent->indexed_vertex[index].y > this->max.y) this->max.y = this->parent->indexed_vertex[index].y;
		if (this->parent->indexed_vertex[index].z > this->max.z) this->max.z = this->parent->indexed_vertex[index].z;
	}


	// Mesh location
	vec3_mid(&this->location,
             &this->min,
             &this->max);


	// Mesh dimension
	vec3_diff(&this->dimension,
              &this->max,
              &this->min);


	// Bounding sphere radius
	this->radius = this->dimension.x > this->dimension.y ?
					  this->dimension.x:
					  this->dimension.y;
	
	this->radius = this->radius > this->dimension.z ?
					  this->radius * 0.5f:
					  this->dimension.z * 0.5f;
	
	/*
	objmesh->radius = vec3_dist(&objmesh->min,
								&objmesh->max) * 0.5f;
	*/
}


void OBJMESH::build_vbo()
{
	// Build the VBO for the vertex data
	unsigned int index,
				 offset;
	
	this->stride  = sizeof(vec3); // Vertex
	this->stride += sizeof(vec3); // Normals
	this->stride += sizeof(vec3); // Face Normals
		
	if (this->objvertexdata[0].uv_index != -1) {
		this->stride += sizeof(vec3); // Tangent
		this->stride += sizeof(vec2); // Uv
	}
	
	this->size = this->objvertexdata.size() * this->stride;
	
	unsigned char *vertex_array = (unsigned char *) malloc(this->size),
				  *vertex_start = vertex_array;

	for (int i=0; i!=this->objvertexdata.size(); ++i) {
		index = this->objvertexdata[i].vertex_index;
		
		memcpy(vertex_array,
               &this->parent->indexed_vertex[index],
               sizeof(vec3));

		// Center the pivot
		vec3_diff((vec3 *)vertex_array,
                  (vec3 *)vertex_array,
                  &this->location);

		vertex_array += sizeof(vec3);


		memcpy(vertex_array,
               &this->parent->indexed_normal[index],
               sizeof(vec3));

		vertex_array += sizeof(vec3);


		memcpy(vertex_array,
               &this->parent->indexed_fnormal[index],
               sizeof(vec3));

		vertex_array += sizeof(vec3);


		if (this->objvertexdata[0].uv_index != -1) {
			memcpy(vertex_array,
                   &this->parent->indexed_uv[this->objvertexdata[i].uv_index],
                   sizeof(vec2));

			vertex_array += sizeof(vec2);

			memcpy(vertex_array,
                   &this->parent->indexed_tangent[index],
                   sizeof(vec3));
            
			vertex_array += sizeof(vec3);				
		}
	}
	
	
	glGenBuffers(1, &this->vbo);
	
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
	
	glBufferData(GL_ARRAY_BUFFER,
                 this->size,
                 vertex_start,
                 GL_STATIC_DRAW);

	free(vertex_start);


	this->offset[AO_Position] = 0;

	offset = sizeof(vec3);

	this->offset[AO_Normal] = offset;

	offset += sizeof(vec3);

	this->offset[AO_FNormal] = offset;

	offset += sizeof(vec3);


	if (this->objvertexdata[0].uv_index != -1) {
		this->offset[AO_TexCoord0] = offset;

		offset += sizeof(vec2);
        
		this->offset[AO_Tangent0] = offset;
	}

	
	for (int i=0; i!=this->objtrianglelist.size(); ++i) {
		glGenBuffers(1, &this->objtrianglelist[i].vbo);
		
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->objtrianglelist[i].vbo);
		
		glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     this->objtrianglelist[i].n_indice_array * sizeof(unsigned short),
                     &this->objtrianglelist[i].indice_array[0],
                     GL_STATIC_DRAW);
	}
}


void OBJMESH::set_attributes()
{
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo);

	glEnableVertexAttribArray(VA_Position);

	glVertexAttribPointer(VA_Position,
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          this->stride,
                          (void *)NULL);


	glEnableVertexAttribArray(VA_Normal);

	glVertexAttribPointer(VA_Normal,
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          this->stride,
                          BUFFER_OFFSET(this->offset[AO_Normal]));


	glEnableVertexAttribArray(VA_FNormal);

	glVertexAttribPointer(VA_FNormal,
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          this->stride,
                          BUFFER_OFFSET(this->offset[AO_FNormal]));


	if (this->offset[AO_TexCoord0] != OFFSET_NO_TEXCOORD_NEEDED) {
		glEnableVertexAttribArray(VA_TexCoord0);

		glVertexAttribPointer(VA_TexCoord0,
                              2,
                              GL_FLOAT,
                              GL_FALSE,
                              this->stride,
                              BUFFER_OFFSET(this->offset[AO_TexCoord0]));
        
		glEnableVertexAttribArray(VA_Tangent0);
        
		glVertexAttribPointer(VA_Tangent0,
                              3,
                              GL_FLOAT,
                              GL_FALSE, 
                              this->stride,
                              BUFFER_OFFSET(this->offset[AO_Tangent0]));
	}
}



void OBJMESH::build()
{
	this->update_bounds();
	
	this->build_vbo();
	

	glGenVertexArraysOES(1, &this->vao);
	
	glBindVertexArrayOES(this->vao);
	
	
	this->set_attributes();
	
	
	if (this->objtrianglelist.size() == 1) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->objtrianglelist[0].vbo);
    }
	
	
	glBindVertexArrayOES(0);
}


void OBJMESH::build2()
{
	this->update_bounds();

	this->build_vbo();
}


void OBJMESH::optimize(unsigned int vertex_cache_size)
{
	unsigned int s = 0;
	
	unsigned short n_group = 0;

	if (vertex_cache_size) SetCacheSize(vertex_cache_size);
	
	for (int i=0; i!=this->objtrianglelist.size(); ++i) {
		PrimitiveGroup *primitivegroup;
	
		if (GenerateStrips(&this->objtrianglelist[i].indice_array[0],
                           this->objtrianglelist[i].n_indice_array,
                           &primitivegroup,
                           &n_group,
                           true)) {
			if (primitivegroup[0].numIndices < this->objtrianglelist[i].n_indice_array) {
				this->objtrianglelist[i].mode = GL_TRIANGLE_STRIP;
				this->objtrianglelist[i].n_indice_array = primitivegroup[0].numIndices;
			
				s = primitivegroup[0].numIndices * sizeof(unsigned short);
						
                this->objtrianglelist[i].indice_array.resize(primitivegroup[0].numIndices);
				
				memcpy(&this->objtrianglelist[i].indice_array[0],
                       &primitivegroup[0].indices[0],
                       s);
			}
			
			delete[] primitivegroup;
		}
	}
}


OBJMESH *OBJ::get_mesh(const char *name, bool exact_name)
{
    if (exact_name) {
        for (int i=0; i!=this->objmesh.size(); ++i) {
            if (!strcmp(this->objmesh[i].name, name))
                return &this->objmesh[i];
        }
    } else {
        for (int i=0; i!=this->objmesh.size(); ++i) {
            if (strstr(this->objmesh[i].name, name))
                return &this->objmesh[i];
        }
	}

	return NULL;
}


int OBJ::get_mesh_index(const char *name, bool exact_name)
{
    if (exact_name) {
        for (int i=0; i!=this->objmesh.size(); ++i) {
            if (!strcmp(this->objmesh[i].name, name))
                return i;
        }
    } else {
        for (int i=0; i!=this->objmesh.size(); ++i) {
            if (strstr(this->objmesh[i].name, name))
                return i;
        }
	}

	return -1;
}


PROGRAM *OBJ::get_program(const char *name, bool exact_name)
{
    if (exact_name) {
        for (int i=0; i!=this->program.size(); ++i) {
            if (!strcmp(this->program[i]->name, name))
                return this->program[i];
        }
    } else {
        for (int i=0; i!=this->program.size(); ++i) {
            if (strstr(this->program[i]->name, name))
                return this->program[i];
        }
	}

	return NULL;
}


OBJMATERIAL *OBJ::get_material(const char *name, bool exact_name)
{
    if (exact_name) {
        for (int i=0; i!=this->objmaterial.size(); ++i) {
            if (!strcmp(this->objmaterial[i].name, name))
                return &this->objmaterial[i];
        }
    } else {
        for (int i=0; i!=this->objmaterial.size(); ++i) {
            if (strstr(this->objmaterial[i].name, name))
                return &this->objmaterial[i];
        }
	}

	return NULL;
}


TEXTURE *OBJ::get_texture(const char *name, bool exact_name)
{
    if (exact_name) {
        for (int i=0; i!=this->texture.size(); ++i) {
            if (!strcmp(this->texture[i]->name, name))
                return this->texture[i];
        }
    } else {
        for (int i=0; i!=this->texture.size(); ++i) {
            if (strstr(this->texture[i]->name, name))
                return this->texture[i];
        }
	}

	return NULL;
}


OBJMATERIAL::OBJMATERIAL(char *name, OBJ *parent) : parent(parent),
    illumination_model(0), dissolve(0), specular_exponent(0),
    optical_density(0), map_ambient(""), map_diffuse(""), map_specular(""),
    map_translucency(""), map_disp(""), map_bump(""), texture_ambient(NULL),
    texture_diffuse(NULL), texture_specular(NULL),
    texture_translucency(NULL), texture_disp(NULL), texture_bump(NULL),
    program(NULL), materialdrawcallback(NULL)
{
    assert(name==NULL || (strlen(name)<sizeof(this->name)));
    if (name == NULL) {
        memset(this->name, 0, sizeof(this->name));
    } else {
        strcpy(this->name, name);
    }

    memset(&this->ambient, 0, sizeof(vec4));
    memset(&this->diffuse, 0, sizeof(vec4));
    memset(&this->specular, 0, sizeof(vec4));
    memset(&this->transmission_filter, 0, sizeof(vec3));
}

OBJMATERIAL::~OBJMATERIAL()
{
    delete program;
}

OBJMATERIAL::OBJMATERIAL(const OBJMATERIAL &src)
{
    strcpy(name, src.name);
    memcpy(&ambient, &src.ambient, sizeof(vec4));
    memcpy(&diffuse, &src.diffuse, sizeof(vec4));
    memcpy(&specular, &src.specular, sizeof(vec4));
    memcpy(&transmission_filter, &src.transmission_filter, sizeof(vec3));
    illumination_model = src.illumination_model;
    dissolve = src.dissolve;
    specular_exponent = src.specular_exponent;
    optical_density = src.optical_density;
    strcpy(map_ambient, src.map_ambient);
    strcpy(map_diffuse, src.map_diffuse);
    strcpy(map_specular, src.map_specular);
    strcpy(map_translucency, src.map_translucency);
    strcpy(map_disp, src.map_disp);
    strcpy(map_bump, src.map_bump);
    texture_ambient = src.texture_ambient;
    texture_diffuse = src.texture_diffuse;
    texture_specular= src.texture_specular;
    texture_translucency = src.texture_translucency;
    texture_disp = src.texture_disp;
    texture_bump = src.texture_bump;
    program = src.program;
    materialdrawcallback = src.materialdrawcallback;
    parent = src.parent;
}

OBJMATERIAL &OBJMATERIAL::operator=(const OBJMATERIAL &rhs)
{
    if (this != &rhs) {
        strcpy(name, rhs.name);
        memcpy(&ambient, &rhs.ambient, sizeof(vec4));
        memcpy(&diffuse, &rhs.diffuse, sizeof(vec4));
        memcpy(&specular, &rhs.specular, sizeof(vec4));
        memcpy(&transmission_filter, &rhs.transmission_filter, sizeof(vec3));
        illumination_model = rhs.illumination_model;
        dissolve = rhs.dissolve;
        specular_exponent = rhs.specular_exponent;
        optical_density = rhs.optical_density;
        strcpy(map_ambient, rhs.map_ambient);
        strcpy(map_diffuse, rhs.map_diffuse);
        strcpy(map_specular, rhs.map_specular);
        strcpy(map_translucency, rhs.map_translucency);
        strcpy(map_disp, rhs.map_disp);
        strcpy(map_bump, rhs.map_bump);
        texture_ambient = rhs.texture_ambient;
        texture_diffuse = rhs.texture_diffuse;
        texture_specular= rhs.texture_specular;
        texture_translucency = rhs.texture_translucency;
        texture_disp = rhs.texture_disp;
        texture_bump = rhs.texture_bump;
        program = rhs.program;
        materialdrawcallback = rhs.materialdrawcallback;
        parent = rhs.parent;
    }
    return *this;
}

void OBJMATERIAL::draw()
{
	if (this) {
		if (this->program) this->program->draw();


		if (this->texture_ambient) {
			glActiveTexture(GL_TEXTURE0);

			TEXTURE_draw(this->texture_ambient);
		}

		
		if (this->texture_diffuse) {
			glActiveTexture(GL_TEXTURE1);

			TEXTURE_draw(this->texture_diffuse);
		}

		
		if (this->texture_specular) {
			glActiveTexture(GL_TEXTURE2);

			TEXTURE_draw(this->texture_specular);
		}


		if (this->texture_disp) {
			glActiveTexture(GL_TEXTURE3);

			TEXTURE_draw(this->texture_disp);
		}	
		
		
		if (this->texture_bump) {
			glActiveTexture(GL_TEXTURE4);

			TEXTURE_draw(this->texture_bump);
		}


		if (this->texture_translucency) {
			glActiveTexture(GL_TEXTURE5);

			TEXTURE_draw(this->texture_translucency);
		}
		

		if (this->materialdrawcallback) this->materialdrawcallback(this);
	}
}


void OBJMESH::draw()
{
	if (this->visible && this->distance) {
		if (this->vao)
            glBindVertexArrayOES(this->vao);
		else
            this->set_attributes();
		
		
		for (int i=0; i!=this->objtrianglelist.size(); ++i) {
			this->current_material = this->objtrianglelist[i].objmaterial;
		
			if (this->current_material) this->current_material->draw();
			
			if (this->vao) {
				if (this->objtrianglelist.size() != 1)
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->objtrianglelist[i].vbo);
			} else {
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->objtrianglelist[i].vbo);
            }
  			
			
			glDrawElements(this->objtrianglelist[i].mode,
                           this->objtrianglelist[i].n_indice_array,
                           GL_UNSIGNED_SHORT,
                           (void *)NULL);
		}
	}
}


void OBJMESH::draw2(OBJMESH *objmesh)
{
	for (int i=0; i!=this->parent->objmesh.size(); ++i) {
		if (&this->parent->objmesh[i] == objmesh) {
			objmesh->draw();
			return;
		}
	}
}


void OBJMESH::draw3(OBJMESH *objmesh)
{
	for (int i=0; i!=this->parent->objmesh.size(); ++i) {
		if (&this->parent->objmesh[i] == objmesh) {
			GFX_push_matrix();

			GFX_translate(objmesh->location.x,
                          objmesh->location.y,
                          objmesh->location.z);

			GFX_rotate(objmesh->rotation.z, 0.0f, 0.0f, 1.0f);
			GFX_rotate(objmesh->rotation.y, 0.0f, 1.0f, 0.0f);
			GFX_rotate(objmesh->rotation.x, 1.0f, 0.0f, 0.0f);

			GFX_scale(objmesh->scale.x,
                      objmesh->scale.y,
                      objmesh->scale.z);
            
			objmesh->draw();
			
			GFX_pop_matrix();
			
			return;
		}
	}
}


void OBJMESH::free_vertex_data()
{
	this->objvertexdata.clear();
	
	for (int i=0; i!=this->objtrianglelist.size(); ++i) {
        this->objtrianglelist[i].objtriangleindex.clear();
		
        this->objtrianglelist[i].indice_array.clear();
	}
}


bool OBJ::load_mtl(char *filename, bool relative_path)
{
	MEMORY *m = mopen(filename, relative_path);

	OBJMATERIAL *objmaterial = NULL;

	if (!m) return false;

	get_file_path(m->filename, this->texture_path);

	get_file_path(m->filename, this->program_path);

	char *line = strtok((char *)m->buffer, "\n"),
		 str[MAX_PATH] = {""};
		 
	vec3 v;

	while (line) {
		if (!line[0] || line[0] == '#') {
            goto next_mat_line;
		} else if (sscanf(line, "newmtl %s", str) == 1) {
			this->objmaterial.push_back(OBJMATERIAL(str, this));
			
			objmaterial = &this->objmaterial[this->objmaterial.size() - 1];
		} else if (sscanf(line, "Ka %f %f %f", &v.x, &v.y, &v.z) == 3) {
            memcpy(&objmaterial->ambient, &v, sizeof(vec3));
        } else if (sscanf(line, "Kd %f %f %f", &v.x, &v.y, &v.z) == 3) {
            memcpy(&objmaterial->diffuse, &v, sizeof(vec3));
        } else if (sscanf(line, "Ks %f %f %f", &v.x, &v.y, &v.z) == 3) {
            memcpy(&objmaterial->specular, &v, sizeof(vec3));
        } else if (sscanf(line, "Tf %f %f %f", &v.x, &v.y, &v.z) == 3) {
            memcpy(&objmaterial->transmission_filter, &v, sizeof(vec3));
        } else if (sscanf(line, "illum %f", &v.x) == 1) {
            objmaterial->illumination_model = (int)v.x;
        } else if (sscanf(line, "d %f", &v.x) == 1) {
			objmaterial->ambient.w  = v.x;
			objmaterial->diffuse.w  = v.x;
			objmaterial->specular.w = v.x;
			objmaterial->dissolve   = v.x;
		} else if (sscanf(line, "Ns %f", &v.x) == 1) {
            objmaterial->specular_exponent = v.x;
        } else if (sscanf(line, "Ni %f", &v.x) == 1) {
            objmaterial->optical_density = v.x;
        } else if (sscanf(line, "map_Ka %s", str) == 1) {
			get_file_name(str, objmaterial->map_ambient);
			
			get_file_extension(objmaterial->map_ambient, str, 1);
			
			if (!strcmp(str, "GFX"))
                this->add_program(objmaterial->map_ambient);
			else
                this->add_texture(objmaterial->map_ambient);
		} else if (sscanf(line, "map_Kd %s", str) == 1) {
			get_file_name(str, objmaterial->map_diffuse);
			
			get_file_extension(objmaterial->map_diffuse, str, 1);
			
			if (!strcmp(str, "GFX"))
                this->add_program(objmaterial->map_diffuse);
			else
                this->add_texture(objmaterial->map_diffuse);
		} else if (sscanf(line, "map_Ks %s", str) == 1) {
			get_file_name(str, objmaterial->map_specular);
			
			get_file_extension(objmaterial->map_specular, str, 1);
			
			if (!strcmp(str, "GFX"))
                this->add_program(objmaterial->map_specular);
			else
                this->add_texture(objmaterial->map_specular);
		} else if (sscanf(line, "map_Tr %s", str) == 1) {
			get_file_name(str, objmaterial->map_translucency);
			
			get_file_extension(objmaterial->map_translucency, str, 1);
			
			if (!strcmp(str, "GFX"))
                this->add_program(objmaterial->map_translucency);
			else
                this->add_texture(objmaterial->map_translucency);
		} else if (sscanf(line, "map_disp %s", str) == 1 ||
                   sscanf(line, "map_Disp %s", str) == 1 ||
                   sscanf(line, "disp %s"    , str) == 1) {
			get_file_name(str, objmaterial->map_disp);
			
			get_file_extension(objmaterial->map_disp, str, 1);
			
			if (!strcmp(str, "GFX"))
                this->add_program(objmaterial->map_disp);
			else
                this->add_texture(objmaterial->map_disp);

		} else if (sscanf(line, "map_bump %s", str) == 1 ||
                   sscanf(line, "map_Bump %s", str) == 1 ||
                   sscanf(line, "bump %s"	, str) == 1) {
			get_file_name(str, objmaterial->map_bump);
			
			get_file_extension(objmaterial->map_bump, str, 1);
			
			if (!strcmp(str, "GFX"))
                this->add_program(objmaterial->map_bump);
			else
                this->add_texture(objmaterial->map_bump);
		}

		next_mat_line:
		
			line = strtok(NULL, "\n");
	}

	mclose(m);

	return true;
}



OBJ::OBJ(char *filename, bool relative_path)
{
    memset(this->texture_path, 0, sizeof(this->texture_path));
    memset(this->program_path, 0, sizeof(this->program_path));

    if (filename == NULL) return;

	MEMORY *o = mopen(filename, relative_path);
	
	if (!o) {
        return;
	} else {
		char name  [MAX_CHAR] = {""},	
			 group [MAX_CHAR] = {""},
			 usemtl[MAX_CHAR] = {""},
			 str   [MAX_PATH] = {""},
			 last  = 0,
			 *line = strtok((char *)o->buffer, "\n");
		
		bool    use_smooth_normals;
		
		OBJMESH *objmesh = NULL;
		
		OBJTRIANGLELIST *objtrianglelist = NULL;
		
		vec3 v;

		while (line) {
			if (!line[0] || line[0] == '#') {
                goto next_obj_line;
			} else if (line[0] == 'f' && line[1] == ' ') {
				bool    useuvs;
						
				int vertex_index[3] = { 0, 0, 0 },
					normal_index[3] = { 0, 0, 0 },
					uv_index	[3] = { 0, 0, 0 },
					triangle_index;
					
				if (sscanf(line, "f %d %d %d", &vertex_index[0],
                                               &vertex_index[1],
                                               &vertex_index[2]) == 3) {
                    useuvs = false;
                } else if (sscanf(line, "f %d//%d %d//%d %d//%d", &vertex_index[0],
																  &normal_index[0],
																  &vertex_index[1],
																  &normal_index[1],
																  &vertex_index[2],
																  &normal_index[2]) == 6) {
                    useuvs = false;
                } else if (sscanf(line, "f %d/%d %d/%d %d/%d", &vertex_index[0],
															   &uv_index    [0],
															   &vertex_index[1],
															   &uv_index    [1],
															   &vertex_index[2],
															   &uv_index    [2]) == 6) {
                    useuvs = true;
                } else {
					sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d", &vertex_index[0],
                                                                 &uv_index    [0],
																 &normal_index[0],
																 &vertex_index[1],
																 &uv_index    [1],
																 &normal_index[1],
																 &vertex_index[2],
																 &uv_index    [2],
																 &normal_index[2]);
					useuvs = true;
				}
				
				
				if (last != 'f') {
					this->objmesh.push_back(OBJMESH(name[0] ? name : usemtl,
                                                   true,
                                                   group,
                                                   1.0f, 1.0f, 1.0f,
                                                   1.0f,
                                                   use_smooth_normals,
                                                   this));

					objmesh = &this->objmesh[this->objmesh.size() - 1];
	
					objmesh->objtrianglelist.push_back(OBJTRIANGLELIST(GL_TRIANGLES,
                                                                       useuvs,
                                                                       usemtl[0] ? this->get_material(usemtl, true) : NULL));

					objtrianglelist = &objmesh->objtrianglelist[objmesh->objtrianglelist.size() - 1];
					
					name  [0] = 0;
					usemtl[0] = 0;
				}
				
				--vertex_index[0];
				--vertex_index[1];
				--vertex_index[2];

				--uv_index[0];
				--uv_index[1];
				--uv_index[2];
				
				
				objmesh->add_vertex_data(objtrianglelist,
										 vertex_index[0],
										 uv_index    [0]);

				objmesh->add_vertex_data(objtrianglelist,
										 vertex_index[1],
										 uv_index    [1]);

				objmesh->add_vertex_data(objtrianglelist,
										 vertex_index[2],
										 uv_index    [2]);
										 
				
				triangle_index = objtrianglelist->objtriangleindex.size();
				
                objtrianglelist->objtriangleindex.push_back(OBJTRIANGLEINDEX(vertex_index, uv_index));
			} else if (sscanf(line, "v %f %f %f", &v.x, &v.y, &v.z) == 3) {
				// Vertex
				this->indexed_vertex.resize(this->indexed_vertex.size() + 1);

				memcpy(&this->indexed_vertex[this->indexed_vertex.size() - 1],
                       &v,
                       sizeof(vec3));


				// Normal
				this->indexed_normal.resize(this->indexed_normal.size() + 1);

				this->indexed_fnormal.resize(this->indexed_fnormal.size() + 1);

				memset(&this->indexed_normal[this->indexed_normal.size() - 1],
                       0,
                       sizeof(vec3));

				memset(&this->indexed_fnormal[this->indexed_fnormal.size() - 1],
                       0,
                       sizeof(vec3));

				// Tangent
				this->indexed_tangent.resize(this->indexed_tangent.size() + 1);
                
				memset(&this->indexed_tangent[this->indexed_tangent.size() - 1],
                       0,
                       sizeof(vec3));
			} else if (sscanf(line, "vn %f %f %f", &v.x, &v.y, &v.z) == 3) {
                // Drop the normals.
                goto next_obj_line;
			
			} else if (sscanf(line, "vt %f %f", &v.x, &v.y) == 2) {
				this->indexed_uv.resize(this->indexed_uv.size() + 1);
				v.y = 1.0f - v.y;
				
				memcpy(&this->indexed_uv[this->indexed_uv.size() - 1],
                       &v,
                       sizeof(vec2));
			} else if (line[0] == 'v' && line[1] == 'n') {
                goto next_obj_line;
            } else if (sscanf(line, "usemtl %s", str) == 1) {
                strcpy(usemtl, str);
            } else if (sscanf(line, "o %s", str) == 1) {
                strcpy(name, str);
            } else if (sscanf(line, "g %s", str) == 1) {
                strcpy(group, str);
			} else if (sscanf(line, "s %s", str) == 1) {
				use_smooth_normals = true;
				
				if (!strcmp(str, "off") || !strcmp(str, "0")) {
                    use_smooth_normals = false;
                }
			} else if (sscanf(line, "mtllib %s", str) == 1) {
				o->position = (unsigned char *)line - o->buffer + strlen(line) + 1;
				
				this->load_mtl(str, relative_path);
				
				line = strtok((char *)&o->buffer[o->position], "\n");
				continue;
			}

			next_obj_line:
			
				last = line[0];
				line = strtok(NULL, "\n");
		}
		
		mclose(o);
	}

	
	// Build Normals and Tangent
	{
		unsigned int index;
		
		for (int i=0; i!=this->objmesh.size(); ++i) {
			OBJMESH *objmesh = &this->objmesh[i];
		
			// Accumulate Normals and Tangent
			for (int j=0; j!= objmesh->objtrianglelist.size(); ++j) {
				OBJTRIANGLELIST *objtrianglelist = &objmesh->objtrianglelist[j];

				for (int k=0; k != objtrianglelist->objtriangleindex.size(); ++k) {
					vec3 v1,
                         v2,
                         normal;


					vec3_diff(&v1,
                              &this->indexed_vertex[objtrianglelist->objtriangleindex[k].vertex_index[0]],
                              &this->indexed_vertex[objtrianglelist->objtriangleindex[k].vertex_index[1]]);

					vec3_diff(&v2,
                              &this->indexed_vertex[objtrianglelist->objtriangleindex[k].vertex_index[0]],
                              &this->indexed_vertex[objtrianglelist->objtriangleindex[k].vertex_index[2]]);


					vec3_cross(&normal, &v1, &v2);

					vec3_normalize(&normal, &normal);


					// Face normals
					memcpy(&this->indexed_fnormal[objtrianglelist->objtriangleindex[k].vertex_index[0]],
                           &normal,
                           sizeof(vec3));

					memcpy(&this->indexed_fnormal[objtrianglelist->objtriangleindex[k].vertex_index[1]],
                           &normal,
                           sizeof(vec3));

					memcpy(&this->indexed_fnormal[objtrianglelist->objtriangleindex[k].vertex_index[2]],
                           &normal,
                           sizeof(vec3));


					// Smooth normals
					vec3_add(&this->indexed_normal[objtrianglelist->objtriangleindex[k].vertex_index[0]],
                             &this->indexed_normal[objtrianglelist->objtriangleindex[k].vertex_index[0]],
                             &normal);

					vec3_add(&this->indexed_normal[objtrianglelist->objtriangleindex[k].vertex_index[1]],
                             &this->indexed_normal[objtrianglelist->objtriangleindex[k].vertex_index[1]],
                             &normal);

					vec3_add(&this->indexed_normal[objtrianglelist->objtriangleindex[k].vertex_index[2]],
                             &this->indexed_normal[objtrianglelist->objtriangleindex[k].vertex_index[2]],
                             &normal);


					if (objtrianglelist->useuvs) {
						vec3 tangent;

						vec2 uv1, uv2;

						float c;

						vec2_diff(&uv1,
                                  &this->indexed_uv[objtrianglelist->objtriangleindex[k].uv_index[2]],
                                  &this->indexed_uv[objtrianglelist->objtriangleindex[k].uv_index[0]]);

						vec2_diff(&uv2,
                                  &this->indexed_uv[objtrianglelist->objtriangleindex[k].uv_index[1]],
                                  &this->indexed_uv[objtrianglelist->objtriangleindex[k].uv_index[0]]);


						c = 1.0f / (uv1.x * uv2.y - uv2.x * uv1.y);

						tangent.x = (v1.x * uv2.y + v2.x * uv1.y) * c;
						tangent.y = (v1.y * uv2.y + v2.y * uv1.y) * c;
						tangent.z = (v1.z * uv2.y + v2.z * uv1.y) * c;
                        
                        
						vec3_add(&this->indexed_tangent[objtrianglelist->objtriangleindex[k].vertex_index[0]], 
                                 &this->indexed_tangent[objtrianglelist->objtriangleindex[k].vertex_index[0]],
                                 &tangent);
                        
						vec3_add(&this->indexed_tangent[objtrianglelist->objtriangleindex[k].vertex_index[1]], 
                                 &this->indexed_tangent[objtrianglelist->objtriangleindex[k].vertex_index[1]],
                                 &tangent);
                        
						vec3_add(&this->indexed_tangent[objtrianglelist->objtriangleindex[k].vertex_index[2]], 
                                 &this->indexed_tangent[objtrianglelist->objtriangleindex[k].vertex_index[2]],
                                 &tangent);			
					}
				}
			}
		}
		

		// Normalize Normals & Tangent
		for (int i=0; i!=this->objmesh.size(); ++i) {
			for (int j=0; j != this->objmesh[i].objvertexdata.size();  ++j) {
				index = this->objmesh[i].objvertexdata[j].vertex_index;

				// Average smooth normals.
				vec3_normalize(&this->indexed_normal[index],
                               &this->indexed_normal[index]);

				if (this->objmesh[i].objvertexdata[j].uv_index != -1) {
					vec3_normalize(&this->indexed_tangent[index],
                                   &this->indexed_tangent[index]);
				}
			}
		}
	}

	return;
}


void OBJ::free_vertex_data()
{
    this->indexed_vertex.clear();

    this->indexed_normal.clear();

    this->indexed_fnormal.clear();

    this->indexed_tangent.clear();

    this->indexed_uv.clear();
}


OBJ::~OBJ()
{
	this->free_vertex_data();
	
	for (int i=0; i!=this->objmesh.size(); ++i) {
		if (this->objmesh[i].vao)
            glDeleteVertexArraysOES(1, &this->objmesh[i].vao);
		
		if (this->objmesh[i].vbo)
            glDeleteBuffers(1, &this->objmesh[i].vbo);
	
		this->objmesh[i].free_vertex_data();
		
		if (this->objmesh[i].objtrianglelist.size()) {
			for (int j=0; j != this->objmesh[i].objtrianglelist.size(); ++j) {
				glDeleteBuffers(1, &this->objmesh[i].objtrianglelist[j].vbo);
			}
		
			this->objmesh[i].objtrianglelist.clear();
		}
	}
	
	this->objmesh.clear();

	
	this->objmaterial.clear();


	for (int i=0; i!=this->program.size(); ++i) {
        delete this->program[i];
		this->program[i] = NULL;
	}
	
    this->program.clear();


	for (int i=0; i!=this->texture.size(); ++i) {
		this->texture[i] = TEXTURE_free(this->texture[i]);
	}
	
    this->texture.clear();
}
