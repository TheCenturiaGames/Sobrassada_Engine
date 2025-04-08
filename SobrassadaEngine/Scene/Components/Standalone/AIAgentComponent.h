#pragma once
#include "Globals.h"

#include "Component.h"


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
    void setPath(const float3& destination) const;
    void Update(float deltaTime) override;
    void Render(float deltaTime) override;
    void RenderEditorInspector() override;
    void Clone(const Component* other) override;
    void Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const override;

  private:
    float3 agentPosition;
    float speed;
    float radius;
    float height;
    int agentId ; // Assigned by dtCrowd
    bool autoAdd; //automatically add to crowd without user input
};