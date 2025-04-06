#pragma once

#include "Resource.h"

#include "Geometry/AABB.h"
#include "Math/float4x4.h"
#include "rapidjson/document.h"
#include <vector>

namespace tinygltf
{
    class Model;
    struct Mesh;
    struct Primitive;
} // namespace tinygltf

namespace math
{
    class float3;
}

class ResourceMaterial;
class GameObject;
struct Vertex;

class ResourceMesh : public Resource
{
  public:
    ResourceMesh(
        UID uid, const std::string& name, const math::float3& maxPos, const math::float3& minPos,
        const rapidjson::Value& importOptions
    );
    ~ResourceMesh() override;

    void Render(
        int program, const float4x4& modelMatrix, unsigned int cameraUBO, const ResourceMaterial* material,
        const std::vector<GameObject*>& bones, const std::vector<float4x4>& bindMatrices
    );
    void LoadData(unsigned int mode, const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);

    const AABB& GetAABB() const { return aabb; }
    int GetIndexCount() const { return indexCount; }
    int GetVertexCount() const { return vertexCount; }
    const std::vector<Vertex>& GetLocalVertices() const { return vertices; }
    const std::vector<unsigned int>& GetIndices() const { return indices; }
    const float4x4& GetDefaultTransform() const { return defaultTransform; }
    const unsigned int GetMode() const { return mode; }

    const UID GetDefaultMaterialUID() const { return defaultMaterialUID; }

  private:
    unsigned int vbo         = 0; // should be deleted
    unsigned int mode        = 0;
    unsigned int vertexCount = 0;
    unsigned int indexCount  = 0;
    AABB aabb;

    std::vector<Vertex> bindPoseVertices;
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    bool generateTangents     = false;
    float4x4 defaultTransform = float4x4::identity;
    
    UID defaultMaterialUID    = INVALID_UID;
};
