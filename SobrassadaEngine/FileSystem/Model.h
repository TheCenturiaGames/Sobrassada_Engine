#pragma once

#include "Globals.h"
#include "Utils/Transform.h"

#include <vector>

struct NodeData
{
    std::string name;
    Transform transform;
    int parentIndex;
    int skinIndex;
    std::vector<std::pair<UID, UID>> meshes;
};

struct Skin
{
    int rootIndex;
    std::vector<int> bonesIndices;
    //std::vector<float4x4> inverseBindMatrices;
};

class Model
{
  public:
    Model() = default;
    Model(const UID id, const std::vector<NodeData>& nodes) : uid(id), nodes(nodes) {};

    const std::vector<NodeData>& GetNodes() const { return nodes; }
    size_t GetNodesCount() const { return nodes.size(); }

    void SetUID(const UID uid) { this->uid = uid; }
    void SetNodes(const std::vector<NodeData>& nodes) { this->nodes = nodes; }
    void SetSkins(const std::vector<Skin>& skins) { this->skins = skins; }

  private:
    UID uid;
    std::vector<NodeData> nodes;
    std::vector<Skin> skins;
};