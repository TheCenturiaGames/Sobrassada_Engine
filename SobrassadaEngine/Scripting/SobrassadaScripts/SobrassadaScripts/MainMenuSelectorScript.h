#pragma once
#include "Script.h"
#include <string>
#include <vector>

class GameObject;

class MainMenuSelectorScript : public Script
{
  public:
    MainMenuSelectorScript(GameObject* parent) : Script(parent) {}

    bool Init() override;
    void Update(float deltaTime) override;
    void Inspector() override {}

  private:
    std::vector<GameObject*> menuItems;
    std::vector<GameObject*> arrowImages;
    int selectedIndex = 0;

    void UpdateSelection();
};
