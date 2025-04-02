#pragma once
#include "Globals.h"

#include "ResourceManagement/Resources/ResourceNavmesh.h"
#include "Scene/Components/Component.h"


#include <vector>

class AIAgentComponent : public Component
{

  public:
    AIAgentComponent(UID uid, GameObject* parent, float3 position, float moveSpeed);
    AIAgentComponent(const rapidjson::Value& initialState, GameObject* parent);
    ~AIAgentComponent() override;

    void SetAgentPosition(float3 agentPosition) { agentPosition = agentPosition; }
    void setPath(const float3& destination); 
    void Update(float deltaTime) override;
    virtual void Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const override;

  private:
    float3 agentPosition;
    float speed;
};