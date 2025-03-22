#pragma once

#include <unordered_set>
#include <vector>

class MeshComponent;
class ResourceMesh;
class MeshComponent;

class GeometryBatch
{
  public:
    GeometryBatch(const MeshComponent* component, const ResourceMesh* resource);
    ~GeometryBatch();

    void LoadData();
    void Render();
    void ClearObjectsToRender();

    void AddComponent(const MeshComponent* component) { components.push_back(component); }
    void AddResource(const ResourceMesh* resource) { uniqueMeshes.insert(resource); }

    const unsigned int GetMode() const { return mode; }

  private:
    std::vector<const MeshComponent*> components;
    std::unordered_set<const ResourceMesh*> uniqueMeshes;

    unsigned int indirect = 0;
    unsigned int vao      = 0;
    unsigned int vbo      = 0;
    unsigned int ebo      = 0;

    unsigned int mode     = 0;
};
