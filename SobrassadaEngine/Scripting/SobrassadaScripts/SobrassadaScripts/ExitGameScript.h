#pragma once
#include "Script.h"
#include "Delegate.h"
#include <list>

class ExitGameScript : public Script
{
  public:
    ExitGameScript(GameObject* parent) : Script(parent) {}
    ~ExitGameScript() override;
    bool Init() override;
    void Update(float deltaTime) override;
    void Inspector() override;

    void OnClick();

  private:
    std::list<Delegate<void>>::iterator delegateID;
    bool hasRegisteredCallback = false;
};
