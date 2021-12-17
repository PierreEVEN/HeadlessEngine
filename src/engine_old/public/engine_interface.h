#pragma once
#include "ios/input_manager.h"
#include "rendering/graphics.h"
#include "ui/window/window_base.h"

#include <memory>

class InputManager;
class PlayerController;

class IEngineInterface
{
  public:
    template <typename IEngineInterface_T = IEngineInterface> static IEngineInterface_T* get()
    {
        return static_cast<IEngineInterface_T*>(get_internal());
    }

    template <typename Interface_T> static void run(WindowParameters window_parameters = {})
    {
        auto new_interface = std::make_shared<Interface_T>();
        store(new_interface);
        new_interface->run_main_task(window_parameters);
    }

    [[nodiscard]] InputManager* get_input_manager() const
    {
        return input_manager.get();
    }

    [[nodiscard]] double get_delta_second() const
    {
        return delta_second;
    }

    void close();

    static void enable_logs(uint32_t log_level);
    static void disable_logs(uint32_t log_level);

    virtual RendererConfiguration get_default_render_pass_configuration() = 0;

  protected:
    virtual GfxInterface* create_graphic_interface();
    virtual InputManager* create_input_manager();

    virtual void engine_load_resources()        = 0;
    virtual void engine_unload_resources()      = 0;
    virtual void engine_tick(double delta_time) = 0;

    IEngineInterface();

  private:
    static void              store(const std::shared_ptr<IEngineInterface>& in_engine_interface);
    static IEngineInterface* get_internal();
    void                     run_main_task(const WindowParameters& window_parameters);

    double                                delta_second           = 0.0;
    std::chrono::steady_clock::time_point last_delta_second_time = std::chrono::steady_clock::now();
    std::unique_ptr<InputManager>         input_manager          = nullptr;
};
