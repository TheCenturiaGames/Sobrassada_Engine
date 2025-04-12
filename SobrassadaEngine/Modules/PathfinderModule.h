#pragma once

#include "Module.h"
#include <unordered_map>

class dtNavMesh;
class dtNavMeshQuery;
class dtCrowd;
class ResourceNavMesh;
class AIAgentComponent;

class PathfinderModule : public Module
{
  public:
    PathfinderModule();
    ~PathfinderModule();
    bool Init() override;
    update_status Update(float deltaTime) override;

    int CreateAgent(const float3& position, const float radius, const float height, const float speed);
    dtCrowd* GetCrowd() const { return crowd; }
    dtNavMeshQuery* GetNavQuery() const { return navQuery; }
    void RemoveAgent(int agentId);
    void InitQuerySystem();
    void HandleClickNavigation(); // Perform raycast and set destination
    bool RaycastToGround(const LineSegment& ray, float3& outHitPoint);
    void RenderCrowdEditor();

    void AddAIAgentComponent(int agentId, AIAgentComponent* comp);
    void RemoveAIAgentComponent(int agentId);
    AIAgentComponent* GetComponentFromAgentId(int agentId);

  private:
    dtNavMeshQuery* navQuery = nullptr;
    dtCrowd* crowd     = nullptr;
    ResourceNavMesh* navmesh = nullptr;
    const unsigned int maxAgents = 100;
    const float maxAgentRadius = 0.5f;
    bool clickNavigationEnabled  = false;

    float3 outHitPoint;

    std::unordered_map<int, AIAgentComponent*> agentComponentMap;
};
