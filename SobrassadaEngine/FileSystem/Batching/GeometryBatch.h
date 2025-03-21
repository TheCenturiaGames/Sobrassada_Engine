#pragma once

#include <unordered_set>

class MeshComponent;
class ResourceMesh;
class MeshComponent;

class GeometryBatch
{
  public:
    GeometryBatch(const MeshComponent* component, const ResourceMesh* resource);
    ~GeometryBatch();

    void AddComponent(const MeshComponent* component) { components.insert(component); }
    void AddResource(const ResourceMesh* resource) { uniqueMeshes.insert(resource); }

    void RemoveComponent(const MeshComponent* component) { components.erase(component); }
    void RemoveResource(const ResourceMesh* resource) { uniqueMeshes.erase(resource); }

    const unsigned int GetMode() const { return mode; }

  private:
    std::unordered_set<const MeshComponent*> components;
    std::unordered_set<const ResourceMesh*> uniqueMeshes;

    unsigned int vbo  = 0;
    unsigned int ebo  = 0;
    unsigned int vao  = 0;

    unsigned int mode = 0;
};
