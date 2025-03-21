#pragma once

#include <unordered_set>

class ComponentMesh;
class ResourceMesh;

class GeometryBatch
{
  public:
    GeometryBatch(const ResourceMesh* mesh);
    ~GeometryBatch();

    void AddComponent(ComponentMesh* component) { components.insert(component); }
    void AddResource(ResourceMesh* resource) { uniqueMeshes.insert(resource); }

    const unsigned int GetMode() const { return mode; }

  private:
    std::unordered_set<ComponentMesh*> components;
    std::unordered_set<ResourceMesh*> uniqueMeshes;

    unsigned int vbo  = 0;
    unsigned int ebo  = 0;
    unsigned int vao  = 0;

    unsigned int mode = 0;
};
