#pragma once

#include "FileSystem/Mesh.h"
#include "Resource.h"

#include <Geometry/AABB.h>

class ResourceMaterial;
class GameObject;

namespace tinygltf
{
    class Model;
    struct Mesh;
    struct Primitive;
} // namespace tinygltf

class ResourceMesh : public Resource
{
  public:
    ResourceMesh(UID uid, const std::string& name, const float3& maxPos, const float3& minPos);
    ~ResourceMesh() override;

    void Render(
        int program, float4x4& modelMatrix, unsigned int cameraUBO, ResourceMaterial* material,
        const std::vector<GameObject*>& bones, const std::vector<float4x4>& bindMatrices
    );
    void LoadData(unsigned int mode, const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);

    const AABB& GetAABB() const { return aabb; }
    int GetIndexCount() const { return indexCount; }

    void SetMaterial(UID materialUID) { this->material = materialUID; }

  private:
    const float4x4 TestSkinning(
        int vertexIndex, const Vertex& vertex, const std::vector<GameObject*>& bones,
        const std::vector<float4x4>& bindMatrices
    );

  private:
    unsigned int vbo         = 0;
    unsigned int ebo         = 0;
    unsigned int vao         = 0;
    unsigned int mode        = 0;
    UID material             = CONSTANT_EMPTY_UID;
    unsigned int vertexCount = 0;
    unsigned int indexCount  = 0;
    AABB aabb;
    std::vector<Vertex> bindPoseVertices;
};
