#pragma once

#include <functional>

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