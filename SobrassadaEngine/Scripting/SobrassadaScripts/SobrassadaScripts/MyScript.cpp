#include "pch.h"
#include "MyScript.h"
#include <iostream>

bool MyScript::Init()
{
    std::cout << "MyScript initialized!" << std::endl;
    return true;
}

void MyScript::Update(float deltaTime)
{
    std::cout << "Updating MyScript. Delta Time: " << deltaTime << std::endl;
}
