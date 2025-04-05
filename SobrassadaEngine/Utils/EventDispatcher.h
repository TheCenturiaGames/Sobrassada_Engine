#pragma once

#include "Delegate.h"

#include <map>

/*
    For testing and checking how to use event dispatchers please have a look at this repo with sample code:
    https://github.com/IDarkgenesis/DelegateTesting
*/

template <typename ReturnValue, typename... Arguments> class EventDispatcher
{
  public:
    EventDispatcher() = default;
    ~EventDispatcher() { subscribedCallbacks.clear(); };

    auto SubscribeCallback(Delegate<ReturnValue, Arguments...>&& delegate)
    {
        return subscribedCallbacks.insert(subscribedCallbacks.end(), std::move(delegate));
    };

    void RemoveCallback(typename std::list<Delegate<ReturnValue, Arguments...>>::iterator delegatePosition)
    {
        subscribedCallbacks.erase(delegatePosition);
    }

    void Call(Arguments... args)
    {
        for (auto& callback : subscribedCallbacks)
        {
            callback.Call(args...);
        }
    };

  private:
    std::list<Delegate<ReturnValue, Arguments...>> subscribedCallbacks;
};

template <> class EventDispatcher<void>
{
  public:
    EventDispatcher() = default;
    ~EventDispatcher() { subscribedCallbacks.clear(); };

    auto SubscribeCallback(Delegate<void>&& delegate)
    {
        return subscribedCallbacks.insert(subscribedCallbacks.end(), std::move(delegate));
    };

    void RemoveCallback(std::list<Delegate<void>>::iterator delegatePosition)
    {
        subscribedCallbacks.erase(delegatePosition);
    }

    void Call()
    {
        for (auto& callback : subscribedCallbacks)
        {
            callback.Call();
        }
    };

  private:
    std::list<Delegate<void>> subscribedCallbacks;
};