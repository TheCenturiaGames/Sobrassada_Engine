// SobrassadaScripts.cpp : Define the exported functions from the DLL.
// That file comes by default in the project, but it is not necessary to use it. We can delete it I think, but Im not sure.

#include "pch.h"
#include "framework.h"
#include "SobrassadaScripts.h"


// Example of exported variable
SOBRASSADA_API int nSobrassadaScripts = 0;

// Example of exported function
SOBRASSADA_API int fnSobrassadaScripts(void)
{
    return 0;
}

// Constructor of exported class
CSobrassadaScripts::CSobrassadaScripts()
{
    return;
}
