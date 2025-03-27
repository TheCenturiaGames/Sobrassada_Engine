#pragma once

#ifdef SOBRASSADASCRIPTS_EXPORTS
#define SOBRASSADA_API __declspec(dllexport)
#else
#define SOBRASSADA_API __declspec(dllimport)
#endif

class Application;

//We need to declare the application to be used in the game dll
Application* App = nullptr;
extern "C" SOBRASSADA_API void setApplication(Application* application)
{
    App = application;
}