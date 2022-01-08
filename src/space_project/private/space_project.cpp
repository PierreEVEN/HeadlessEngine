

#include "space_region.h"

#include <application/application.h>
#include <gfx/gfx.h>

std::vector<SpaceRegion> space_regions;

void render()
{
}

void declare_render_pass()
{
    gfx::RenderPass::declare(gfx::RenderPass::Config{.pass_name = "gbuffer",
                                                     .color_attachments =
                                                         std::vector<gfx::RenderPass::Config::Attachment>{
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
                                                     .depth_attachment = gfx::RenderPass::Config::Attachment{
                                                         .attachment_name = "depth",
                                                         .image_format    = ETypeFormat::D32_SFLOAT,
                                                     }});
    gfx::RenderPass::declare(gfx::RenderPass::Config{.pass_name = "region_combine",
                                                     .color_attachments =
                                                         std::vector<gfx::RenderPass::Config::Attachment>{
                                                             {
                                                                 .attachment_name = "color",
                                                             },
                                                         },
                                                     .depth_attachment = gfx::RenderPass::Config::Attachment{
                                                         .attachment_name = "depth",
                                                         .image_format    = ETypeFormat::D32_SFLOAT,
                                                     }});
}

int main(int argc, char* argv[])
{
    application::create();
    gfx::init();
    auto* window  = create_window(application::window::WindowConfig{.name = "padenom", .window_style = application::window::EWindowStyle::WINDOWED});
    auto* surface = gfx::Surface::create_surface(window);
    declare_render_pass();

    gfx::View camera;

    const auto combine_pass = gfx::RenderPassInstance::create(window->width(), window->width(), gfx::RenderPassID::get("region_combine"));
    combine_pass->on_draw(
        [](gfx::CommandBuffer* command_buffer)
        {
            // Repeat for each of the gbuffers textures
            // std::vector<Texture> children_albedo;
            // command_buffer.set_texture_array("albedo_buffers", children_albedo);
            // command_buffer.draw_mesh(screen_mesh, region_combine);
        });
    for (auto& region : space_regions)
    {
        const auto gbuffer_pass = gfx::RenderPassInstance::create(window->width(), window->width(), gfx::RenderPassID::get("gbuffer"));
        gbuffer_pass->on_draw(
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
            region.pre_render(&camera);
        }
        // render everything
        surface->render();
        window->update();
    }

    delete surface;
    destroy_window(window);
    gfx::destroy();
    application::destroy();
    exit(EXIT_SUCCESS);
}
