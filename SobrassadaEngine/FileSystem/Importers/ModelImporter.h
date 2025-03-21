#pragma once

#include <Globals.h>

#include <string>
#include <vector>

class ResourceModel;
struct NodeData;

namespace tinygltf
{
    class Model;
    class Node;
} // namespace tinygltf

namespace ModelImporter
{
    UID ImportModel(
        const tinygltf::Model& model, const std::vector<std::vector<std::pair<UID, UID>>>& meshesUIDs,
        const char* filePath, const std::string& targetFilePath, const UID sourceUID = INVALID_UID
    );
    void CopyModel(
        const std::string& filePath, const std::string& targetFilePath, const std::string& name, const UID sourceUID
    );
    ResourceModel* LoadModel(UID modelUID);
    void FillNodes(
        const std::vector<tinygltf::Node>& nodesList, int nodeId, int parentId,
        const std::vector<std::vector<std::pair<UID, UID>>>& meshesUIDs, std::vector<NodeData>& outNodes
    );
    const float4x4 GetNodeTransform(const tinygltf::Node& node);
}; // namespace ModelImporter
