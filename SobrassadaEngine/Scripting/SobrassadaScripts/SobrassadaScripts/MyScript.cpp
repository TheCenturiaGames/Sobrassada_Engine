#include "pch.h"
#include "MyScript.h"
#include <iostream>
#include "Application.h"
#include "ResourcesModule.h"


bool MyScript::Init()
{
    //App->GetInputModule()
    std::cout << "MyScript initialized!" << std::endl;
    return true;
}

void MyScript::Update(float deltaTime)
{
    std::cout << "Updating MyScript. Delta Time: " << deltaTime << std::endl;
    App->GetResourcesModule()->GetspecularGlossinessProgram();
}
