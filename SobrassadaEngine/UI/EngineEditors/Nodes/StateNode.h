#pragma once
#include "Libs/ImNodeFlow-1.2.2/include/ImNodeFlow.h"
#include "ResourceManagement/Resources/ResourceStateMachine.h"

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

  private:
    State state;
    std::shared_ptr<ImFlow::InPin<int>> inputPin;
    std::shared_ptr<ImFlow::OutPin<int>> outputPin;
};