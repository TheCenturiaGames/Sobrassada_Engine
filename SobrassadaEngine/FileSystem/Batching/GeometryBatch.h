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
typedef struct __GLsync* GLsync;
typedef unsigned int GLuint;

struct AccMeshCount
{
    unsigned int accVertexCount;
    unsigned int accIndexCount;
};

class GeometryBatch
{
  public:
    GeometryBatch(const MeshComponent* component, int id);
    ~GeometryBatch();

    void LoadData();
    void Render(
        unsigned int program, unsigned int cameraUBO, float3 cameraPos,
        const std::vector<MeshComponent*>& meshesToRender
    );

    void AddComponent(const MeshComponent* component) { components.push_back(component); }

    const unsigned int GetMode() const { return mode; }
    const bool GetIsMetallic() const { return isMetallic; }

  private:
    void LockBuffer();
    void UpdateBuffer();
    void WaitBuffer();

    void GenerateCommandsAndSSBO(const std::vector<MeshComponent*>& meshes, std::vector<Command>& commands);

    void CleanUp();

  private:
    int id = 0;
    std::vector<const MeshComponent*> components;
    std::unordered_map<const MeshComponent*, std::size_t> componentsMap; // index of position added

    std::unordered_map<const ResourceMesh*, std::size_t> uniqueMeshesMap;
    std::vector<AccMeshCount> uniqueMeshesCount;

    void* ptrModels[2]     = {nullptr, nullptr};
    GLsync gSync[2]        = {nullptr, nullptr};
    GLuint models[2]       = {0, 0};
    int currentBufferIndex = 0;
    std::size_t modelsSize = 0;

    std::vector<float4x4> totalModels;
    std::vector<MaterialGPU> totalMaterials;

    unsigned int totalVertexCount = 0;
    unsigned int totalIndexCount  = 0;

    unsigned int indirect         = 0;
    unsigned int vao              = 0;
    unsigned int vbo              = 0;
    unsigned int ebo              = 0;
    unsigned int materials        = 0;

    bool isMetallic               = false;
    unsigned int mode             = 0;
};
