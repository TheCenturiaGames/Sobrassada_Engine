#pragma once

#include "Module.h"

struct dtNavMesh;
class dtCrowd;

class PathfinderModule : public Module
{
  public:
    PathfinderModule();
    ~PathfinderModule();
    bool Init() override;
    update_status Update(float deltaTime) override;

    std::vector<float3> FindPath(float3 start, float3 end);

  private:
    dtNavMeshQuery* navQuery;
    dtCrowd* crowd = nullptr;
};

