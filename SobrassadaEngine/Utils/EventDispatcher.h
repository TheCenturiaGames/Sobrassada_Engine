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

    int SubscribeCallback(Delegate<ReturnValue, Arguments...>&& delegate)
    {
        subscribedCallbacks.insert({++lastID, std::move(delegate)});

        return lastID;
    };

    void RemoveCallback(int id)
    {
        auto iterator = subscribedCallbacks.find(id);
        if (iterator != subscribedCallbacks.end()) subscribedCallbacks.erase(iterator);
    }

    void Call(Arguments... args)
    {
        for (auto& callback : subscribedCallbacks)
        {
            callback.second.Call(args...);
        }
    };

  private:
    std::map<int, Delegate<ReturnValue, Arguments...>> subscribedCallbacks;
    int lastID = -1;
};

template <> class EventDispatcher<void>
{
  public:
    EventDispatcher() = default;
    ~EventDispatcher() { subscribedCallbacks.clear(); };

    int SubscribeCallback(Delegate<void>&& delegate)
    {
        subscribedCallbacks.insert({++lastID, std::move(delegate)});

        return lastID;
    };

    void RemoveCallback(int id)
    {
        auto iterator = subscribedCallbacks.find(id);
        if (iterator != subscribedCallbacks.end()) subscribedCallbacks.erase(iterator);
    }

    void Call()
    {
        for (auto callback : subscribedCallbacks)
        {
            callback.second.Call();
        }
    };

  private:
    std::map<int, Delegate<void>> subscribedCallbacks;
    int lastID = -1;
};