#pragma once

#include <functional>

/*
    For testing and checking how to use delegates please have a look at this repo with sample code:
    https://github.com/IDarkgenesis/DelegateTesting
*/

template <typename ReturnValue, typename... Arguments> class Delegate
{
  public:
    Delegate() = default;

    Delegate(std::function<ReturnValue(Arguments...)>& newCallback) { callback = newCallback; };

    Delegate(std::function<ReturnValue(Arguments...)>&& newCallback) { callback = std::move(newCallback); }

    ~Delegate() = default;

    ReturnValue Call(Arguments... args) { return callback(args...); }

  private:
    std::function<ReturnValue(Arguments...)> callback;
};