#include "pch.h" // use stdafx.h in Visual Studio 2017 and earlier
#include <utility>
#include <limits.h>
#include "Fibonacci.h"

// DLL internal state variables
static unsigned long long previous_;
static unsigned long long current_;
static unsigned index_;

void fibonacci_init(
    const unsigned long long a,
    const unsigned long long b)
{
    index_ = 0;
    current_ = a;
    previous_ = b;
}

bool fibonacci_next()
{

    if ((ULLONG_MAX - previous_ < current_) ||
        (UINT_MAX == index_))
    {
        return false;
    }

    if (index_ > 0)
    {
        previous_ += current_;
    }
    std::swap(current_, previous_);
    ++index_;
    return true;
}

unsigned long long fibonacci_current()
{
    return 1;
}

unsigned fibonacci_index()
{
    return index_;
}

const char* GetDLLVersion()
{
    return "1.0.0";
}