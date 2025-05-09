#pragma once

#include "Globals.h"

void glog(const char file[], int line, const char* format, ...)
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
    static wchar_t wide_char[4096];
    MultiByteToWideChar(CP_ACP, 0, tmp_string2, -1, wide_char, 4096);
    OutputDebugString(wide_char);
#endif

    // Allocating memory to store logs
    char* newString;
    newString = (char*)malloc(4096);

    if (newString == NULL) return;

    memcpy(newString, tmp_string, 4096 * sizeof(char));

    if (Logs->size() >= 200)
    {
        free((*Logs)[0]);
        Logs->erase(Logs->begin());
    }

    Logs->push_back(newString);
}