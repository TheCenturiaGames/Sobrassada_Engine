#pragma once

#include "Globals.h"
#include "EngineEditorBase.h"
#include "ImNodeFlow.h"

class ResourceStateMachine;
class StateNode;
class AnimationComponent;

class StateMachineEditor : public EngineEditorBase
{
  public:
    StateMachineEditor(const std::string& editorName, UID uid, ResourceStateMachine* stateMachine);
    ~StateMachineEditor() override;
    ResourceStateMachine* GetLoadedStateMachine() { return resource; }
    void SetAnimComponent(AnimationComponent* animComponent) { this->animComponent = animComponent; }

  private:
    bool RenderEditor() override;
    void BuildGraph();
    void DetectNewTransitions();
    void CreateBaseState(StateNode& node);
    void SaveMachine();
    void ShowSavePopup();
    void LoadMachine();
    void ShowLoadPopup();
    void RemoveStateNode(StateNode& node);
    void ShowInspector();
    void DeleteStateResource(StateNode& node);
    const void ShowTriggers();
    const void ShowTriggersPopup();


  private:
    UID uid;
    int stateCont                  = 0;
    ResourceStateMachine* resource = nullptr;
    AnimationComponent* animComponent = nullptr;
    std::unique_ptr<ImFlow::ImNodeFlow> graph;
    std::vector<std::string> availableClips;
    std::vector<std::shared_ptr<StateNode>> nodes;
    char stateMachineName[128] = "";
    bool alreadySaved          = false;
    StateNode* selectedNode = nullptr;
    bool saveInitialized    = false;
    int selectedIndex = -1;
    std::vector<std::string> allStateMachineNames;
    char newTriggerName[64] = "";

};