

#include "engine_interface.h"

#include "assets/asset_base.h"
#include "game_engine.h"
#include "rendering/graphics.h"
#include "rendering/renderer/renderer.h"
#include "rendering/swapchain_config.h"
#include "rendering/vulkan/common.h"
#include "statsRecorder.h"

static std::shared_ptr<IEngineInterface> engine_interface_reference;

void IEngineInterface::close()
{
    glfwSetWindowShouldClose(Graphics::get()->get_glfw_handle(), true);
}

GfxInterface* IEngineInterface::create_graphic_interface()
{
    return new GfxInterface();
}

InputManager* IEngineInterface::create_input_manager()
{
    return new InputManager(Graphics::get()->get_glfw_handle());
}

void IEngineInterface::enable_logs(uint32_t log_level)
{
    Logger::get().enable_logs(log_level);
}

void IEngineInterface::disable_logs(uint32_t log_level)
{
    Logger::get().disable_logs(log_level);
}

IEngineInterface::IEngineInterface()
{
}

void IEngineInterface::store(const std::shared_ptr<IEngineInterface>& in_engine_interface)
{
    engine_interface_reference = in_engine_interface;
}

IEngineInterface* IEngineInterface::get_internal()
{
    return engine_interface_reference.get();
}

void IEngineInterface::run_main_task(const WindowParameters& window_parameters)
{
    // Initialize engine
    GameEngine::init();

    AssetManager::initialize<AssetManager>();

    // Create graphic context
    Graphics::create(create_graphic_interface(), window_parameters);

    // Create input manager
    input_manager = std::unique_ptr<InputManager>(create_input_manager());

    // Load resource
    engine_load_resources();

    do
    {
        // poll inputs
        input_manager->poll_events(get_delta_second());

        // Compute delta second
        const auto now         = std::chrono::steady_clock::now();
        delta_second           = static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(now - last_delta_second_time).count()) / 1000000000.0;
        last_delta_second_time = now;

        engine_tick(delta_second);
        AssetManager::get()->try_delete_dirty_items();

        // render scene
        auto swapchain_frame = Graphics::get()->begin_frame();
        Graphics::get()->get_renderer()->render_frame(swapchain_frame);
        Graphics::get()->end_frame(swapchain_frame);
    } while (!glfwWindowShouldClose(Graphics::get()->get_glfw_handle()));

    vkDeviceWaitIdle(Graphics::get()->get_logical_device());

    engine_unload_resources();
    AssetManager::destroy();

    // Destroy graphic context
    Graphics::destroy();

    // Shutdown engine
    GameEngine::cleanup();
}
