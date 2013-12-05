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


NAVIGATION::NAVIGATION(char *name) :
    triangle_flags(NULL),
    dtnavmesh(NULL),
    program(NULL)
{
    assert(name==NULL || strlen(name)<sizeof(this->name));
    if (name == NULL) {
        memset(this->name, 0, sizeof(this->name));
    } else {
        strcpy(this->name, name);
    }

    this->tolerance.x = 1.0f;
    this->tolerance.y = 2.0f;
    this->tolerance.z = 1.0f;
}


NAVIGATION::~NAVIGATION()
{
    if (this->dtnavmesh)
        dtFreeNavMesh(this->dtnavmesh);

    if (this->program)
        delete this->program;
}

	
bool NAVIGATION::build(OBJMESH *objmesh)
{
    unsigned int    k = 0,
                    triangle_count = 0;

    int *indices = NULL;

    vec3 *vertex_array = (vec3 *) malloc(objmesh->objvertexdata.size() * sizeof(vec3)),
    *vertex_start = vertex_array;

    rcHeightfield *rcheightfield;

    rcCompactHeightfield *rccompactheightfield;

    rcContourSet *rccontourset;

    rcPolyMesh *rcpolymesh;

    rcPolyMeshDetail *rcpolymeshdetail;
    
    
    for (auto objvertexdata=objmesh->objvertexdata.begin();
         objvertexdata != objmesh->objvertexdata.end(); ++objvertexdata) {
        memcpy(vertex_array,
               &objmesh->parent->indexed_vertex[objvertexdata->vertex_index],
               sizeof(vec3));

        vec3_to_recast(vertex_array);

        ++vertex_array;
    }

    
    for (auto objtrianglelist=objmesh->objtrianglelist.begin();
         objtrianglelist != objmesh->objtrianglelist.end(); ++objtrianglelist) {

        triangle_count += objtrianglelist->n_indice_array;

        indices = (int *) realloc(indices, triangle_count * sizeof(int));

        for (int j=0; j != objtrianglelist->n_indice_array; ++j) {
            indices[k] = objtrianglelist->indice_array[j];
            
            ++k;
        }
    }

    triangle_count /= 3;

    rcConfig rcconfig;

    memset(&rcconfig, 0, sizeof(rcConfig));

    rcconfig.cs                     = this->navigationconfiguration.cell_size;
    rcconfig.ch                     = this->navigationconfiguration.cell_height;
    rcconfig.walkableHeight         = (int)ceilf (this->navigationconfiguration.agent_height / rcconfig.ch);
    rcconfig.walkableRadius         = (int)ceilf (this->navigationconfiguration.agent_radius / rcconfig.cs);
    rcconfig.walkableClimb          = (int)floorf(this->navigationconfiguration.agent_max_climb / rcconfig.ch);
    rcconfig.walkableSlopeAngle     = this->navigationconfiguration.agent_max_slope;
    rcconfig.minRegionSize          = (int)rcSqr(this->navigationconfiguration.region_min_size);
    rcconfig.mergeRegionSize        = (int)rcSqr(this->navigationconfiguration.region_merge_size);
    rcconfig.maxEdgeLen             = (int)(this->navigationconfiguration.edge_max_len / rcconfig.cs);
    rcconfig.maxSimplificationError = this->navigationconfiguration.edge_max_error;
    rcconfig.maxVertsPerPoly        = (int)this->navigationconfiguration.vert_per_poly;
    rcconfig.detailSampleDist	    = rcconfig.cs * this->navigationconfiguration.detail_sample_dst;
    rcconfig.detailSampleMaxError   = rcconfig.ch * this->navigationconfiguration.detail_sample_max_error;
    
    
    rcCalcBounds((float *)vertex_start,
                 objmesh->objvertexdata.size(),
                 rcconfig.bmin,
                 rcconfig.bmax);


    rcCalcGridSize(rcconfig.bmin,
                   rcconfig.bmax,
                   rcconfig.cs,
                   &rcconfig.width,
                   &rcconfig.height);


    rcheightfield = rcAllocHeightfield();

    rcCreateHeightfield(*rcheightfield,
                        rcconfig.width,
                        rcconfig.height,
                        rcconfig.bmin,
                        rcconfig.bmax,
                        rcconfig.cs,
                        rcconfig.ch);


    this->triangle_flags = new unsigned char[triangle_count];

    memset(this->triangle_flags, 0, triangle_count * sizeof(unsigned char));

    rcMarkWalkableTriangles(rcconfig.walkableSlopeAngle,
                            (float *)vertex_start,
                            objmesh->objvertexdata.size(),
                            indices,
                            triangle_count,
                            this->triangle_flags);
    
    
    rcRasterizeTriangles((float *)vertex_start,
                         objmesh->objvertexdata.size(),
                         indices,
                         this->triangle_flags,
                         triangle_count,
                         *rcheightfield,
                         rcconfig.walkableClimb);
    
    
    delete []this->triangle_flags;
    this->triangle_flags = NULL;
    
    free(vertex_start);
    free(indices);
    
    
    rcFilterLowHangingWalkableObstacles(rcconfig.walkableClimb,
                                        *rcheightfield);


    rcFilterLedgeSpans(rcconfig.walkableHeight,
                       rcconfig.walkableClimb,
                       *rcheightfield);


    rcFilterWalkableLowHeightSpans(rcconfig.walkableHeight,
                                   *rcheightfield);


    rccompactheightfield = rcAllocCompactHeightfield();

    rcBuildCompactHeightfield(rcconfig.walkableHeight,
                              rcconfig.walkableClimb,
                              RC_WALKABLE,
                              *rcheightfield,
                              *rccompactheightfield);

    rcFreeHeightField(rcheightfield);
    rcheightfield = NULL;

    rcErodeArea(RC_WALKABLE_AREA,
                rcconfig.walkableRadius,
                *rccompactheightfield);


    rcBuildDistanceField(*rccompactheightfield);


    rcBuildRegions(*rccompactheightfield,
                   rcconfig.borderSize,
                   rcconfig.minRegionSize,
                   rcconfig.mergeRegionSize);
    
    
    rccontourset = rcAllocContourSet();
    
    rcBuildContours(*rccompactheightfield,
                    rcconfig.maxSimplificationError,
                    rcconfig.maxEdgeLen,
                    *rccontourset);
    
    
    rcpolymesh = rcAllocPolyMesh();
    
    rcBuildPolyMesh(*rccontourset,
                    rcconfig.maxVertsPerPoly,
                    *rcpolymesh);
    
    
    rcpolymeshdetail = rcAllocPolyMeshDetail();

    rcBuildPolyMeshDetail(*rcpolymesh,
                          *rccompactheightfield,
                          rcconfig.detailSampleDist,
                          rcconfig.detailSampleMaxError,
                          *rcpolymeshdetail);


    rcFreeCompactHeightfield(rccompactheightfield);
    rccompactheightfield = NULL;

    rcFreeContourSet(rccontourset);
    rccontourset = NULL;


    if (rcconfig.maxVertsPerPoly <= DT_VERTS_PER_POLYGON) {
        dtNavMeshCreateParams dtnavmeshcreateparams;

        unsigned char *nav_data = NULL;

        int nav_data_size = 0;

        for (int i=0; i != rcpolymesh->npolys; ++i) {
            if (rcpolymesh->areas[i] == RC_WALKABLE_AREA) {
                rcpolymesh->areas[i] = 0;
                rcpolymesh->flags[i] = 0x01;
            }
        }
        
        
        memset(&dtnavmeshcreateparams, 0, sizeof(dtNavMeshCreateParams));

        dtnavmeshcreateparams.verts	       = rcpolymesh->verts;
        dtnavmeshcreateparams.vertCount	       = rcpolymesh->nverts;
        dtnavmeshcreateparams.polys	       = rcpolymesh->polys;
        dtnavmeshcreateparams.polyAreas	       = rcpolymesh->areas;
        dtnavmeshcreateparams.polyFlags	       = rcpolymesh->flags;
        dtnavmeshcreateparams.polyCount	       = rcpolymesh->npolys;
        dtnavmeshcreateparams.nvp	       = rcpolymesh->nvp;

        dtnavmeshcreateparams.detailMeshes     = rcpolymeshdetail->meshes;
        dtnavmeshcreateparams.detailVerts      = rcpolymeshdetail->verts;
        dtnavmeshcreateparams.detailVertsCount = rcpolymeshdetail->nverts;
        dtnavmeshcreateparams.detailTris       = rcpolymeshdetail->tris;
        dtnavmeshcreateparams.detailTriCount   = rcpolymeshdetail->ntris;

        dtnavmeshcreateparams.walkableHeight   = this->navigationconfiguration.agent_height;
        dtnavmeshcreateparams.walkableRadius   = this->navigationconfiguration.agent_radius;
        dtnavmeshcreateparams.walkableClimb    = this->navigationconfiguration.agent_max_climb;

        rcVcopy(dtnavmeshcreateparams.bmin, rcpolymesh->bmin);
        rcVcopy(dtnavmeshcreateparams.bmax, rcpolymesh->bmax);

        dtnavmeshcreateparams.cs = rcconfig.cs;
        dtnavmeshcreateparams.ch = rcconfig.ch;


        dtCreateNavMeshData(&dtnavmeshcreateparams,
                            &nav_data,
                            &nav_data_size);
        
        if (!nav_data) return false;
        
        this->dtnavmesh = dtAllocNavMesh();
        
        this->dtnavmesh->init(nav_data,
                              nav_data_size,
                              DT_TILE_FREE_DATA,
                              NAVIGATION_MAX_NODE);
        
        rcFreePolyMesh(rcpolymesh);
        rcpolymesh = NULL;
        
        rcFreePolyMeshDetail(rcpolymeshdetail);
        rcpolymeshdetail = NULL;
        
        return true;
    }
    
    return false;
}


bool NAVIGATION::get_path(NAVIGATIONPATH *navigationpath, NAVIGATIONPATHDATA *navigationpathdata)
{
    vec3    start_location = { navigationpath->start_location.x,
                               navigationpath->start_location.y,
                               navigationpath->start_location.z },

            end_location = { navigationpath->end_location.x,
                             navigationpath->end_location.y,
                             navigationpath->end_location.z };

    vec3_to_recast(&start_location);

    vec3_to_recast(&end_location);

    navigationpath->start_reference = this->dtnavmesh->findNearestPoly((float *)&start_location,
                                                                       (float *)&this->tolerance,
                                                                       &navigationpath->path_filter,
                                                                       0);


    navigationpath->end_reference = this->dtnavmesh->findNearestPoly((float *)&end_location,
                                                                     (float *)&this->tolerance,
                                                                     &navigationpath->path_filter,
                                                                     0);


    navigationpath->poly_count = this->dtnavmesh->findPath(navigationpath->start_reference,
                                                           navigationpath->end_reference,
                                                           (float *)&start_location,
                                                           (float *)&end_location,
                                                           &navigationpath->path_filter,
                                                           navigationpath->poly_array,
                                                           NAVIGATION_MAX_POLY);

    if (navigationpath->poly_count) {
        vec3 closest_end;

        if (navigationpath->poly_array[navigationpath->poly_count - 1] != navigationpath->end_reference) {
            this->dtnavmesh->closestPointOnPoly(navigationpath->poly_array[navigationpath->poly_count - 1],
                                                (float *)&end_location,
                                                (float *)&closest_end);
        } else {
            memcpy(&closest_end, &navigationpath->end_location, sizeof(vec3));
        }


        navigationpathdata->path_point_count = this->dtnavmesh->findStraightPath((float *)&start_location,
                                                                                 (float *)&closest_end,
                                                                                 navigationpath->poly_array,
                                                                                 navigationpath->poly_count,
                                                                                 (float *)navigationpathdata->path_point_array,
                                                                                 navigationpathdata->path_flags_array,
                                                                                 navigationpathdata->path_poly_array,
                                                                                 NAVIGATION_MAX_POLY);
        
        memcpy(&navigationpathdata->path_point_array[navigationpathdata->path_point_count],
               &end_location,
               sizeof(vec3));
        
        
        if (navigationpathdata->path_point_count) {
            for (int i=0; i != navigationpathdata->path_point_count + 1; ++i)
                recast_to_vec3(&navigationpathdata->path_point_array[i]);
            
            return true;
        }
    }
    
    return false;
}


void NAVIGATION::draw()
{
    if (!this->program) {
        this->program = new PROGRAM(this->name);

        this->program->vertex_shader = new SHADER(this->name, GL_VERTEX_SHADER);

        this->program->vertex_shader->compile("uniform highp mat4 MODELVIEWPROJECTIONMATRIX;"
                                              "attribute highp vec3 POSITION;"
                                              "void main( void ) {"
                                              "gl_Position = MODELVIEWPROJECTIONMATRIX * vec4( POSITION, 1.0 ); }",
                                              false);

        this->program->fragment_shader = new SHADER(this->name, GL_FRAGMENT_SHADER);

        this->program->fragment_shader->compile("void main( void ) {"
                                                "gl_FragColor = vec4( 0.25, 0.5, 1.0, 0.65 ); }",
                                                false);

        this->program->link(false);
    }

    char vertex_attribute = this->program->get_vertex_attrib_location(VA_Position_String);

    glBindVertexArrayOES(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glEnable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    this->program->draw();

    glUniformMatrix4fv(this->program->get_uniform_location((char *)"MODELVIEWPROJECTIONMATRIX"),
                       1,
                       GL_FALSE,
                       (float *)GFX_get_modelview_projection_matrix());

    glEnableVertexAttribArray(vertex_attribute);


    for (int j=0; j != this->dtnavmesh->getMaxTiles(); ++j) {
        dtMeshTile *_dtMeshTile = this->dtnavmesh->getTile(j);

        if (!_dtMeshTile->header) {
            continue;
        }

        for (int k=0; k != _dtMeshTile->header->polyCount; ++k) {
            dtPoly *_dtPoly = &_dtMeshTile->polys[k];

            if (_dtPoly->type == DT_POLYTYPE_OFFMESH_CONNECTION) {
                continue;
            } else {
                dtPolyDetail* pd = &_dtMeshTile->detailMeshes[k];

                for (int l=0; l != pd->triCount; ++l) {
                    vec3 v[3];

                    const unsigned char *t = &_dtMeshTile->detailTris[(pd->triBase + l) << 2];

                    for (int m=2; m!=-1; --m) {
                        if (t[m] < _dtPoly->vertCount) {
                            memcpy(&v[m],
                                   &_dtMeshTile->verts[_dtPoly->verts[t[m]] * 3],
                                   sizeof(vec3));
                        } else {
                            memcpy(&v[m],
                                   &_dtMeshTile->detailVerts[(pd->vertBase + t[m] - _dtPoly->vertCount) * 3],
                                   sizeof(vec3));
                        }

                        recast_to_vec3(&v[m]);
                    }


                    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (float *)v);
                    
                    glDrawArrays(GL_TRIANGLES, 0, 3);
                }
            }
        }
    }
    
    glDisable(GL_BLEND);	
}
