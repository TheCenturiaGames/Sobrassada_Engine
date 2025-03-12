
#include "Globals.h"
#include <string>

class EngineEditorBase
{
  public:
    EngineEditorBase(const std::string& editorName, const UID uid) : name(editorName), uid(uid) {}

    virtual ~EngineEditorBase() {}
    virtual bool RenderEditor();

   

    UID GetUID() const { return uid; }

  protected:
    std::string name = "EngineEditor";
    UID uid;
};