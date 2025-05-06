#pragma once

#include <functional>

template <typename ReturnValue, typename... Arguments> class Delegate
{
  public:
    Delegate() = default;

    Delegate(std::function<ReturnValue(Arguments...)>&& newCallback) { callback = std::move(newCallback); }

    Delegate(const std::function<ReturnValue(Arguments...)>& newCallback) { callback = newCallback; }

    ~Delegate() = default;

    ReturnValue Call(Arguments... args) { return callback(args...); }

    template <typename T> static Delegate FromMethod(T* instance, ReturnValue (T::*method)(Arguments...))
    {
        return Delegate([=](Arguments... args) { return (instance->*method)(args...); });
    }

    static Delegate FromFunction(std::function<ReturnValue(Arguments...)> func) { return Delegate(std::move(func)); }

  private:
    std::function<ReturnValue(Arguments...)> callback;
};
