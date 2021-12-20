#pragma once
#include <functional>
#include <vector>

namespace ecs
{
class ISystem
{
  private:
    virtual void execute() = 0;

  public:
};

template <typename... Component_T> class TSystemIterator final
{

    class Iterable final
    {
    };

    using iterable_type = std::tuple<Component_T...>;

  public:
    [[nodiscard]] iterable_type* begin() const
    {
        return nullptr;
    }

    [[nodiscard]] iterable_type* end() const
    {
        return nullptr;
    }
};

template <typename... Component_T> class TSystem final : public ISystem
{
  public:
    TSystem(std::function<void(TSystemIterator<Component_T...>)> iterator)
    {
    }

    void execute() override
    {
    }
};

struct MyComponent1
{
    int x;
};

struct MyComponent2
{
    float y;
};

class SystemRegitry
{
    
  public:
    template<typename...Args, typename Lambda> void add_update(Lambda callback)
    {
        new TSystem<Args...>(callback);



    }
};

struct MyComponent
{
    static void update_comp1_comp2(TSystemIterator<MyComponent1, MyComponent2> iterator)
    {
        for (auto& [comp1, comp2] : iterator)
        {
            comp1.x++;
            comp2.y--;
        }
    }

    static void add_systems(SystemRegitry registry)
    {
        registry.add_update<MyComponent1, MyComponent2>(update_comp1_comp2);
    }
};

} // namespace ecs