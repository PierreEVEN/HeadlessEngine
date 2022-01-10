
#include "main_game_interface.h"

#include "assets/asset_material.h"
#include "assets/asset_material_instance.h"
#include "assets/asset_texture.h"
#include "custom_graphic_interface.h"
#include "deferred_renderer.h"
#include "misc/primitives.h"
#include "rendering/shaders/shader_property.h"
#include "scene/node_camera.h"
#include "scene/node_mesh.h"
#include "scene_importer.h"
#include "ui/imgui/imgui_impl_glfw.h"
#include "ui/imgui/imgui_impl_vulkan.h"
#include "ui/window/windows/content_browser.h"
#include "ui/window/windows/profiler.h"
#include "ui/window/windows/scene_outliner.h"

static bool g_debug_lines = false;

RendererConfiguration MainGameInterface::get_default_render_pass_configuration()
{
    // We use a preconfigured deferred renderer configuration
    auto deferred_config = DeferredRenderer::create_configuration();

    deferred_config.get_render_pass("render_scene")->on_pass_rendering.add_lambda([&](SwapchainFrame* render_context) {
        main_camera->update_view(*render_context);
    });

    deferred_config.add_render_pass(ImGuiImplementation::get_ui_render_pass(TAssetPtr<ATexture>("framebuffer_image-combine_deferred_0"), imgui_instance));

    deferred_config.get_render_pass("ui_render_pass")->on_pass_rendering.add_lambda([&](SwapchainFrame* render_context) {
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        auto mat = imgui_instance->material_instance->get_material_base()->get_pipeline("ui_render_pass");
        if (!TAssetPtr<ATexture>("framebuffer_image-combine_deferred_0") || !mat)
        {
            LOG_FATAL("background image is null");
        }

        ImGui::GetBackgroundDrawList()->AddImage(
            TAssetPtr<ATexture>("framebuffer_image-combine_deferred_0")->get_imgui_handle(render_context->image_index, *mat->get_descriptor_sets_layouts()), ImVec2{0, 0},
            ImVec2{static_cast<float>(Graphics::get()->get_swapchain()->get_swapchain_extend().width), static_cast<float>(Graphics::get()->get_swapchain()->get_swapchain_extend().height)});

        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("file"))
            {
                if (ImGui::MenuItem("quit"))
                    close();
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("misc"))
            {
                if (ImGui::MenuItem("scene outliner"))
                    WindowManager::create<SceneOutliner>("scene outliner", nullptr, root_scene.get(), main_camera.get());
                if (ImGui::MenuItem("demo window"))
                    WindowManager::create<DemoWindow>("demo window", nullptr);
                if (ImGui::MenuItem("profiler"))
                    WindowManager::create<ProfilerWindow>("profiler", nullptr);
                if (ImGui::MenuItem("content browser"))
                    WindowManager::create<ContentBrowser>("content browser", nullptr);
                ImGui::Checkbox("show bounds", &g_debug_lines);
                ImGui::EndMenu();
            }
            ImGui::Text("%d fps   ...   %lf ms", static_cast<int>(1.0 / get_delta_second()), get_delta_second() * 1000.0);
            ImGui::EndMainMenuBar();
        }

        ImGui::SetNextWindowPos(ImVec2(-4, -4));
        ImGui::SetNextWindowSize(ImVec2(Graphics::get()->get_swapchain()->get_swapchain_extend().width + 8.f, Graphics::get()->get_swapchain()->get_swapchain_extend().height + 8.f));
        if (ImGui::Begin("BackgroundHUD", nullptr, ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground))
        {
            ImGui::DockSpace(ImGui::GetID("Master dockSpace"), ImVec2(0.f, 0.f), ImGuiDockNodeFlags_PassthruCentralNode);
        }
        ImGui::End();

        WindowManager::get()->draw();

        ImGui::EndFrame();
        ImGui::Render();
        imgui_instance->ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), *render_context);

        if (g_debug_lines)
            for (const auto& primitive : root_scene->get_nodes())
                main_camera->get_debug_draw()->draw_box(primitive->get_world_bounds());
    });

    return deferred_config;
}

GfxInterface* MainGameInterface::create_graphic_interface()
{
    // We want to use our custom graphic interface class
    return new CustomGraphicInterface();
}

static void create_default_objects()
{
    // Create textures
    AssetManager::get()->create<ATexture2D>("default_texture", std::vector<uint8_t>{255, 255, 255, 255}, 1, 1, 4);

    // Create shaders

    // Default shader
    {
        const ShaderInfos vertex_config{
            .shader_stage            = VK_SHADER_STAGE_VERTEX_BIT,
            .use_view_data_buffer    = true,
            .use_scene_object_buffer = true,
        };
        const auto vertex_shader = AssetManager::get()->create<AShader>("default_vertex_shader", "data/shaders/default.vs.glsl", vertex_config);

        const ShaderInfos fragment_config{
            .shader_stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .textures{
                TextureProperty{.binding_name = "p_diffuse", .texture = TAssetPtr<ATexture>("default_texture")},
            },
        };
        const auto fragment_shader = AssetManager::get()->create<AShader>("default_fragment_shader", "data/shaders/default.fs.glsl", fragment_config, vertex_shader);

        MaterialInfos material_infos{
            .vertex_stage    = vertex_shader,
            .fragment_stage  = fragment_shader,
            .renderer_passes = {"render_scene"},
        };
        const auto material = AssetManager::get()->create<AMaterialBase>("default_material_base", material_infos);
        AssetManager::get()->create<AMaterialInstance>("default_material", material);
    }

    // Create meshes
    primitive::create_primitive<primitive::CubePrimitive>("default_cube");
}

void MainGameInterface::engine_load_resources()
{
    create_default_objects();
    DeferredRenderer::create_deferred_assets();
    SceneImporter::create_default_resources();
    // Create scene
    root_scene = std::make_unique<Scene>();
    NMesh::register_component(root_scene.get());
    main_camera = root_scene->add_node<NCamera>("camera");
    controller  = std::make_unique<CameraBasicController>(main_camera, get_input_manager());

    imgui_instance = std::make_unique<ImGuiImplementation>();

    SceneImporter scene_importer;
    // auto san_miguel = scene_importer.import_file("data/models/sanMiguel.glb", "sanMiguel", root_scene.get());
    // san_miguel->set_relative_rotation(glm::dvec3(M_PI / 2, 0, M_PI));
    // san_miguel->set_relative_scale(glm::vec3(40));

    // scene_importer.import_file("data/models/F-16_b.glb", "f16", root_scene.get());
    // scene_importer.import_file("data/models/fireplaceRoom.glb", "fireplaceRoom", root_scene.get())->set_relative_rotation(glm::dvec3(M_PI, 0, 0));
    // scene_importer.import_file("data/models/powerplant.glb", "powerplant", root_scene.get());
    // scene_importer.import_file("data/models/rungholt.glb", "rungholt", root_scene.get());
    // scene_importer.import_file("data/models/bistro.glb", "cafe_ext", root_scene.get());
    // scene_importer.import_file("data/models/bistro_interior.glb", "cafe_int", root_scene.get());
    // scene_importer.import_file("data/models/sibenik.glb", "sponza_elem", root_scene.get());
    // root_scene->add_node<NMesh>("cube", TAssetPtr<AMeshData>("default_cube"), TAssetPtr<AMaterialBase>("default_material"));

    const auto sponza_root = scene_importer.import_file("data/models/sponza.glb", "sponza_elem", root_scene.get());
    const int max_x = 10, max_y = 10;
    for (int x = 0; x < max_x; ++x)
    {
        for (int y = 0; y < max_y; ++y)
        {
            const int                 i        = x + y * max_x;
            std::shared_ptr<NodeBase> new_root = root_scene->add_node<NodeBase>("sponza_root " + std::to_string(i));
            new_root->set_relative_position(glm::dvec3(x * 4000 + 4000, y * 4000 + 4000, 0));

            for (const auto& child : sponza_root->get_children())
            {
                auto*                  mesh_node = dynamic_cast<NMesh*>(child);
                std::shared_ptr<NMesh> new_child = root_scene->add_node<NMesh>("sponza_child " + std::to_string(i) + mesh_node->get_name(), mesh_node->get_mesh(), mesh_node->get_material());
                new_child->attach_to(new_root);
            }
        }
    }
}

void MainGameInterface::engine_tick(double delta_time)
{
    root_scene->tick(get_delta_second());
}

void MainGameInterface::engine_unload_resources()
{
    imgui_instance = nullptr;
    main_camera    = nullptr;
    controller     = nullptr;
    root_scene     = nullptr;
}
