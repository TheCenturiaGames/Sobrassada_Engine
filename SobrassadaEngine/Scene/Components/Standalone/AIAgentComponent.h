#pragma once

#include "Component.h"

#include <vector>

class AIAgentComponent : public Component
{

  public:
    AIAgentComponent(UID uid, GameObject* parent);
    AIAgentComponent(const rapidjson::Value& initialState, GameObject* parent);
    ~AIAgentComponent() override;

    void Update(float deltaTime) override;
    void Render(float deltaTime) override;
    void RenderEditorInspector() override;
    void Clone(const Component* other) override;
    void Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const override;
    void AddToCrowd();
    void RecreateAgent();
    void SetPath(const float3& destination) const;

  private:
    float speed  = 0.f;
    float radius = 0.f;
    float height = 0.f;
    int agentId  = -1; // Assigned by dtCrowd
};