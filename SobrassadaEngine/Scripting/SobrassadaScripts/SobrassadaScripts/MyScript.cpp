#include "pch.h"
#include "MyScript.h"
#include <iostream>
#include "GlobalsDLL.h"
#include "Application.h"

bool MyScript::Init()
{
    std::cout << "MyScript initialized!" << std::endl;
    return true;
}

void MyScript::Update(float deltaTime)
{
    std::cout << "Updating MyScript. Delta Time: " << deltaTime << std::endl;
}
