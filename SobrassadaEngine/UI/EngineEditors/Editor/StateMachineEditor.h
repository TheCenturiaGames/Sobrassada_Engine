#pragma once

#include "Globals.h"
#include "UI/EngineEditors/EngineEditorBase.h"
#include "ImNodeFlow-1.2.2/include/ImNodeFlow.h"

class ResourceStateMachine;
class StateNode;

class StateMachineEditor : public EngineEditorBase
{
  public:
    StateMachineEditor(const std::string& editorName, UID uid, ResourceStateMachine* stateMachine);
    ~StateMachineEditor() override;

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


  private:
    UID uid;
    ResourceStateMachine* resource = nullptr;
    std::unique_ptr<ImFlow::ImNodeFlow> graph;
    std::vector<std::string> availableClips;
    std::vector<std::string> availableTriggers;
    std::vector<std::shared_ptr<StateNode>> nodes;
    char stateMachineName[128] = "";
    bool alreadySaved          = false;
    StateNode* selectedNode = nullptr;
    bool saveInitialized    = false;
    int selectedIndex = -1;
    std::vector<std::string> allStateMachineNames;
};