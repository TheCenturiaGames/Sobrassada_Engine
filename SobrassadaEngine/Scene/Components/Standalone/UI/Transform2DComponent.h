#pragma once

#include "Component.h"

class Transform2DComponent : public Component
{
  public:
    Transform2DComponent(UID uid, GameObject* parent);
    ~Transform2DComponent();
	  
  private:
    float x;
    float y;
    float width;
    float height;
};