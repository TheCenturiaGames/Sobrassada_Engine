#pragma once

#include "Module.h"

#include <map>

class ResourceNavMesh;
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
    // update_status PostUpdate(float deltaTime) override;
    bool ShutDown() override;

    void CreateNavMesh();
    Resource* RequestResource(UID uid);
    void ReleaseResource(const Resource* resource);
    void UnloadAllResources();

    ResourceNavMesh* GetNavMesh() { return tmpNavmesh; }
    BatchManager* GetBatchManager() { return batchManager; }

  private:
    Resource* CreateNewResource(UID uid);

  private:
    std::map<UID, Resource*> resources;
    BatchManager* batchManager = nullptr;
    ResourceNavMesh* tmpNavmesh; // TODO Remove when NavMeshImporter is done.
};
