#include "NodeEditor.h"



NodeEditor::~NodeEditor()
{
}



bool NodeEditor::RenderEditor()
{
    if (!EngineEditorBase::RenderEditor()) return false; 

    ImFlow::ImNodeFlow myGrid;
    myGrid.update();
   
}

