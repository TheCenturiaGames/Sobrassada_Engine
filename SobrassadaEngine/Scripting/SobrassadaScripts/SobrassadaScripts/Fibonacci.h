#pragma once
#include <iostream>

#ifdef SOBRASSADASCRIPTS_EXPORTS
#define SOBRASSADA_API __declspec(dllexport)
#else
#define SOBRASSADA_API __declspec(dllimport)
#endif

extern "C" SOBRASSADA_API void fibonacci_init(
    const unsigned long long a, const unsigned long long b);

extern "C" SOBRASSADA_API bool fibonacci_next();

extern "C" SOBRASSADA_API unsigned long long fibonacci_current();

extern "C" SOBRASSADA_API unsigned fibonacci_index();

extern "C" SOBRASSADA_API const char* GetDLLVersion();