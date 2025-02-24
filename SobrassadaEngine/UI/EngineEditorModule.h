#pragma once
#include "Module.h"
#include <string>
class EngineEditorModule : public Module
{
public:
    EngineEditorModule(const std::string& editorName): name(editorName) {};
    ~EngineEditorModule(){};
    void RenderEditor(bool &engineEditorWindow);
    
protected:
    std::string name = "EngineEditor";


    
};
