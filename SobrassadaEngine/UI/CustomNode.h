#pragma once
#include "Libs/ImNodeFlow-1.2.2/include/ImNodeFlow.h"
class CustomNode : public ImFlow::BaseNode
{
  public:
    CustomNode()
    {
        setTitle("Simple sum");
        setStyle(ImFlow::NodeStyle::green());
        inputPin = addIN<int>("IN_VAL", 0, ImFlow::ConnectionFilter::SameType());
        outputPin = addOUT<int>("OUT_VAL");
        outputPin->behaviour([this]() { return getInVal<int>("IN_VAL") + m_valB; });
       
    }

    void draw() override
    {
      
        ImGui::SetNextItemWidth(100.f);
        ImGui::InputInt("##ValB", &m_valB);

         
         ImGui::Text("Resultado: %d",getInVal<int>("IN_VAL") + m_valB);
     

    }

   

    std::shared_ptr<ImFlow::InPin<int>> getInputPin() { return inputPin; }
    std::shared_ptr<ImFlow::OutPin<int>> getOutputPin() { return outputPin; }

  private:
    int m_valB = 0;
    std::shared_ptr<ImFlow::InPin<int>> inputPin;
    std::shared_ptr<ImFlow::OutPin<int>> outputPin;

};
