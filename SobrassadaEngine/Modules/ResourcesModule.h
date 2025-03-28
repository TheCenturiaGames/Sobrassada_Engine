#pragma once

#include "Module.h"

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

    Resource* RequestResource(UID uid);
    void ReleaseResource(const Resource* resource);

    void UnloadAllResources();

    BatchManager* GetBatchManager() { return batchManager; }

  private:
    Resource* CreateNewResource(UID uid);

  private:
    // We will need to do a resourceProgram
    int specularGlossinessProgram = -1;
    int metallicRoughnessProgram  = -1;
    std::map<UID, Resource*> resources;
    BatchManager* batchManager = nullptr;
};
