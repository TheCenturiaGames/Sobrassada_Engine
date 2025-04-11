#pragma once
#include "ResourceStateMachine.h"

class StateNode : public ImFlow::BaseNode
{
  public:
    StateNode() 
    {
        setTitle("New State");
        setStyle(ImFlow::NodeStyle::cyan());
        inputPin  = addIN<int>("In", 0, ImFlow::ConnectionFilter::SameType());
        outputPin = addOUT<int>("Out");
    }

    void draw() override { ImGui::Text("Clip: %s", state.clipName.GetString().c_str()); }

    std::shared_ptr<ImFlow::InPin<int>> getInputPin() { return inputPin; }
    std::shared_ptr<ImFlow::OutPin<int>> getOutputPin() { return outputPin; }
    const std::string& GetStateName() { return getName(); }
    void SetStateName(const std::string& newName)
    {
        state.name = HashString(newName);
        setTitle(newName);
    }
    void SetClipName(const std::string& newClip) { state.clipName = HashString(newClip); }
    std::string GetClipName() const { return state.clipName.GetString(); }

  private:
    State state;
    std::shared_ptr<ImFlow::InPin<int>> inputPin;
    std::shared_ptr<ImFlow::OutPin<int>> outputPin;
};