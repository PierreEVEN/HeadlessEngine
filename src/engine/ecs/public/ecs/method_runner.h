#pragma once
#include <type_traits>

namespace ecs
{
// IRunner : component-independent interface
template <typename... Args_T> class IMethodRunner
{
  public:
    virtual ~IMethodRunner()                                              = default;
    virtual void execute(void* instances, size_t count, Args_T... args) = 0;
};

// TRunner : execute a given method on a vector of components
template <typename Component_T, typename... Args_T> class TMethodRunner final : public IMethodRunner<Args_T...>
{
  public:
    using Method = void (Component_T::*)(Args_T...);

    TMethodRunner(const Method render_func) : method(render_func)
    {
    }

  private:
    void execute(void* start_ptr, size_t count, Args_T... args) override
    {
        auto* current_component = static_cast<Component_T*>(start_ptr);

        for (auto* end_component = static_cast<Component_T*>(start_ptr) + count; current_component != end_component; current_component += 1)
            (current_component->*method)(args...);
    }

    Method method;
};

}
