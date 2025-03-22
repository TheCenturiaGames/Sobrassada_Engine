#pragma once

#include <unordered_set>
#include <vector>

class MeshComponent;
class ResourceMesh;
class MeshComponent;
struct Command;

class GeometryBatch
{
  public:
    GeometryBatch(const MeshComponent* component, const ResourceMesh* resource);
    ~GeometryBatch();

    void LoadData();
    void Render();
    void ClearObjectsToRender();

    void AddComponent(const MeshComponent* component) { components.push_back(component); }

    const unsigned int GetMode() const { return mode; }

  private:
    std::vector<const MeshComponent*> components;
    std::vector<const ResourceMesh*> uniqueMeshes;
    std::vector<Command> commands;

    unsigned int indirect    = 0;
    unsigned int vao         = 0;
    unsigned int vbo         = 0;
    unsigned int ebo         = 0;

    unsigned int mode        = 0;
    unsigned int vertexCount = 0;
    unsigned int indexCount  = 0;
};
