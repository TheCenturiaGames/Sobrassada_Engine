#pragma once

class Script
{
  public:
    virtual ~Script() {}

    virtual bool Init()                  = 0;
    virtual void Update(float deltaTime) = 0;
};