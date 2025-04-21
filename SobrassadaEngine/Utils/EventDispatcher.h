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

    bool IsValidCallback(std::list<Delegate<void>>::iterator it) const
    {
        for (auto iter = subscribedCallbacks.begin(); iter != subscribedCallbacks.end(); ++iter)
        {
            if (iter == it) return true;
        }
        return false;
    }

    void SafeRemoveCallback(std::list<Delegate<void>>::iterator delegatePosition)
    {
        if (IsValidCallback(delegatePosition))
        {
            subscribedCallbacks.erase(delegatePosition);
        }
        else
        {
            GLOG("[WARNING] Tried to remove invalid or stale delegate iterator!");
        }
    }


  private:
    std::list<Delegate<void>> subscribedCallbacks;
};