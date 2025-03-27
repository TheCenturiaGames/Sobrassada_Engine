#pragma once

#include "Module.h"
#include "Resource.h"
#include "ResourceNavmesh.h"

#include <map>

class ResourcesModule : public Module
{
  public:
    ResourcesModule();
    ~ResourcesModule() override;

    bool Init() override;
    bool ShutDown() override;

    void CreateNavMesh();
    Resource* RequestResource(UID uid);
    void ReleaseResource(const Resource* resource);
    ResourceNavMesh* GetNavMesh() { return &myNavmesh; }

    void UnloadAllResources();

  private:
    Resource* CreateNewResource(UID uid);

  private:
    std::map<UID, Resource*> resources;
    ResourceNavMesh myNavmesh;
};
