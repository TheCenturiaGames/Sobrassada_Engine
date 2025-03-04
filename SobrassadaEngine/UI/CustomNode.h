#pragma once
#include "Libs/ImNodeFlow-1.2.2/include/ImNodeFlow.h"
class CustomNode : public ImFlow::BaseNode
{
    explicit CustomNode()
    {
        setTitle("I'm custom");
        setStyle(ImFlow:: NodeStyle::brown());
        addIN<int>("I'm input", 0, 0, ImFlow:: PinStyle::red());
    }
};
