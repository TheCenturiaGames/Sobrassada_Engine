#pragma once

#include "Math/float3.h"
#include "Math/float4x4.h"
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
    GeometryBatch(const MeshComponent* component);
    ~GeometryBatch();

    void LoadData();
    void Render(const std::vector<MeshComponent*>& meshesToRender);

    void AddComponent(const MeshComponent* component) { components.push_back(component); }

    const unsigned int GetMode() const { return mode; }
    const bool GetIsMetallic() const { return isMetallic; }
    const bool GetHasBones() const { return hasBones; }
    const unsigned int GetVertexCount() const { return totalVertexCount; }
    const unsigned int GetIndexCount() const { return totalIndexCount; }

  private:
    void LockBuffer();
    void UpdateModels(const std::vector<MeshComponent*>& meshesToRender);
    void UpdateBones(const std::vector<MeshComponent*>& meshesToRender);
    void WaitBuffer();

    void GenerateCommands(const std::vector<MeshComponent*>& meshes, std::vector<Command>& commands);

    void CleanUp();

  private:
    std::vector<const MeshComponent*> components;
    std::unordered_map<const MeshComponent*, std::size_t> componentsMap; // index of position added

    std::unordered_map<const ResourceMesh*, std::size_t> uniqueMeshesMap;
    std::vector<AccMeshCount> uniqueMeshesCount;

    GLsync gSync[2]        = {nullptr, nullptr};
    int currentBufferIndex = 0;

    GLuint models[2]       = {0, 0};
    float4x4* ptrModels[2] = {nullptr, nullptr};
    std::size_t modelsSize = 0;

    GLuint bones[2]        = {0, 0};
    GLuint bonesIndex      = 0;
    float4x4* ptrBones[2]  = {nullptr, nullptr};
    std::size_t bonesSize  = 0;
    std::size_t bonesIndexSize  = 0;
    std::vector<unsigned int> bonesCount;

    unsigned int totalVertexCount = 0;
    unsigned int totalIndexCount  = 0;

    unsigned int indirect         = 0;
    unsigned int vao              = 0;
    unsigned int vbo              = 0;
    unsigned int ebo              = 0;
    unsigned int materials        = 0;

    bool hasBones                 = false;
    bool isMetallic               = false;
    unsigned int mode             = 0;
};
