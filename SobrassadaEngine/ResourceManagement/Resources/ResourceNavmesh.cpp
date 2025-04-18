    #include "ResourceNavmesh.h"

    #include "Application.h"
    #include "DebugDrawModule.h"
    #include "EditorUIModule.h"
    #include "FileSystem/Mesh.h"
    #include "NavmeshImporter.h"
    #include "NavMeshConfig.h"

    #include <DetourNavMeshBuilder.h>
    #include <DetourNavMeshQuery.h>
    #include <Recast.h>
    #include <ResourceMesh.h>
    #include <ResourcesModule.h>
    #include <algorithm>

    ResourceNavMesh::~ResourceNavMesh()
    {
        if (navQuery)
        {
            dtFreeNavMeshQuery(navQuery);
            navQuery = nullptr;
        }
        if (navMesh)
        {
            dtFreeNavMesh(navMesh); // Free the navMesh memory
            navMesh = nullptr;      // Set the pointer to null for safety
        }

    }

    ResourceNavMesh::ResourceNavMesh(UID uid, const std::string& name) : Resource(uid, name, ResourceType::Navmesh)
    {

    
    }
    // add together all meshes to create navmesh - needs the direction to a vector of resourcemesh pointers
    bool ResourceNavMesh::BuildNavMesh(
        const std::vector<std::pair<const ResourceMesh*, const float4x4&>>& meshes, const float minPoint[3],
        const float maxPoint[3], const NavMeshConfig& userConfig
    )
    {
        int allVertexCount   = 0;
        int allTriangleCount = 0;
        int indexOffset      = 0;
        navQuery = dtAllocNavMeshQuery();
        rcConfig config;

        //fills temporary config with values from the ui
        userConfig.ApplyTo(&config);

        rcContext* context = new rcContext();

        // first pass to get necessary sizes and AABB
        for (const auto& mesh : meshes)
        {
            allVertexCount   += mesh.first->GetVertexCount();
            allTriangleCount += (mesh.first->GetIndexCount() / 3);
        }

        config.bmin[0] = minPoint[0];
        config.bmin[1] = minPoint[1];
        config.bmin[2] = minPoint[2];

        config.bmax[0] = maxPoint[0];
        config.bmax[1] = maxPoint[1];
        config.bmax[2] = maxPoint[2];

        std::vector<float> navmeshVertices;
        std::vector<int> navmeshTriangles;

        // needs to be just float insteaf of float3
        navmeshVertices.reserve(allVertexCount * 3);
        navmeshTriangles.reserve(allTriangleCount);

        // needs to add all vertices and indices from every single mesh in the navmesh
        for (const auto& mesh : meshes)
        {
            if (!mesh.first) continue;

            const std::vector<Vertex>& meshVerts         = mesh.first->GetLocalVertices();
            const std::vector<unsigned int>& meshIndices = mesh.first->GetIndices();

            const int vertexCount                        = mesh.first->GetVertexCount();

            for (const Vertex& vertex : meshVerts)
            {
                float4 tmpvertex(vertex.position.x, vertex.position.y, vertex.position.z, 1);

                tmpvertex = mesh.second.Transform(tmpvertex);
                navmeshVertices.push_back(tmpvertex.x);
                navmeshVertices.push_back(tmpvertex.y);
                navmeshVertices.push_back(tmpvertex.z);
            }

            for (const unsigned int& index : meshIndices)
            {
                navmeshTriangles.push_back(index + indexOffset);
            }
            indexOffset += vertexCount;
        }
        rcCalcGridSize(config.bmin, config.bmax, config.cs, &config.width, &config.height);

        rcHeightfield* heightfield = rcAllocHeightfield();

        if (!heightfield)
        {
            GLOG("Failed to allocate heightfield!");
            delete context;
            return false;
        }

        if (!rcCreateHeightfield(
                context, *heightfield, config.width, config.height, config.bmin, config.bmax, config.cs, config.ch
            ))
        {
            GLOG("Failed to create heightfield");
            delete context;
            return false;
        }

        unsigned char* triAreas = new unsigned char[allTriangleCount]();

        rcMarkWalkableTriangles(
            context, config.walkableSlopeAngle, navmeshVertices.data(), allVertexCount, navmeshTriangles.data(),
            allTriangleCount, triAreas
        );

        // CHECK OBSTACLES HERE IN THE FUTURE

        if (!rcRasterizeTriangles(
                context, navmeshVertices.data(), allVertexCount, navmeshTriangles.data(), triAreas, allTriangleCount,
                *heightfield, config.walkableClimb
            ))
        {
            GLOG("Failed to rasterize triangles for navmesh!");
            delete[] triAreas;
            rcFreeHeightField(heightfield);
            delete context;
            return false;
        }

        delete[] triAreas;
        triAreas = 0;

        if (userConfig.GetSettings().filterLowHangingObstacles) rcFilterLowHangingWalkableObstacles(context, config.walkableClimb, *heightfield);
        if (userConfig.GetSettings().filterLedgeSpans) rcFilterLedgeSpans(context, config.walkableHeight, config.walkableClimb, *heightfield);
        if (userConfig.GetSettings().filterWalkableLowHeightSpans) rcFilterWalkableLowHeightSpans(context, config.walkableHeight, *heightfield);

        // make a compact heightfield from the heightfield

        rcCompactHeightfield* compactHeightfield = rcAllocCompactHeightfield();

        if (!compactHeightfield)
        {
            GLOG("buildNavigation: Compact heightfield out of memory.");
            rcFreeHeightField(heightfield);
            delete context;
            return false;
        }
        if (!rcBuildCompactHeightfield(
                context, config.walkableHeight, config.walkableClimb, *heightfield, *compactHeightfield
            ))
        {
            GLOG("buildNavigation: Could not build compact data.");
            rcFreeHeightField(heightfield);
            delete context;
            return false;
        }

        rcFreeHeightField(heightfield);
        heightfield = 0;

        // Erode the walkable area by agent radius.
        if (!rcErodeWalkableArea(context, config.walkableRadius, *compactHeightfield))
        {
            GLOG("buildNavigation: Could not erode compact heightfield .");
            rcFreeCompactHeightfield(compactHeightfield);
            delete context;
            return false;
        }

        if (userConfig.GetSettings().partitionType == SamplePartitionType::SAMPLE_PARTITION_WATERSHED)
        {
            // Prepare for region partitioning, by calculating distance field along the walkable surface.
            if (!rcBuildDistanceField(context, *compactHeightfield))
            {
                GLOG("buildNavigation: Could not build distance field.");
                rcFreeCompactHeightfield(compactHeightfield);
                delete context;
                return false;
            }

            // Partition the walkable surface into simple regions without holes.
            if (!rcBuildRegions(context, *compactHeightfield, 0, config.minRegionArea, config.mergeRegionArea))
            {
                GLOG("buildNavigation: Could not build watershed regions.");
                rcFreeCompactHeightfield(compactHeightfield);
                delete context;
                return false;
            }
        }
        else if (userConfig.GetSettings().partitionType == SamplePartitionType::SAMPLE_PARTITION_MONOTONE)
        {
            // Partition the walkable surface into simple regions without holes.
            // Monotone partitioning does not need distancefield.
            if (!rcBuildRegionsMonotone(context, *compactHeightfield, 0, config.minRegionArea, config.mergeRegionArea))
            {
                GLOG("buildNavigation: Could not build monotone regions.");
                rcFreeCompactHeightfield(compactHeightfield);
                delete context;
                return false;
            }
        }
        else // SAMPLE_PARTITION_LAYERS
        {
            // Partition the walkable surface into simple regions without holes.
            if (!rcBuildLayerRegions(context, *compactHeightfield, 0, config.minRegionArea))
            {
                GLOG("buildNavigation: Could not build layer regions.");
                rcFreeCompactHeightfield(compactHeightfield);
                delete context;
                return false;
            }
        }
    
        // allocate and build contourSet (for tracing region contours)
        rcContourSet* contourSet = rcAllocContourSet();
        if (!contourSet)
        {
            GLOG("buildNavigation: ContourSet out of memory ");
            rcFreeCompactHeightfield(compactHeightfield);
            delete context;
            return false;
        }
        if (!rcBuildContours(context, *compactHeightfield, config.maxSimplificationError, config.maxEdgeLen, *contourSet))
        {
            GLOG("buildNavigation: Could not create contours.");
            delete context;
            rcFreeCompactHeightfield(compactHeightfield);
            rcFreeContourSet(contourSet);
            return false;
        }

        // Build polygon navmesh from the contours.

        // temporal polymesh and detailpolymesh
    

        rcPolyMesh* polymesh = rcAllocPolyMesh();

        if (!polymesh)
        {
            GLOG("buildNavigation: Out of memory 'tempPolyMesh'.");
            return false;
        }
        if (!rcBuildPolyMesh(context, *contourSet, config.maxVertsPerPoly, *polymesh))
        {
            GLOG("buildNavigation: Could not triangulate contours.");
            return false;
        }
    

        rcPolyMeshDetail* polymeshDetail = rcAllocPolyMeshDetail();
        if (!polymeshDetail)
        {
            GLOG("buildNavigation: Out of memory 'tempPolyMeshDetail'.");
            return false;
        }

        if (!rcBuildPolyMeshDetail(
                context, *polymesh, *compactHeightfield, config.detailSampleDist, config.detailSampleMaxError,
                *polymeshDetail
            ))
        {
            GLOG("buildNavigation: Could not build detail mesh.");
            return false;
        }

        rcFreeCompactHeightfield(compactHeightfield);
        rcFreeContourSet(contourSet);

        // mark areas for detour
        if (config.maxVertsPerPoly <= DT_VERTS_PER_POLYGON)
        {

            for (int i = 0; i < polymesh->npolys; ++i)
            {
                if (polymesh->areas[i] == RC_WALKABLE_AREA) polymesh->areas[i] = SAMPLE_POLYAREA_GROUND;

                if (polymesh->areas[i] == SAMPLE_POLYAREA_GROUND || polymesh->areas[i] == SAMPLE_POLYAREA_GRASS ||
                    polymesh->areas[i] == SAMPLE_POLYAREA_ROAD)
                {
                    polymesh->flags[i] = SAMPLE_POLYFLAGS_WALK;
                }
                else if (polymesh->areas[i] == SAMPLE_POLYAREA_WATER)
                {
                    polymesh->flags[i] = SAMPLE_POLYFLAGS_SWIM;
                }
                else if (polymesh->areas[i] == SAMPLE_POLYAREA_DOOR)
                {
                    polymesh->flags[i] = SAMPLE_POLYFLAGS_WALK | SAMPLE_POLYFLAGS_DOOR;
                }
            }
        }


        CreateDetourData(polymesh, polymeshDetail, config);

        delete context;
        return true;
    }

    void ResourceNavMesh::CreateDetourData(const rcPolyMesh* polymesh, const rcPolyMeshDetail* polymeshDetail, const rcConfig& config)
    {
        if (!polymesh) return;
        unsigned char* navData        = 0;
        int navDataSize               = 0;

        dtNavMeshCreateParams* params = new dtNavMeshCreateParams();

        params->verts                 = polymesh->verts;
        params->vertCount             = polymesh->nverts;
        params->polys                 = polymesh->polys;
        params->polyAreas             = polymesh->areas;
        params->polyFlags             = polymesh->flags;
        params->polyCount             = polymesh->npolys;
        params->nvp                   = polymesh->nvp;
        params->detailMeshes          = polymeshDetail->meshes;
        params->detailVerts           = polymeshDetail->verts;
        params->detailVertsCount      = polymeshDetail->nverts;
        params->detailTris            = polymeshDetail->tris;
        params->detailTriCount        = polymeshDetail->ntris;
        /*
        params.offMeshConVerts  = meshes->getOffMeshConnectionVerts();
        params.offMeshConRad    = m_geom->getOffMeshConnectionRads();
        params.offMeshConDir    = m_geom->getOffMeshConnectionDirs();
        params.offMeshConAreas  = m_geom->getOffMeshConnectionAreas(); This is used for jumping/doors/teleporting
        params.offMeshConFlags  = m_geom->getOffMeshConnectionFlags();
        params.offMeshConUserID = m_geom->getOffMeshConnectionId();
        params.offMeshConCount  = m_geom->getOffMeshConnectionCount();
        */
        params->walkableHeight        = (float)config.walkableHeight;
        params->walkableRadius        = (float)config.walkableRadius;
        params->walkableClimb         = (float)config.walkableClimb;
        rcVcopy(params->bmin, polymesh->bmin);
        rcVcopy(params->bmax, polymesh->bmax);
        params->cs          = config.cs;
        params->ch          = config.ch;
        params->buildBvTree = true;

        if (!dtCreateNavMeshData(params, &navData, &navDataSize))
        {
            GLOG("Could not build Detour navmesh.");
            delete params;
            return;
        }

        delete params;

        navMesh = dtAllocNavMesh();

        if (!navMesh)
        {
            dtFree(navData);
            GLOG("Could not create Detour navmesh");
            return;
        }

        dtStatus status;

        status = navMesh->init(navData, navDataSize, DT_TILE_FREE_DATA);
        if (dtStatusFailed(status))
        {
            dtFree(navData);
            GLOG("Could not init Detour navmesh");
            return;
        }
        status = navQuery->init(navMesh, 2048);
        if (dtStatusFailed(status))
        {
            GLOG("Could not init Detour navmesh query");
            return;
        }
    }

    void ResourceNavMesh::SetDetourNavMesh(dtNavMesh* navMesh)
    {
        this->navMesh = navMesh;
    }
    
