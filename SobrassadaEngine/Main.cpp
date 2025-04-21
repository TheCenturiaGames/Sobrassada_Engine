#include "Application.h"
#include "Globals.h"

#include "SDL.h"
#pragma comment(lib, "Libs/SDL/lib/SDL2.lib")
#pragma comment(lib, "Libs/SDL/lib/SDL2main.lib")

#ifdef OPTICK
#include "optick.h"
#endif

extern "C"
{
    _declspec(dllexport) DWORD NvOptimusEnablement                = 1;
    _declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

enum MainState
{
    MAIN_CREATION,
    MAIN_START,
    MAIN_UPDATE,
    MAIN_FINISH,
    MAIN_EXIT
};

Application* App         = NULL;
std::vector<char*> *Logs = NULL;
LCG* rng;

int  main(int argc, char** argv)
{
    int mainReturn      = EXIT_SUCCESS;
    MainState mainState = MAIN_CREATION;

    Logs                = new std::vector<char*>();
    rng                 = new LCG();

    while (mainState != MAIN_EXIT)
    {
#ifdef OPTICK
        OPTICK_FRAME("Main application update");
#endif
        switch (mainState)
        {
        case MAIN_CREATION:

            GLOG("----- Application Creation -----");
            App       = new Application();
            mainState = MAIN_START;
            break;

        case MAIN_START:

            GLOG("----- Application Init -----");
            if (App->Init() == false)
            {
                GLOG("----- Application Init exits with error -----");
                mainState = MAIN_EXIT;
            }
            else
            {
                mainState = MAIN_UPDATE;
                GLOG("----- Application Update -----");
            }

            break;

        case MAIN_UPDATE:
        {
            int update_return = App->Update();

            if (update_return == UPDATE_ERROR)
            {
                GLOG("----- Application Update exits with error -----");
                mainState = MAIN_EXIT;
            }

            if (update_return == UPDATE_RESTART)
            {
                mainState = MAIN_CREATION;
                App->ShutDown();
                delete App;
                // Free memory from log*
                for (auto log : *Logs)
                {
                    free(log);
                }
                Logs->clear();
            }

            if (update_return == UPDATE_STOP) mainState = MAIN_FINISH;
        }
        break;

        case MAIN_FINISH:

            GLOG("----- Application ShutDown -----");
            if (App->ShutDown() == false)
            {
                GLOG("----- Application ShutDown exits with error -----");
            }
            else mainReturn = EXIT_SUCCESS;

            mainState = MAIN_EXIT;

            break;
        }
    }

    delete App;

    // Free memory from log*
    for (auto log : *Logs)
    {
        free(log);
    }
    Logs->clear();
    delete Logs;
    delete rng;

    return mainReturn;
}