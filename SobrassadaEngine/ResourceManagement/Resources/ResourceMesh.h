#pragma once

#include "FileSystem/Mesh.h"
#include "Resource.h"
#include "Math/float4x4.h"

#include <Geometry/AABB.h>

class ResourceMaterial;

class ResourceMesh : public Resource
{
  public:
    ResourceMesh(UID uid, const std::string& name, const float3& maxPos, const float3& minPos, const float4x4& transform);
    ~ResourceMesh() override;

    void Render(int program, const float4x4& modelMatrix, unsigned int cameraUBO, const ResourceMaterial* material) const;
    void LoadData(unsigned int mode, const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices, float4x4& transform);

    const AABB& GetAABB() const { return aabb; }
    int GetIndexCount() const { return indexCount; }
    const std::vector<Vertex>& GetLocalVertices() const { return vertices; }
    const std::vector<unsigned int>& GetIndices() const { return indices; }

    const float4x4& GetTransform() const { return currentMeshTransform; }
  private:
    unsigned int vbo         = 0;
    unsigned int ebo         = 0;
    unsigned int vao         = 0;
    unsigned int mode        = 0;
    unsigned int vertexCount = 0;
    unsigned int indexCount  = 0;

    float4x4 currentMeshTransform       = float4x4::identity;

    AABB aabb;

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
};
