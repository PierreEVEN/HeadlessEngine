
#include "testGameInterface.h"

#include "assets/asset_material.h"
#include "assets/asset_shader.h"
#include "assets/asset_texture.h"
#include "camera_basic_controller.h"
#include "imgui.h"
#include "ios/mesh_importer.h"
#include "ios/scene_importer.h"
#include "misc/primitives.h"
#include "scene/node_camera.h"
#include "scene/node_mesh.h"
#include "ui/window/window_base.h"
#include "ui/window/windows/content_browser.h"
#include "ui/window/windows/profiler.h"
#include "ui/window/windows/scene_outliner.h"

PlayerController* TestGameInterface::get_controller()
{
    return nullptr;
}

void create_default_objects()
{
    // Create textures
    AssetManager::get()->create<ATexture>("default_texture", std::vector<uint8_t>{0, 255, 0, 255, 255, 0, 255, 255, 0, 0, 0, 255, 0, 0, 255, 255}, 2, 2, 4);

    // Create shaders
    AssetManager::get()->create<AShader>("gltf_vertex_shader", "data/shaders/gltf.vs.glsl", EShaderStage::VERTEX_SHADER);
    AssetManager::get()->create<AShader>("gltf_fragment_shader", "data/shaders/gltf.fs.glsl", EShaderStage::FRAGMENT_SHADER);
    AssetManager::get()->create<AShader>("default_vertex_shader", "data/shaders/default.vs.glsl", EShaderStage::VERTEX_SHADER);
    AssetManager::get()->create<AShader>("default_fragment_shader", "data/shaders/default.fs.glsl", EShaderStage::FRAGMENT_SHADER);

    // create materials
    TAssetPtr<AMaterial> material =
        AssetManager::get()->create<AMaterial>("default_material", ShaderStageData{.shader = TAssetPtr<AShader>("default_vertex_shader")}, ShaderStageData{.shader = TAssetPtr<AShader>("default_fragment_shader")});

    // Create meshes
    primitive::create_primitive<primitive::CubePrimitive>("default_cube");
}

void TestGameInterface::load_resources()
{
    create_default_objects();

    // Create scene
    root_scene = std::make_unique<Scene>();
    NMesh::register_component(root_scene.get());
    main_camera = root_scene->add_node<NCamera>("camera");
    controller  = std::make_unique<CameraBasicController>(main_camera, get_input_manager());

    // Import scenes
    SceneImporter scene_importer;

    /*
    auto san_miguel = scene_importer.import_file("data/models/sanMiguel.glb", "sanMiguel", root_scene.get());
    san_miguel->set_relative_rotation(glm::dvec3(M_PI / 2, 0, M_PI));
    san_miguel->set_relative_scale(glm::vec3(40));
    */
    // auto san_miguel = scene_importer.import_file("data/models/F-16_b.glb", "f16", root_scene.get());
    // scene_importer.import_file("data/models/fireplaceRoom.glb", "fireplaceRoom", root_scene.get())->set_relative_rotation(glm::dvec3(M_PI, 0, 0));
    // scene_importer.import_file("data/models/powerplant.glb", "powerplant", root_scene.get());
    // scene_importer.import_file("data/models/rungholt.glb", "rungholt", root_scene.get());
    // scene_importer.import_file("data/models/bistro.glb", "cafe_ext", root_scene.get());
    // scene_importer.import_file("data/models/bistro_interior.glb", "cafe_int", root_scene.get());
    // scene_importer.import_file("data/models/sibenik.glb", "sponza_elem", root_scene.get());
    // root_scene->add_node<NMesh>("cube", TAssetPtr<AMeshData>("default_cube"), TAssetPtr<AMaterial>("default_material"));

    const auto sponza_root = scene_importer.import_file("data/models/sponza.glb", "sponza_elem", root_scene.get());

    int max_x = 10, max_y = 10;
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

void TestGameInterface::pre_initialize()
{
}

void TestGameInterface::pre_shutdown()
{
}

void TestGameInterface::unload_resources()
{
    main_camera = nullptr;
    controller  = nullptr;
    root_scene  = nullptr;
}

void TestGameInterface::render_scene(SwapchainStatus render_context)
{
    main_camera->update_view(render_context);
}

void TestGameInterface::render_ui()
{
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
            ImGui::EndMenu();
        }
        ImGui::Text("%d fps   ...   %lf ms", static_cast<int>(1.0 / get_delta_second()), get_delta_second() * 1000.0);
        ImGui::EndMainMenuBar();
    }
    return;
    for (const auto& primitive : root_scene->get_nodes())
    {
        main_camera->get_debug_draw()->draw_box(primitive->get_world_bounds());
    }
}

void TestGameInterface::render_hud()
{
}

void TestGameInterface::pre_draw()
{
    root_scene->tick(0.0001);
}

void TestGameInterface::post_draw()
{
}