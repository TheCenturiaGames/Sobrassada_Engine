#pragma once

#include "Component.h"

class Transform2DComponent : public Component
{
  public:
    Transform2DComponent(UID uid, GameObject* parent);
    ~Transform2DComponent();

    void Update(float deltaTime) override;
    void Render(float deltaTime) override;
    void Clone(const Component* otherComponent) override;
	  
  private:
    float x;
    float y;
    float width;
    float height;
};