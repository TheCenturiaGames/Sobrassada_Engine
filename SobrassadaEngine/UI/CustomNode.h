#pragma once
#include "Libs/ImNodeFlow-1.2.2/include/ImNodeFlow.h"
class CustomNode : public ImFlow::BaseNode
{
  public:
    CustomNode()
    {
        inputPin  = addIN<float>("Input", 1.0f, [](ImFlow::Pin*, ImFlow::Pin*) { return true; });
        outputPin = addOUT<float>("Output");

       
        outputPin->behaviour([this]() { return this->getInVal<float>("Input") * 2.0f; });
       
    }

    void draw() override
    {
      
        showIN<float>("Input", 1.0f, [](ImFlow::Pin*, ImFlow::Pin*) { return true; });
        showOUT<float>("Output", [this]() { return this->getInVal<float>("Input") * 2.0f; });
    }
    std::shared_ptr<ImFlow::InPin<float>> getInputPin() { return inputPin; }
    std::shared_ptr<ImFlow::OutPin<float>> getOutputPin() { return outputPin; }

  private:
    std::shared_ptr<ImFlow::InPin<float>> inputPin;
    std::shared_ptr<ImFlow::OutPin<float>> outputPin;

};
