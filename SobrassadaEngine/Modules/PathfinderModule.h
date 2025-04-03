#pragma once

#include "Module.h"
#include "Globals.h"

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
    void QueryNavmesh(UID uid);

  private:
    dtNavMeshQuery* navQuery;
    dtCrowd* crowd = nullptr;
};

