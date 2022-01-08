#include "application/application.h"
#include "application/window.h"
#include "gfx/materials/master_material.h"
#include "gfx/materials/material_instance.h"
#include "gfx/mesh.h"
#include "gfx/texture.h"
#include "gfx/view.h"

#include <gfx/gfx.h>

#include <cpputils/logger.hpp>

/***
 * ###########  SURFACE
 *
 * render() {
 *  // on_render.execute();
 *
 *  ecs.pre_render();
 *  ecs.render();
 * }
 *
 *
 */







int main()
{
    Logger::get().enable_logs(Logger::LogType::LOG_LEVEL_INFO | Logger::LogType::LOG_LEVEL_DEBUG);

    /**
     * 1° initialize the application and the gfx backend
     */
    application::create();
    gfx::init();

    /**
     * 2° declare some windows with surfaces (surfaces are layers that allow rendering images from the gfx backend onto application window)
     */
    auto* window_1  = create_window(application::window::WindowConfig{.name = application::get_full_name(), .window_style = application::window::EWindowStyle::WINDOWED});
    auto  surface_1 = std::unique_ptr<gfx::Surface>(gfx::Surface::create_surface(window_1));

    std::unique_ptr<gfx::View> main_view = std::make_unique<gfx::View>();

    /**
     * 3° render pass definition
     */

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

    /**
     * 4° frame graph construction
     */
    auto g_buffer_graph_pass = gfx::RenderPassInstance::create(surface_1->get_container()->width(), surface_1->get_container()->width(), gfx::RenderPassID::get("gbuffer"));
    surface_1->link_dependency(g_buffer_graph_pass);
    surface_1->build_framegraph();

    auto glob_mat     = gfx::MasterMaterial::create("data/shaders/draw_procedural_test.shb");
    auto mat_instance = gfx::MaterialInstance::create(glob_mat);
    auto vertices     = std::vector{gfx::Mesh::Vertex{
                                    .pos = glm::vec3(0, 0, 0),
                                },
                                gfx::Mesh::Vertex{
                                    .pos = glm::vec3(1, 0, 0),
                                },
                                gfx::Mesh::Vertex{
                                    .pos = glm::vec3(1, 1, 0),
                                },
                                gfx::Mesh::Vertex{
                                    .pos = glm::vec3(0, 1, 0),
                                }};
    auto indices      = std::vector<uint32_t>{0, 1, 2, 0, 2, 3};
    auto glob_mesh    = std::make_shared<gfx::Mesh>("test_mesh", vertices, indices);
    
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
    glob_mesh           = nullptr;
    mat_instance        = nullptr;
    glob_mat            = nullptr;
    surface_1           = nullptr;
    g_buffer_graph_pass = nullptr;
    main_view           = nullptr;
    gfx::RenderPass::destroy_passes();

    // Destroy graphic backend and close application
    gfx::destroy();
    application::destroy();

    exit(EXIT_SUCCESS);
}
