#pragma once
#include "cpputils/logger.hpp"

#include <unordered_set>
#include <vector>

class IEngineInterface;
class WindowBase;

class WindowBase
{
    friend class WindowManager;

  public:
    void close();

  protected:
    WindowBase()
    {
    }
    virtual ~WindowBase();

    virtual void draw_content() = 0;

  private:
    void draw();

    bool                     open = true;
    const char*              window_name;
    size_t                   window_id;
    WindowBase*              parent_window = nullptr;
    std::vector<WindowBase*> children;
};

class WindowManager final
{
    friend WindowBase;

  public:
    ~WindowManager();
    void draw();

    template <typename Window_T, typename... Args> static Window_T* create(const char* name, WindowBase* parent, Args&&... arguments)
    {
        LOG_INFO("open window %s", name);
        Window_T* window_memory      = static_cast<Window_T*>(malloc(sizeof(Window_T)));
        window_memory->open          = true;
        window_memory->window_name   = name;
        window_memory->parent_window = parent;
        if (parent)
        {
            parent->children.push_back(window_memory);
        }

        new (window_memory) Window_T(std::forward<Args>(arguments)...);

        if (!window_memory->window_name)
        {
            LOG_ERROR("don't call WindowBase() constructor in children class : %s", typeid(Window_T).name());
            delete window_memory;
            window_memory = nullptr;
        }

        add_window_memory_internal(window_memory);
        return window_memory;
    }

    [[nodiscard]] static WindowManager* get();

  private:
    static void add_window_memory_internal(WindowBase* memory);
    void add_window(WindowBase* window);
    void remove_window(WindowBase* window);

    std::vector<WindowBase*>   windows;
    std::unordered_set<size_t> window_ids;
};

class DemoWindow : public WindowBase
{
  public:
    DemoWindow()
    {
    }

  protected:
    void draw_content() override;
};