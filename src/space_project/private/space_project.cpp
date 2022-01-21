

#include "scene/SubScene.h"
#include "space_region.h"
#include "ui.h"

#include <application/application.h>
#include <gfx/gfx.h>

void declare_render_pass()
{
    const auto color_attachments = std::vector<gfx::RenderPass::Config::Attachment>{
        {
            .attachment_name = "albedo",
            .clear_value     = gfx::ClearValue{.color = {0, 0, 0, 1}},
        },
        {
            .attachment_name = "normal",
            .clear_value     = gfx::ClearValue{.color = {0, 0, 0, 1}},
        },
        {
            .attachment_name = "velocity",
            .image_format    = ETypeFormat::R16G16B16A16_SFLOAT,
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
    gfx::RenderPass::declare(gfx::RenderPass::Config{
        .pass_name         = "deferred_combine",
        .color_attachments = region_combine_color_attachments,
    });

    const auto ui_pass_color_attachments = std::vector<gfx::RenderPass::Config::Attachment>{
        {
            .attachment_name = "color",
            .clear_value     = gfx::ClearValue{.color = {0, 0, 0, 1}},
        },
    };
    gfx::RenderPass::declare(gfx::RenderPass::Config{
        .pass_name         = "ui_pass",
        .color_attachments = region_combine_color_attachments,
    });
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    Logger::get().enable_logs(Logger::LogType::LOG_LEVEL_INFO | Logger::LogType::LOG_LEVEL_DEBUG);

    application::create();
    gfx::init();
    auto* window  = create_window(application::window::WindowConfig{
        .name            = "padenom",
        .absolute_width  = 800,
        .absolute_height = 600,
        .window_style    = application::window::EWindowStyle::WINDOWED,
    });
    auto* surface = gfx::Surface::create_surface(window);
    declare_render_pass();

    std::unique_ptr<scene::Universe> global_universe = std::make_unique<CustomUniverse>();
    std::unique_ptr<ui::UICanvas>    canvas          = std::make_unique<ui::UICanvas>(nullptr);

    auto gbuffer_pass          = gfx::RenderPassInstance::create(window->absolute_width(), window->absolute_height(), gfx::RenderPassID::get("gbuffer"));
    auto deferred_combine_pass = gfx::RenderPassInstance::create(window->absolute_width(), window->absolute_height(), gfx::RenderPassID::get("deferred_combine"));
    auto ui_pass               = gfx::RenderPassInstance::create(window->absolute_width(), window->absolute_height(), gfx::RenderPassID::get("ui_pass"));

    auto resolve_sampler                  = gfx::Sampler::create("resolve sampler", {});
    auto region_combine_material_instance = gfx::MaterialInstance::create(gfx::MasterMaterial::create("data/shaders/draw_procedural_test.shb"));
    auto resolve_material_instance        = gfx::MaterialInstance::create(gfx::MasterMaterial::create("data/shaders/engine/resolve.shb"));
    resolve_material_instance->bind_texture("combine_albedo", deferred_combine_pass->get_framebuffer_images()[0]);
    resolve_material_instance->bind_texture("ui_result", ui_pass->get_framebuffer_images()[0]);
    resolve_material_instance->bind_sampler("ui_sampler", resolve_sampler);

    // Pass render content
    gbuffer_pass->on_draw_pass.add_lambda(
        [&](gfx::CommandBuffer* command_buffer)
        {
            global_universe->render(command_buffer);
        });
    deferred_combine_pass->on_draw_pass.add_lambda(
        [&region_combine_material_instance](gfx::CommandBuffer* command_buffer)
        {
            std::vector<gfx::Texture> children_albedo;
            command_buffer->draw_procedural(region_combine_material_instance.get(), 3);
        });
    ui_pass->on_draw_pass.add_lambda(
        [&](gfx::CommandBuffer* command_buffer)
        {
            canvas->init(ui::UICanvas::Context{
                .draw_pos_x  = 0,
                .draw_pos_y  = 0,
                .draw_width  = window->absolute_width(),
                .draw_height = window->absolute_height(),
            });
            canvas->start_window("this is a test window");
            canvas->label("this is a text yay");
            canvas->end_window();

            canvas->submit(command_buffer);
        });
    surface->on_draw->add_lambda(
        [&](gfx::CommandBuffer* command_buffer)
        {
            command_buffer->draw_procedural(resolve_material_instance.get(), 3);
        });

    deferred_combine_pass->link_dependency(gbuffer_pass);
    surface->link_dependency(ui_pass);
    surface->link_dependency(deferred_combine_pass);
    surface->build_framegraph();

    // Game loop
    while (application::window::Window::get_window_count() > 0)
    {
        gfx::next_frame();
        global_universe->tick();
        global_universe->pre_render();
        surface->render();
        window->update();
    }

    global_universe                  = nullptr;
    gbuffer_pass                     = nullptr;
    resolve_sampler                  = nullptr;
    resolve_material_instance        = nullptr;
    ui_pass                          = nullptr;
    canvas                           = nullptr;
    region_combine_material_instance = nullptr;
    deferred_combine_pass            = nullptr;
    delete surface;
    gfx::destroy();
    application::destroy();
    exit(EXIT_SUCCESS);
}
