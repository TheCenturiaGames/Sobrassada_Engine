#pragma once
#include "Globals.h"

#include "ResourceManagement/Resources/ResourceNavmesh.h"
#include "Scene/Components/Component.h"


#include <vector>

class AIAgentComponent : public Component
{

  public:
    AIAgentComponent(UID uid, GameObject* parent);
    AIAgentComponent(const rapidjson::Value& initialState, GameObject* parent);
    ~AIAgentComponent() override;

    void AddToCrowd();
    void RecreateAgent();
    void SetAgentPosition(float3 agentPosition) { agentPosition = agentPosition; }
    void setPath(const float3& destination); 
    void Update(float deltaTime) override;
    virtual void Render(float deltaTime) override;
    virtual void Clone(const Component* other) override;
    virtual void Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const override;

  private:
    float3 agentPosition;
    float speed;
    float radius;
    float height;
    int agentId ; // Assigned by dtCrowd
    bool autoAdd; //automatically add to crowd without user input
};