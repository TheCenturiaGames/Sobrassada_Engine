#include "ResourceNavMesh.h"

#include "Application.h"
#include "DebugDrawModule.h"
#include "EditorUIModule.h"
#include "FileSystem/Mesh.h"

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

    if (polymesh)
    {
        rcFreePolyMesh(polymesh); // Free the polymesh memory
        polymesh = nullptr;
    }

    if (polymeshDetail)
    {
        rcFreePolyMeshDetail(polymeshDetail); // Free the detail polymesh memory
        polymeshDetail = nullptr;
    }
    if (config)
    {
        delete config;
    }
}

ResourceNavMesh::ResourceNavMesh(UID uid, const std::string& name) : Resource(uid, name, ResourceType::Navmesh)
{
    config          = new rcConfig();
    // Default Heightfield Options
    config->bmin[0] = config->bmin[1] = config->bmin[2] = FLT_MAX;
    config->bmax[0] = config->bmax[1] = config->bmax[2] = FLT_MIN;
    config->cs                                          = 0.1f; // Default cell size
    config->ch                                          = 0.2f; // Default cell height
    config->width                                       = 1000; // Arbitrary default width
    config->height                                      = 1000; // Arbitrary default height

    // Default Walkable Options
    config->walkableSlopeAngle                          = 90.0f; // Max slope an agent can walk on
    config->walkableClimb                               = 20;    // Max step height agent can climb
    config->walkableHeight                              = 20;    // Min height required to pass
    config->walkableRadius                              = 1;     // Agent radius

    // Default Partition Options
    partitionType                                       = SAMPLE_PARTITION_MONOTONE;
    config->minRegionArea                               = 70; // Min region area (small regions will be removed)
    config->mergeRegionArea                             = 20; // Merge regions smaller than this size

    // Default Contour Options
    config->maxSimplificationError                      = 1.3f;
    config->maxEdgeLen                                  = 12;
    config->maxVertsPerPoly                             = 6;

    // Default PolyMesh Options
    config->detailSampleDist                            = 6.0f; // Higher = more accurate, lower = faster
    config->detailSampleMaxError                        = 1.0f; // Higher = more smooth, lower = accurate

    // Default Filters
    m_filterLowHangingObstacles                         = true;
    m_filterLedgeSpans                                  = true;
    m_filterWalkableLowHeightSpans                      = true;

    m_agentHeight                                       = 1.0f;
    m_agentMaxClimb                                     = 0.5f;
    m_agentRadius                                       = 0.5f;

    navQuery                                            = dtAllocNavMeshQuery();
}
// add together all meshes to create navmesh - needs the direction to a vector of resourcemesh pointers
bool ResourceNavMesh::BuildNavMesh(
    const std::vector<std::pair<const ResourceMesh*, const float4x4&>>& meshes, const float minPoint[3],
    const float maxPoint[3]
)
{
    int allVertexCount   = 0;
    int allTriangleCount = 0;
    int indexOffset      = 0;

    rcContext* context   = new rcContext();

    // first pass to get necessary sizes and AABB
    for (const auto& mesh : meshes)
    {
        allVertexCount   += mesh.first->GetVertexCount();
        allTriangleCount += (mesh.first->GetIndexCount() / 3);
    }

    config->bmin[0] = minPoint[0];
    config->bmin[1] = minPoint[1];
    config->bmin[2] = minPoint[2];

    config->bmax[0] = maxPoint[0];
    config->bmax[1] = maxPoint[1];
    config->bmax[2] = maxPoint[2];

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
    rcCalcGridSize(config->bmin, config->bmax, config->cs, &config->width, &config->height);
    heightfield = rcAllocHeightfield();

    if (!heightfield)
    {
        GLOG("Failed to allocate heightfield!");
        delete context;
        return false;
    }

    if (!rcCreateHeightfield(
            context, *heightfield, config->width, config->height, config->bmin, config->bmax, config->cs, config->ch
        ))
    {
        GLOG("Failed to create heightfield");
        delete context;
        return false;
    }

    unsigned char* triAreas = new unsigned char[allTriangleCount]();

    rcMarkWalkableTriangles(
        context, config->walkableSlopeAngle, navmeshVertices.data(), allVertexCount, navmeshTriangles.data(),
        allTriangleCount, triAreas
    );

    // CHECK OBSTACLES HERE IN THE FUTURE

    if (!rcRasterizeTriangles(
            context, navmeshVertices.data(), allVertexCount, navmeshTriangles.data(), triAreas, allTriangleCount,
            *heightfield, config->walkableClimb
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

    if (m_filterLowHangingObstacles) rcFilterLowHangingWalkableObstacles(context, config->walkableClimb, *heightfield);
    if (m_filterLedgeSpans) rcFilterLedgeSpans(context, config->walkableHeight, config->walkableClimb, *heightfield);
    if (m_filterWalkableLowHeightSpans) rcFilterWalkableLowHeightSpans(context, config->walkableHeight, *heightfield);

    // make a compact heightfield from the heightfield
    compactHeightfield = rcAllocCompactHeightfield();

    if (!compactHeightfield)
    {
        GLOG("buildNavigation: Compact heightfield out of memory.");
        rcFreeHeightField(heightfield);
        delete context;
        return false;
    }
    if (!rcBuildCompactHeightfield(
            context, config->walkableHeight, config->walkableClimb, *heightfield, *compactHeightfield
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
    if (!rcErodeWalkableArea(context, config->walkableRadius, *compactHeightfield))
    {
        GLOG("buildNavigation: Could not erode compact heightfield .");
        rcFreeCompactHeightfield(compactHeightfield);
        delete context;
        return false;
    }

    if (partitionType == SAMPLE_PARTITION_WATERSHED)
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
        if (!rcBuildRegions(context, *compactHeightfield, 0, config->minRegionArea, config->mergeRegionArea))
        {
            GLOG("buildNavigation: Could not build watershed regions.");
            rcFreeCompactHeightfield(compactHeightfield);
            delete context;
            return false;
        }
    }
    else if (partitionType == SAMPLE_PARTITION_MONOTONE)
    {
        // Partition the walkable surface into simple regions without holes.
        // Monotone partitioning does not need distancefield.
        if (!rcBuildRegionsMonotone(context, *compactHeightfield, 0, config->minRegionArea, config->mergeRegionArea))
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
        if (!rcBuildLayerRegions(context, *compactHeightfield, 0, config->minRegionArea))
        {
            GLOG("buildNavigation: Could not build layer regions.");
            rcFreeCompactHeightfield(compactHeightfield);
            delete context;
            return false;
        }
    }

    // allocate and build contourSet (for tracing region contours)
    contourSet = rcAllocContourSet();
    if (!contourSet)
    {
        GLOG("buildNavigation: ContourSet out of memory ");
        rcFreeCompactHeightfield(compactHeightfield);
        delete context;
        return false;
    }
    if (!rcBuildContours(context, *compactHeightfield, config->maxSimplificationError, config->maxEdgeLen, *contourSet))
    {
        GLOG("buildNavigation: Could not create contours.");
        delete context;
        rcFreeCompactHeightfield(compactHeightfield);
        rcFreeContourSet(contourSet);
        return false;
    }

    // Build polygon navmesh from the contours.

    // temporal polymesh and detailpolymesh
    polymesh = rcAllocPolyMesh();

    if (!polymesh)
    {
        GLOG("buildNavigation: Out of memory 'tempPolyMesh'.");
        return false;
    }
    if (!rcBuildPolyMesh(context, *contourSet, config->maxVertsPerPoly, *polymesh))
    {
        GLOG("buildNavigation: Could not triangulate contours.");
        return false;
    }
    polymeshDetail = rcAllocPolyMeshDetail();
    if (!polymeshDetail)
    {
        GLOG("buildNavigation: Out of memory 'tempPolyMeshDetail'.");
        return false;
    }

    if (!rcBuildPolyMeshDetail(
            context, *polymesh, *compactHeightfield, config->detailSampleDist, config->detailSampleMaxError,
            *polymeshDetail
        ))
    {
        GLOG("buildNavigation: Could not build detail mesh.");
        return false;
    }

    rcFreeCompactHeightfield(compactHeightfield);
    rcFreeContourSet(contourSet);

    // mark areas for detour
    if (config->maxVertsPerPoly <= DT_VERTS_PER_POLYGON)
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

    CreateDetourData();

    delete context;
    return true;
}

void ResourceNavMesh::CreateDetourData()
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
    params->walkableHeight        = m_agentHeight;
    params->walkableRadius        = m_agentRadius;
    params->walkableClimb         = m_agentMaxClimb;
    rcVcopy(params->bmin, polymesh->bmin);
    rcVcopy(params->bmax, polymesh->bmax);
    params->cs          = config->cs;
    params->ch          = config->ch;
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

void ResourceNavMesh::RenderNavmeshEditor()
{

    ImGui::SliderFloat("Cell Size", &config->cs, 0.1f, 1.0f);
    ImGui::SliderFloat("Cell Height", &config->ch, 0.1f, 1.0f);

    ImGui::SliderFloat("Max Walkable Slope Angle", &config->walkableSlopeAngle, 0.f, 90.f);
    ImGui::SliderInt("Max Walkable Climb", &config->walkableClimb, 1, 100);
    ImGui::SliderInt("Max Walkable Height", &config->walkableHeight, 1, 100);
    ImGui::SliderInt("Agent Radius", &config->walkableRadius, 1, 100);

    SamplePartitionType currentSelection = SamplePartitionType ::SAMPLE_PARTITION_MONOTONE;
    int currentIndex                     = static_cast<int>(currentSelection);

    const char* partitionLabels[]        = {
        "SAMPLE_PARTITION_WATERSHED", "SAMPLE_PARTITION_MONOTONE", "SAMPLE_PARTITION_LAYERS"
    };
    if (ImGui::ListBox("Partition Type", &currentIndex, partitionLabels, IM_ARRAYSIZE(partitionLabels)))
    {
        currentSelection = static_cast<SamplePartitionType>(currentIndex);
        partitionType    = currentSelection;
    }

    ImGui::SliderInt("Minimum Region Area", &config->minRegionArea, 1, 100);
    ImGui::SliderInt("Merge Region Area", &config->mergeRegionArea, 10, 100);

    ImGui::SliderFloat("Max Simplification Error", &config->maxSimplificationError, 0.1f, 5.0f);
    ImGui::SliderInt("Max Edges Length", &config->maxEdgeLen, 1, 100);
    ImGui::SliderInt("Max Vertices Per Polygon", &config->maxVertsPerPoly, 1, 100);

    ImGui::SliderFloat("Detail Sample Distance", &config->detailSampleDist, 0.1f, 5.0f);
    ImGui::SliderFloat("Detail Sample Max Error", &config->detailSampleMaxError, 0.1f, 5.0f);

    ImGui::Checkbox("Filter Low Hanging Obstacles", &m_filterLowHangingObstacles);
    ImGui::Checkbox("Filter Ledge Spans", &m_filterLedgeSpans);
    ImGui::Checkbox("Filter Walkable Low Height Spans", &m_filterWalkableLowHeightSpans);

    ImGui::SliderFloat("Detour Agent Height", &m_agentHeight, 0.1f, 5.0f);
    ImGui::SliderFloat("Detour Max Agent Climb", &m_agentMaxClimb, 0.1f, 5.0f);
    ImGui::SliderFloat("Detour Agent Radius", &m_agentRadius, 0.1f, 5.0f);
}