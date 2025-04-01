#include "pch.h"
#include "MyScript.h"
#include <iostream>
#include <Globals.h>

bool MyScript::Init()
{
    GLOG("HOLA");
    std::cout << "MyScript initialized!" << std::endl;
    return true;
}

void MyScript::Update(float deltaTime)
{
    std::cout << "Updating MyScript. Delta Time: " << deltaTime << std::endl;
}
