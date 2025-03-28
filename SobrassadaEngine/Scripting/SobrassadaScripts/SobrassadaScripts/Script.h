#pragma once
#include "GlobalsDLL.h"

class Script
{
  public:
    Script() {}
    virtual ~Script() {}

    virtual bool Init()                  = 0;
    virtual void Update(float deltaTime) = 0;
};