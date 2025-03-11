#pragma once

#include "FileSystem.h"
#include "Model.h"
#include "ResourceManagement/Resources/ResourceModel.h"

#include "Math/float4x4.h"
#include "tiny_gltf.h"

class ResourceModel;

namespace ModelImporter
{
    UID ImportModel(
        const tinygltf::Model& model, const std::vector<std::vector<std::pair<UID, UID>>>& meshesUIDs,
        const char* filePath, const UID sourceUID = INVALID_UUID
    );
    void CopyModel(const std::string& filePath);
    ResourceModel* LoadModel(UID modelUID);
    void FillNodes(
        const std::vector<tinygltf::Node>& nodesList, int nodeId, int parentId,
        const std::vector<std::vector<std::pair<UID, UID>>>& meshesUIDs, std::vector<NodeData>& outNodes
    );
    const float4x4 GetNodeTransform(const tinygltf::Node& node);
}; // namespace ModelImporter
