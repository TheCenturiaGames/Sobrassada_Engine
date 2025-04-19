#include "MetaNavmesh.h"
#include "NavMeshConfig.h"

MetaNavmesh::MetaNavmesh(UID uid, const std::string& assetPath, const NavMeshConfig& config)
    : MetaFile(uid, assetPath), config(config)
{
}
void MetaNavmesh::AddImportOptions(rapidjson::Document& doc, rapidjson::Document::AllocatorType& allocator) const
{
    rapidjson::Value configObj(rapidjson::kObjectType);

    configObj.AddMember("cellSize", config.GetSettings().cellSize, allocator);
    configObj.AddMember("cellHeight", config.GetSettings().cellHeight, allocator);
    configObj.AddMember("walkableSlopeAngle", config.GetSettings().walkableSlopeAngle, allocator);
    configObj.AddMember("walkableClimb", config.GetSettings().walkableClimb, allocator);
    configObj.AddMember("walkableHeight", config.GetSettings().walkableHeight, allocator);
    configObj.AddMember("walkableRadius", config.GetSettings().walkableRadius, allocator);

    configObj.AddMember("maxEdgeLen", config.GetSettings().maxEdgeLen, allocator);
    configObj.AddMember("maxSimplificationError", config.GetSettings().maxSimplificationError, allocator);
    configObj.AddMember("minRegionArea", config.GetSettings().minRegionArea, allocator);
    configObj.AddMember("mergeRegionArea", config.GetSettings().mergeRegionArea, allocator);
    configObj.AddMember("maxVertsPerPoly", config.GetSettings().maxVertsPerPoly, allocator);

    configObj.AddMember("detailSampleDist", config.GetSettings().detailSampleDist, allocator);
    configObj.AddMember("detailSampleMaxError", config.GetSettings().detailSampleMaxError, allocator);

    configObj.AddMember("filterLowHangingObstacles", config.GetSettings().filterLowHangingObstacles, allocator);
    configObj.AddMember("filterLedgeSpans", config.GetSettings().filterLedgeSpans, allocator);
    configObj.AddMember("filterWalkableLowHeightSpans", config.GetSettings().filterWalkableLowHeightSpans, allocator);

    configObj.AddMember("agentHeight", config.GetSettings().agentHeight, allocator);
    configObj.AddMember("agentRadius", config.GetSettings().agentRadius, allocator);
    configObj.AddMember("agentMaxClimb", config.GetSettings().agentMaxClimb, allocator);

    doc.AddMember("NavMeshConfig", configObj, allocator);
}
