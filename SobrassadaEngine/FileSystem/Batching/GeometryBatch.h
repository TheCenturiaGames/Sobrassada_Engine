#pragma once

#include <unordered_set>
#include <vector>
#include <Math/float3.h>

class MeshComponent;
class ResourceMesh;
class MeshComponent;
struct Command;

class GeometryBatch
{
  public:
    GeometryBatch(const MeshComponent* component);
    ~GeometryBatch();

    void LoadData();
    void Render(unsigned int program, unsigned int cameraUBO, float3 cameraPos);
    void ClearObjectsToRender();

    void AddComponent(const MeshComponent* component) { components.push_back(component); }

    const unsigned int GetMode() const { return mode; }
    const bool GetIsMetallic() const { return isMetallic; }

  private:
    std::vector<const MeshComponent*> components;
    std::vector<const ResourceMesh*> uniqueMeshes;
    std::vector<Command> commands;
    unsigned int vertexCount = 0;
    unsigned int indexCount  = 0;

    unsigned int indirect    = 0;
    unsigned int vao         = 0;
    unsigned int vbo         = 0;
    unsigned int ebo         = 0;
    unsigned int models      = 0;
    unsigned int materials   = 0;

    bool isMetallic          = false;
    unsigned int mode        = 0;
};
