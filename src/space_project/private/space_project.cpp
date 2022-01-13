

#include "space_region.h"

#include <application/application.h>
#include <gfx/gfx.h>

std::vector<SpaceRegion> space_regions;

void render()
{
}

void declare_render_pass()
{
    const auto color_attachments = std::vector<gfx::RenderPass::Config::Attachment>{
        {
            .attachment_name = "velocity",
            .image_format    = ETypeFormat::R16G16B16A16_SFLOAT,
            .clear_value     = gfx::ClearValue{.color = {0, 0, 0, 1}},
        },
        {
            .attachment_name = "normal",
            .clear_value     = gfx::ClearValue{.color = {0, 0, 0, 1}},
        },
        {
            .attachment_name = "albedo",
            .clear_value     = gfx::ClearValue{.color = {0, 0, 0, 1}},
        },
    };
    const auto gbuffer_depth = gfx::RenderPass::Config::Attachment{
        .attachment_name = "depth",
        .image_format    = ETypeFormat::D32_SFLOAT,
        .clear_value     = gfx::ClearValue{.depth = 1, .stencil = 0},
    };
    gfx::RenderPass::declare(gfx::RenderPass::Config{
        .pass_name         = "gbuffer",
        .color_attachments = color_attachments,
        .depth_attachment  = gbuffer_depth,
    });

    const auto region_combine_color_attachments = std::vector<gfx::RenderPass::Config::Attachment>{
        {
            .attachment_name = "color",
            .clear_value     = gfx::ClearValue{.color = {0, 0, 0, 1}},
        },
    };
    const auto region_combine_depth_attachment = gfx::RenderPass::Config::Attachment{
        .attachment_name = "depth",
        .image_format    = ETypeFormat::D32_SFLOAT,
        .clear_value     = gfx::ClearValue{.depth = 1, .stencil = 0},
    };

    gfx::RenderPass::declare(gfx::RenderPass::Config{
        .pass_name         = "region_combine",
        .color_attachments = region_combine_color_attachments,
        .depth_attachment  = region_combine_depth_attachment,
    });
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{

    Logger::get().enable_logs(Logger::LogType::LOG_LEVEL_INFO | Logger::LogType::LOG_LEVEL_DEBUG);

    application::create();
    gfx::init();
    auto* window  = create_window(application::window::WindowConfig{
        .name         = "padenom",
        .absolute_width        = 800,
        .absolute_height       = 600,
        .window_style = application::window::EWindowStyle::WINDOWED,
    });
    auto* surface = gfx::Surface::create_surface(window);
    declare_render_pass();

    space_regions.emplace_back(SpaceRegion{});

    auto region_combine_master_material   = gfx::MasterMaterial::create("data/shaders/draw_procedural_test.shb");
    auto region_combine_material_instance = gfx::MaterialInstance::create(region_combine_master_material);

    auto combine_pass = gfx::RenderPassInstance::create(window->absolute_width(), window->absolute_height(), gfx::RenderPassID::get("region_combine"));
    combine_pass->on_draw_pass.add_lambda(
        [&region_combine_material_instance](gfx::CommandBuffer* command_buffer)
        {
            std::vector<gfx::Texture> children_albedo;
            // command_buffer->set_texture_array("albedo_buffers", children_albedo);
            command_buffer->draw_procedural(region_combine_material_instance.get(), 3, 0, 1, 0);
        });
    for (auto& region : space_regions)
    {
        auto gbuffer_pass = gfx::RenderPassInstance::create(window->absolute_width(), window->absolute_height(), gfx::RenderPassID::get("gbuffer"));
        gbuffer_pass->on_draw_pass.add_lambda(
            [&region](gfx::CommandBuffer* command_buffer)
            {
                // This is a pass where we will render a region
                region.render(command_buffer);
            });
        combine_pass->link_dependency(gbuffer_pass);
    }

    auto surface_resolve_master_material   = gfx::MasterMaterial::create("data/shaders/draw_procedural_test.shb");
    auto surface_resolve_material_instance = gfx::MaterialInstance::create(surface_resolve_master_material);
    surface->link_dependency(combine_pass);
    surface->build_framegraph();
    surface->on_draw->add_lambda(
        [&surface_resolve_material_instance](gfx::CommandBuffer* command_buffer)
        {            
            command_buffer->draw_procedural(surface_resolve_material_instance.get(), 3, 0, 1, 0);
        });

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
    surface_resolve_master_material  = nullptr;
    surface_resolve_material_instance = nullptr;
    region_combine_master_material   = nullptr;
    region_combine_material_instance = nullptr;
    combine_pass                     = nullptr;
    delete surface;
    gfx::destroy();
    application::destroy();
    exit(EXIT_SUCCESS);
}
