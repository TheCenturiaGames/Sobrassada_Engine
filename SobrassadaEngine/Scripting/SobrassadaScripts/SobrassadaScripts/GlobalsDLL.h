#pragma once

#include <vector>

#ifdef SOBRASSADASCRIPTS_EXPORTS
#define SOBRASSADA_API __declspec(dllexport)
#else
#define SOBRASSADA_API __declspec(dllimport)
#endif

class Application;
extern Application* App;

struct LogEntry
{
    char* message;
    float timestamp;
};

extern std::vector<LogEntry>* logsDLL;
void glogDLL(const char file[], int line, const char* format, ...);
#define GLOG(format, ...) glogDLL(__FILE__, __LINE__, format, __VA_ARGS__);