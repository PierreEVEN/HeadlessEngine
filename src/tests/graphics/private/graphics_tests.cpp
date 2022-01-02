#include "shader_builder_test.h"
#include "application/application.h"
#include "application/window.h"
#include "gfx/texture.h"
#include "gfx/view.h"

#include <gfx/gfx.h>

#include <cpputils/logger.hpp>

void create_frame_graph(gfx::Surface* surface)
{
    const auto temporal_anti_aliasing = gfx::RenderPass::create(256, 256,
                                                                gfx::RenderPassConfig{
                                                                    .pass_name = "temporal anti aliasing",
                                                                    .color_attachments =
                                                                        std::vector<gfx::RenderPassConfig::Attachment>{
                                                                            {
                                                                                .attachment_name = "color",
                                                                            },
                                                                        },
                                                                });
    temporal_anti_aliasing->generate_framebuffer_images();

    const auto gbuffer_resolve = gfx::RenderPass::create(256, 256,
                                                         gfx::RenderPassConfig{
                                                             .pass_name = "gbuffer resolve",
                                                             .color_attachments =
                                                                 std::vector<gfx::RenderPassConfig::Attachment>{
                                                                     {
                                                                         .attachment_name = "color",
                                                                     },
                                                                 },
                                                         });
    gbuffer_resolve->generate_framebuffer_images();

    const auto gbuffers = gfx::RenderPass::create(256, 256,
                                                  gfx::RenderPassConfig{.pass_name = "gbuffers",
                                                                        .color_attachments =
                                                                            std::vector<gfx::RenderPassConfig::Attachment>{
                                                                                {
                                                                                    .attachment_name = "albedo",
                                                                                },
                                                                                {
                                                                                    .attachment_name = "normal",
                                                                                },
                                                                                {
                                                                                    .attachment_name = "velocity",
                                                                                },
                                                                            },
                                                                        .depth_attachment = gfx::RenderPassConfig::Attachment{
                                                                            .attachment_name = "depth",
                                                                            .image_format    = gfx::EImageFormat::DEPTH_32_FLOAT,
                                                                        }});
    gbuffers->generate_framebuffer_images();

    surface->add_child(temporal_anti_aliasing);
    temporal_anti_aliasing->add_child(gbuffer_resolve);
    gbuffer_resolve->add_child(gbuffers);
}

int main()
{
    Logger::get().enable_logs(Logger::LogType::LOG_LEVEL_INFO | Logger::LogType::LOG_LEVEL_DEBUG);

    shader_builder::test();

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

    std::unique_ptr<gfx::View> main_view = std::make_unique<gfx::View>();

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
    create_frame_graph(surface_1.get());

    /**
     * 5° Application loop
     */
    while (application::window::Window::get_window_count() > 0)
    {
        gfx::next_frame();
        for (uint32_t i = 0; i < application::window::Window::get_window_count(); ++i)
        {
            surface_1->render();
            application::window::Window::get_window(i)->update();
        }
    }

    /**
     * 6° clean GPU data : //@TODO automatically free allocated resources
     */
    texture         = nullptr;
    main_view       = nullptr;
    gpu_buffer      = nullptr;
    indirect_buffer = nullptr;
    surface_1       = nullptr;

    // Destroy graphic backend and close application
    gfx::destroy();
    application::destroy();

    exit(EXIT_SUCCESS);
}
