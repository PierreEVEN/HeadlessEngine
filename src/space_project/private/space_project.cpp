

#include "space_region.h"

#include <application/application.h>
#include <gfx/gfx.h>

std::vector<SpaceRegion> space_regions;

void render()
{
}

void declare_render_pass()
{
    const std::vector<gfx::RenderPass::Config::Attachment> color_attachments{{
                                                                                 .attachment_name = "velocity",
                                                                                 .image_format = ETypeFormat::R16G16B16A16_SFLOAT,
                                                                                 .clear_value     = gfx::ClearValue{.color = {1, 0, 1, 1}},
                                                                             },
                                                                             {
                                                                                 .attachment_name = "normal",
                                                                                 .clear_value     = gfx::ClearValue{.color = {1, 1, 0, 1}},
                                                                             },
                                                                             {
                                                                                 .attachment_name = "albedo",
                                                                                 .clear_value     = gfx::ClearValue{.color = {1, 1, 0.5, 1}},
                                                                             }};

    gfx::RenderPass::declare(gfx::RenderPass::Config{.pass_name = "gbuffer",
                                                     .color_attachments = color_attachments,
                                                     .depth_attachment = gfx::RenderPass::Config::Attachment{
                                                         .attachment_name = "depth",
                                                         .image_format    = ETypeFormat::D32_SFLOAT,
                                                         .clear_value     = gfx::ClearValue{.depth = 0},
                                                     }});
    gfx::RenderPass::declare(gfx::RenderPass::Config{.pass_name = "region_combine",
                                                     .color_attachments =
                                                         std::vector<gfx::RenderPass::Config::Attachment>{
                                                             {
                                                                 .attachment_name = "color",
                                                                 .clear_value     = gfx::ClearValue{.color = {1, 0, 0, 1}},
                                                             },
                                                         },
                                                     .depth_attachment = gfx::RenderPass::Config::Attachment{
                                                         .attachment_name = "depth",
                                                         .image_format    = ETypeFormat::D32_SFLOAT,
                                                         .clear_value     = gfx::ClearValue{.depth = 0},
                                                     }});
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{

    Logger::get().enable_logs(Logger::LogType::LOG_LEVEL_INFO | Logger::LogType::LOG_LEVEL_DEBUG);

    application::create();
    gfx::init();
    auto* window  = create_window(application::window::WindowConfig{
        .name         = "padenom",
        .width        = 800,
        .height       = 600,
        .window_style = application::window::EWindowStyle::WINDOWED,
    });
    auto* surface = gfx::Surface::create_surface(window);
    declare_render_pass();

    space_regions.emplace_back(SpaceRegion{});

    auto combine_pass = gfx::RenderPassInstance::create(window->width(), window->width(), gfx::RenderPassID::get("region_combine"));
    combine_pass->on_draw_pass.add_lambda(
        []([[maybe_unused]] gfx::CommandBuffer* command_buffer)
        {
            // Repeat for each of the gbuffers textures
            // std::vector<Texture> children_albedo;
            // command_buffer.set_texture_array("albedo_buffers", children_albedo);
            // command_buffer.draw_mesh(screen_mesh, region_combine);
        });
    for (auto& region : space_regions)
    {
        auto gbuffer_pass = gfx::RenderPassInstance::create(window->width(), window->width(), gfx::RenderPassID::get("gbuffer"));
        gbuffer_pass->on_draw_pass.add_lambda(
            [&region](gfx::CommandBuffer* command_buffer)
            {
                // This is a pass where we will render a region
                region.render(command_buffer);
            });
        combine_pass->link_dependency(gbuffer_pass);
    }
    surface->link_dependency(combine_pass);
    surface->build_framegraph();

    while (application::window::Window::get_window_count() > 0)
    {
        gfx::next_frame();

        // update main scene
        for (auto& region : space_regions)
        {
            // gameplay
            region.tick();
            // send gameplay to gfx
            gfx::View camera;
            region.pre_render(&camera);
        }
        // render everything
        surface->render();
        window->update();
    }

    space_regions.clear();
    combine_pass = nullptr;
    delete surface;
    gfx::destroy();
    application::destroy();
    exit(EXIT_SUCCESS);
}
