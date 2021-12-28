#include "application/application.h"
#include "application/window.h"
#include "gfx/texture.h"

#include <gfx/gfx.h>

#include <cpputils/logger.hpp>

int main()
{
    Logger::get().enable_logs(Logger::LogType::LOG_LEVEL_INFO | Logger::LogType::LOG_LEVEL_DEBUG);

    /**
     * 1° initialize the application and the gfx backend
     */
    application::create();
    gfx::init();

    /**
     * 2° create some windows with surfaces (surfaces are layers that allow rendering images from the gfx backend onto application window)
     */
    auto* window_1  = create_window(application::window::WindowConfig{.name = application::get_full_name(), .window_style = application::window::EWindowStyle::WINDOWED});
    auto  surface_1 = std::unique_ptr<gfx::Surface>(gfx::Surface::create_surface(window_1));

    std::unique_ptr<gfx::RenderTarget> main_render_target = std::make_unique<gfx::RenderTarget>();
    std::unique_ptr<gfx::View>         main_view          = std::make_unique<gfx::View>();

    /**
     * 3° Load some data on the GPU
     */
    int32_t data[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};

    std::unique_ptr<gfx::Buffer> gpu_buffer = std::make_unique<gfx::Buffer>("gpu_buffer", 64, gfx::EBufferUsage::GPU_MEMORY);
    gpu_buffer->set_data(data, sizeof(int32_t) * 16);

    std::unique_ptr<gfx::Buffer> indirect_buffer = std::make_unique<gfx::Buffer>("indirect_buffer", 64, gfx::EBufferUsage::INDIRECT_DRAW_ARGUMENT);
    indirect_buffer->set_data(data, sizeof(int32_t) * 16);

    std::shared_ptr<gfx::Texture> texture = gfx::Texture::create(5, 5, gfx::TextureParameter{.format = gfx::EImageFormat::R_UNORM_8});

    texture->set_pixels(std::vector<uint8_t>{
        255, 255, 255, 0, 255, 255, 255, 255, 0, 255, 255, 255, 255, 0, 255, 255, 255, 255, 0, 255, 255, 255, 255, 0, 255,
    });

    /**
     * 4° framegraph definition
     */
    auto       post_process_resolve   = std::make_unique<gfx::FrameGraphResource>();
    const auto temporal_anti_aliasing = std::make_shared<gfx::FrameGraphResource>();
    const auto gbuffer_resolve        = std::make_shared<gfx::FrameGraphResource>();
    const auto gbuffers               = std::make_shared<gfx::FrameGraphResource>();
    const auto mini_map               = std::make_shared<gfx::FrameGraphResource>();

    post_process_resolve->add_child(temporal_anti_aliasing);
    post_process_resolve->add_child(mini_map);
    temporal_anti_aliasing->add_child(gbuffer_resolve);
    gbuffer_resolve->add_child(gbuffers);

    const auto framegraph = std::shared_ptr<gfx::FrameGraph>();
    framegraph->set_root(std::move(post_process_resolve));
    framegraph->generate();

    /**
     * 5° Application loop
     */
    while (application::window::Window::get_window_count() > 0)
    {
        gfx::next_frame();
        for (uint32_t i = 0; i < application::window::Window::get_window_count(); ++i)
        {
            main_render_target->update(*main_view);
            surface_1->display(*main_render_target);

            application::window::Window::get_window(i)->update();
        }
    }

    /**
     * 6° clean GPU data : //@TODO automatically free allocated resources
     */
    texture            = nullptr;
    main_render_target = nullptr;
    main_view          = nullptr;
    gpu_buffer         = nullptr;
    indirect_buffer    = nullptr;
    surface_1          = nullptr;

    // Destroy graphic backend and close application
    gfx::destroy();
    application::destroy();

    exit(EXIT_SUCCESS);
}
