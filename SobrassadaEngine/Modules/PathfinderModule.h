#pragma once

#include "Globals.h"
#include "Module.h"

class dtNavMesh;
class dtNavMeshQuery;
class dtCrowd;
class ResourceNavMesh;

class PathfinderModule : public Module
{
  public:
    PathfinderModule();
    ~PathfinderModule();
    bool Init() override;
    update_status Update(float deltaTime) override;

    //std::vector<float3> FindPath(float3 start, float3 end);
    int CreateAgent(float3 position, float radius, float height, float speed);
    dtCrowd* GetCrowd() const { return crowd; }
    dtNavMeshQuery* GetNavQuery() const { return navQuery; }
    void RemoveAgent(int agentId);
    void InitQuerySystem();
    void HandleClickNavigation(); // Perform raycast and set destination

  private:
    dtNavMeshQuery* navQuery = nullptr;
    dtCrowd* crowd     = nullptr;
    ResourceNavMesh* navmesh = nullptr;
    const unsigned int maxAgents = 100;
    const float maxAgentRadius = 0.5f;
    bool clickNavigationEnabled  = false;
};
