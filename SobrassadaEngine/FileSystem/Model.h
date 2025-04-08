#pragma once

#include "Globals.h"

#include "Math/float4x4.h"
#include <vector>

struct NodeData
{
    std::string name;
    float4x4 transform;
    int parentIndex;
    int skinIndex;
    std::vector<std::pair<UID, UID>> meshes;
};

struct Skin
{
    int rootIndex;
    std::vector<int> bonesIndices;
    std::vector<float4x4> inverseBindMatrices;
};

class Model
{
  public:
    Model() = default;
    Model(const UID id, const std::vector<std::vector<NodeData>>& nodes, const std::vector<Skin>& skins)
        : uid(id), nodes(nodes), skins(skins) {};
    ~Model() = default;

    const std::vector<std::vector<NodeData>>& GetNodes() const { return nodes; }
    const Skin& GetSkin(int skinIndex) const { return skins[skinIndex]; }
    const UID GetAnimationUID() const { return animUID; }
    const std::vector<UID>& GetAllAnimationUIDs() const { return allAnimUIDs; }

    void SetUID(const UID uid) { this->uid = uid; }
    void SetNodes(const std::vector<std::vector<NodeData>>& nodes) { this->nodes = nodes; }
    void SetSkins(const std::vector<Skin>& skins) { this->skins = skins; }
    void SetAnimationUID(UID animationUID) { animUID = animationUID; }
    void SetAllAnimationUIDs(std::vector<UID> animUids) { allAnimUIDs = animUids; }

  private:
    UID uid;
    std::vector<std::vector<NodeData>> nodes;
    std::vector<Skin> skins;
    UID animUID = INVALID_UID;
    std::vector<UID> allAnimUIDs;
};