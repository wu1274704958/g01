#pragma once

#include <vector>
#include <algorithm>
#include <type_traits>
#include <concepts>

/**
 * Concept: checks that Args... exactly match the parameters of a member function pointer of T.
 *
 * Example:
 *   MatchesMemberFunc<IMyListener, &IMyListener::OnEvent, int, float>
 *   is true only if OnEvent has signature void(int, float).
 */
template<typename T, auto MemberFuncPtr, typename... Args>
concept MatchesMemberFunc = requires(T* obj, Args&&... args)
{
    (obj->*MemberFuncPtr)(std::forward<Args>(args)...);
};

/**
 * EventListenerManager<T>
 *
 * A utility for registering/unregistering listeners that implement interface T,
 * and batch-dispatching calls to any member function of T.
 *
 * T must be a pure interface (abstract class with virtual functions).
 *
 * Usage:
 *   EventListenerManager<IMyListener> Manager;
 *   Manager.AddListener(&myListener);
 *   Manager.Dispatch(&IMyListener::OnEvent, 42, 3.14f);
 *   Manager.RemoveListener(&myListener);
 */
template<typename T>
requires std::is_class_v<T>
class EventListenerManager
{
public:
    /**
     * Register a listener. Ignores nullptr or duplicates.
     */
    void AddListener(T* Listener)
    {
        if (!Listener) return;
        if (Contains(Listener)) return;
        Listeners.push_back(Listener);
    }

    /**
     * Unregister a listener.
     */
    void RemoveListener(T* Listener)
    {
        Listeners.erase(
            std::remove(Listeners.begin(), Listeners.end(), Listener),
            Listeners.end()
        );
    }

    /**
     * Returns true if the listener is already registered.
     */
    bool Contains(T* Listener) const
    {
        return std::find(Listeners.begin(), Listeners.end(), Listener) != Listeners.end();
    }

    /**
     * Remove all listeners.
     */
    void Clear()
    {
        Listeners.clear();
    }

    /**
     * Returns the number of registered listeners.
     */
    int32_t Num() const
    {
        return static_cast<int32_t>(Listeners.size());
    }

    /**
     * Dispatch a call to all registered listeners.
     *
     * @param MemberFunc  A member function pointer of T, e.g. &IMyListener::OnEvent
     * @param args        Arguments forwarded to the member function.
     *
     * The concept MatchesMemberFunc enforces at compile time that Args...
     * are compatible with MemberFunc's parameter list.
     *
     * Example:
     *   Manager.Dispatch(&IMyListener::OnEvent, 42, 3.14f);
     */
    template<auto MemberFunc, typename... Args>
    requires MatchesMemberFunc<T, MemberFunc, Args...>
    void Dispatch(Args&&... args)
    {
        // Iterate over a copy to guard against removals during dispatch
        auto Snapshot = Listeners;
        for (T* Listener : Snapshot)
        {
            (Listener->*MemberFunc)(std::forward<Args>(args)...);
        }
    }

    /**
     * Overload: accepts the member function pointer as first argument
     * for a more explicit call style:
     *   Manager.Dispatch(&IMyListener::OnEvent, 42, 3.14f);
     *
     * Note: both overloads work; prefer the template-argument version for
     * stricter compile-time checking.
     */
    template<typename Ret, typename... Params, typename... Args>
    requires (sizeof...(Params) == sizeof...(Args)) &&
             (std::convertible_to<Args, Params> && ...)
    void Dispatch(Ret (T::* MemberFunc)(Params...), Args&&... args)
    {
        auto Snapshot = Listeners;
        for (T* Listener : Snapshot)
        {
            (Listener->*MemberFunc)(std::forward<Args>(args)...);
        }
    }

private:
    std::vector<T*> Listeners;
};

