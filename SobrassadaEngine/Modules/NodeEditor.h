#pragma once
#include "EngineEditorBase.h"
#include "Globals.h"
#include "imgui.h"
#include "Libs/ImNodeFlow-1.2.2/include/ImNodeFlow.h"
#include <vector>
#include <string>



class NodeEditor : public EngineEditorBase
{
  public:
    NodeEditor(const std::string& editorName, UID uid)
        : EngineEditorBase(editorName, uid),
          myGrid(std::make_unique<ImFlow::ImNodeFlow>("FlowGrid" + std::to_string(uid)))
    {
    }
    ~NodeEditor();
  

    private:
    bool RenderEditor() override;
    
  
    

  private:
    UID uid;
    std::unique_ptr<ImFlow::ImNodeFlow> myGrid;
};
