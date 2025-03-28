#pragma once
#include "pch.h"
#include "GlobalsDLL.h"
#include <windows.h>

std::vector<char*>* logsDLL = new std::vector<char*>();
Application* App            = nullptr;

void glogDLL(const char file[], int line, const char* format, ...)
{
    static char tmp_string[4096];
    static char tmp_string2[4096];
    static va_list ap;

    // Construct the string from variable arguments
    va_start(ap, format);
    vsprintf_s(tmp_string, 4096, format, ap);
    va_end(ap);
    sprintf_s(tmp_string2, 4096, "\n%s(%d) : %s", file, line, tmp_string);
#ifdef _DEBUG
    wchar_t wtmp_string2[4096];
    MultiByteToWideChar(CP_UTF8, 0, tmp_string2, -1, wtmp_string2, 4096);
    OutputDebugString(wtmp_string2);
#endif

    // Allocating memory to store logs
    char* newString;
    newString = (char*)malloc(4096);

    if (newString == NULL) return;

    memcpy(newString, tmp_string, 4096 * sizeof(char));

    logsDLL->push_back(newString);
}