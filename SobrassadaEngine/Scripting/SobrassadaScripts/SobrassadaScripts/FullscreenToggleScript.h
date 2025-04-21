#pragma once
#include "Script.h"
#include "Utils/Delegate.h"
#include <list>

class FullscreenToggleScript : public Script
{
  public:
    FullscreenToggleScript(GameObject* parent) : Script(parent) {}
    ~FullscreenToggleScript() override;
    bool Init() override;
    void Update(float deltaTime) override;
    void Inspector() override;

    void OnClick();

  private:
    std::list<Delegate<void>>::iterator delegateID;
    bool hasRegisteredCallback = false;
};
