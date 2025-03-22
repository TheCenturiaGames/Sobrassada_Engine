#pragma once

#include "Globals.h"
#include "UI/EngineEditors/EngineEditorBase.h"
#include "ResourceManagement/Resources/ResourceStateMachine.h"
#include "Libs/ImNodeFlow-1.2.2/include/ImNodeFlow.h"

class StateMachineEditor : public EngineEditorBase
{
  public:
    StateMachineEditor(const std::string& editorName, UID uid, ResourceStateMachine* stateMachine);
    ~StateMachineEditor() override;

  private:
    bool RenderEditor() override;
    void BuildGraph();

  private:
    UID uid;
    ResourceStateMachine* resource = nullptr;
    std::unique_ptr<ImFlow::ImNodeFlow> graph;

};