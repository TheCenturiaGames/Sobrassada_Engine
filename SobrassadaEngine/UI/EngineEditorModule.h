#pragma once
#include "Module.h"

class EngineEditorModule : public Module
{
public:
    EngineEditorModule(const char* editorName): name(editorName) {};
    ~EngineEditorModule(){};
    void RenderEditor(bool &engineEditorWindow);
protected:
    const char* name = "EngineEditor";


    
};
