#pragma once

#include "Module.h"
#include "Resource.h"
#include "ResourceNavmesh.h"

#include <map>

class BatchManager;
class GeometryBatch;
class MeshComponent;
class Resource;

class ResourcesModule : public Module
{
  public:
    ResourcesModule();
    ~ResourcesModule() override;

    bool Init() override;
    //update_status PostUpdate(float deltaTime) override;
    bool ShutDown() override;

    void CreateNavMesh();
    Resource* RequestResource(UID uid);
    void ReleaseResource(const Resource* resource);
    ResourceNavMesh* GetNavMesh() { return &myNavmesh; }

    void UnloadAllResources();

    BatchManager* GetBatchManager() { return batchManager; }

  private:
    Resource* CreateNewResource(UID uid);

  private:
    std::map<UID, Resource*> resources;
    BatchManager* batchManager = nullptr;
    ResourceNavMesh myNavmesh;
};
