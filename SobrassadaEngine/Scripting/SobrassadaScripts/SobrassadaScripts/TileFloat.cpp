#include "TileFloat.h"
#include "Globals.h"

TileFloat::TileFloat(GameObject* parent) : Script(parent)
{
    // Using ImGui in the dll cause problems, so we need to call ImGui outside the dll
    fields.push_back({InspectorField::FieldType::Text, (void*)"Test"});
    fields.push_back({"Speed", InspectorField::FieldType::Float, &speed, 0.0f, 2.0f});

}

bool TileFloat::Init()
{
    GLOG("Initiating TileFloatScript");
    return true;
}