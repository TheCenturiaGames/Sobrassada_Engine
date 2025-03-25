#pragma once

#include <Math/float3.h>
#include <Math/float4x4.h>
#include <unordered_map>
#include <vector>

class MeshComponent;
class ResourceMesh;
class MeshComponent;
struct Command;
struct MaterialGPU;

struct AccMeshCount
{
    unsigned int accVertexCount;
    unsigned int accIndexCount;
};

class GeometryBatch
{
  public:
    GeometryBatch(const MeshComponent* component);
    ~GeometryBatch();

    void LoadData();
    void Render(
        unsigned int program, unsigned int cameraUBO, float3 cameraPos,
        const std::vector<MeshComponent*>& meshesToRender
    );

    void GenerateCommandsAndSSBO(
        const std::vector<MeshComponent*>& meshes, std::vector<Command>& commands, std::vector<float4x4>& totalModels,
        std::vector<MaterialGPU>& totalMaterials
    );

    void AddComponent(const MeshComponent* component) { components.push_back(component); }

    const unsigned int GetMode() const { return mode; }
    const bool GetIsMetallic() const { return isMetallic; }

  private:
    std::vector<const MeshComponent*> components;
    std::vector<const ResourceMesh*> uniqueMeshes;
    std::unordered_map<const ResourceMesh*, std::size_t> uniqueMeshesMap;
    std::vector<AccMeshCount> uniqueMeshesCount;

    unsigned int totalVertexCount = 0;
    unsigned int totalIndexCount  = 0;

    unsigned int indirect         = 0;
    unsigned int vao              = 0;
    unsigned int vbo              = 0;
    unsigned int ebo              = 0;
    unsigned int models           = 0;
    unsigned int materials        = 0;

    bool isMetallic               = false;
    unsigned int mode             = 0;
};
