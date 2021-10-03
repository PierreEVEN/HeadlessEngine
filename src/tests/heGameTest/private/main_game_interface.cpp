
#include "main_game_interface.h"

#include "assets/asset_material.h"
#include "assets/asset_shader.h"
#include "assets/asset_texture.h"
#include "camera_basic_controller.h"
#include "custom_graphic_interface.h"
#include "imgui.h"
#include "assets/asset_material_instance.h"
#include "ios/mesh_importer.h"
#include "ios/scene_importer.h"
#include "misc/primitives.h"
#include "rendering/shaders/shader_library.h"
#include "rendering/shaders/shader_property.h"
#include "rendering/swapchain_config.h"
#include "rendering/vulkan/common.h"
#include "scene/node_camera.h"
#include "scene/node_mesh.h"

static RendererConfiguration make_forward_renderer_config(std::shared_ptr<NCamera>& camera)
{
    EventRenderRenderPass forward_prepass_render;
    forward_prepass_render.add_lambda([&](SwapchainFrame* render_context) {
        camera->update_view(*render_context);
    });

    return RendererConfiguration({
        RenderPassSettings{
            .pass_name         = "render_scene",
            .sample_count      = static_cast<VkSampleCountFlagBits>(vulkan_common::get_msaa_sample_count()),
            .on_pass_rendering = forward_prepass_render,
            .color_attachments =
                std::vector<RenderPassAttachment>{
                    {
                        .image_format = Graphics::get()->get_swapchain_config()->get_surface_format().format,
                        .clear_value  = std::optional<VkClearValue>({.color = {.float32{0.4f, 0.5f, 0.6f, 1.0f}}}),
                    },
                },
            .depth_attachment =
                RenderPassAttachment{
                    .image_format = vulkan_utils::get_depth_format(),
                    .clear_value  = std::optional<VkClearValue>(VkClearValue{.depthStencil = {.depth = 1, .stencil = 0}}),
                },
        },
    });
}

static RendererConfiguration make_deferred_renderer_config(std::shared_ptr<NCamera>& camera)
{
    EventRenderRenderPass deferred_prepass_render;
    EventRenderRenderPass deferred_combine_render;
    EventRenderRenderPass deferred_post_process_rendering;
    deferred_prepass_render.add_lambda([&](SwapchainFrame* render_context) {
        camera->update_view(*render_context);
    });
    deferred_combine_render.add_lambda([&](SwapchainFrame* render_context) {
        //@TODO fix crash when no vertex buffer is bound
        TAssetPtr<AMaterialInstance> material("deferred_resolve_material");
        if (!material)
        {
            LOG_WARNING("deferred_resolve_material is not valid");
            return;
        }
        material->update_descriptor_sets(render_context->render_pass, render_context->view, render_context->image_index);
        vkCmdBindDescriptorSets(render_context->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, material->get_pipeline_layout(render_context->render_pass), 0, 1,
                                &material->get_descriptor_sets(render_context->render_pass)[render_context->image_index], 0, nullptr);

        vkCmdBindPipeline(render_context->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, material->get_pipeline(render_context->render_pass));
        vkCmdDraw(render_context->command_buffer, 3, 1, 0, 0);
    });
    deferred_post_process_rendering.add_lambda([&](SwapchainFrame* render_context) {
        TAssetPtr<AMaterialInstance> material("post_process_resolve_material");
        if (!material)
        {
            LOG_WARNING("post_process_resolve_material is not valid");
            return;
        }
        material->update_descriptor_sets(render_context->render_pass, render_context->view, render_context->image_index);
        vkCmdBindDescriptorSets(render_context->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, material->get_pipeline_layout(render_context->render_pass), 0, 1,
                                &material->get_descriptor_sets(render_context->render_pass)[render_context->image_index], 0, nullptr);

        vkCmdBindPipeline(render_context->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, material->get_pipeline(render_context->render_pass));
        vkCmdDraw(render_context->command_buffer, 3, 1, 0, 0);
    });

    return RendererConfiguration(
        {
            RenderPassSettings{
                .pass_name         = "render_scene",
                .on_pass_rendering = deferred_prepass_render,
                .color_attachments =
                    std::vector<RenderPassAttachment>{
                        // Albedo
                        {
                            .image_format = VK_FORMAT_R16G16B16A16_SFLOAT,
                            .clear_value  = std::optional<VkClearValue>({.color = {.float32{0.75f, 1.f, 1.2f, 1.0f}}}),
                        },
                        // World Normals
                        {
                            .image_format = VK_FORMAT_R16G16B16A16_SFLOAT,
                            .clear_value  = std::optional<VkClearValue>({.color = {.float32{0, 0, 0, 0}}}),
                        },
                        // World Position
                        {
                            .image_format = VK_FORMAT_R16G16B16A16_SFLOAT,
                            .clear_value  = std::optional<VkClearValue>({.color = {.float32{0, 0, 0, 0}}}),
                        },
                    },
                .depth_attachment =
                    RenderPassAttachment{
                        .image_format = vulkan_utils::get_depth_format(),
                        .clear_value  = std::optional<VkClearValue>(VkClearValue{.depthStencil = {.depth = 1, .stencil = 0}}),
                    },
            },

            RenderPassSettings{
                .pass_name         = "combine_deferred",
                .on_pass_rendering = deferred_combine_render,
                .color_attachments =
                    std::vector<RenderPassAttachment>{
                        {
                            .image_format = VK_FORMAT_R16G16B16A16_SFLOAT,
                        },
                    },
            },

            RenderPassSettings{
                .pass_name         = "post_processing_0",
                .on_pass_rendering = deferred_post_process_rendering,
                .color_attachments =
                    std::vector<RenderPassAttachment>{
                        {
                            .image_format = Graphics::get()->get_swapchain_config()->get_surface_format().format,
                        },
                    },
            },
        });
}

RendererConfiguration MainGameInterface::get_default_render_pass_configuration()
{
    // return make_forward_renderer_config(main_camera);
    return make_deferred_renderer_config(main_camera);
}

GfxInterface* MainGameInterface::create_graphic_interface()
{
    // We want to use our custom graphic interface class
    return new CustomGraphicInterface();
}

static void create_default_objects()
{
    // Create textures
    AssetManager::get()->create<ATexture2D>("default_texture", std::vector<uint8_t>{0, 255, 0, 255, 255, 0, 255, 255, 0, 0, 0, 255, 0, 0, 255, 255}, 2, 2, 4);

    // Create shaders

    // Gltf Shader
    {
        const ShaderConfiguration vertex_config{
            .shader_stage            = VK_SHADER_STAGE_VERTEX_BIT,
            .use_view_data_buffer    = true,
            .use_scene_object_buffer = true,
        };
        const auto vertex_shader = AssetManager::get()->create<AShader>("gltf_vertex_shader", "data/shaders/gltf.vs.glsl", vertex_config);

        const ShaderConfiguration fragment_config{
            .shader_stage            = VK_SHADER_STAGE_FRAGMENT_BIT,
            .input_stage = vertex_shader,
            .properties{
                ShaderUserProperty::create<ShaderPropertyTextureSampler>("diffuse_color", dynamic_cast<ATexture*>(TAssetPtr<ATexture>("default_texture").get())),
            },
        };
        const auto fragment_shader = AssetManager::get()->create<AShader>("gltf_fragment_shader", "data/shaders/gltf.fs.glsl", fragment_config);
    }
    
    // Default shader
    {
        const ShaderConfiguration vertex_config{
            .shader_stage            = VK_SHADER_STAGE_VERTEX_BIT,
            .use_view_data_buffer    = true,
            .use_scene_object_buffer = true,
        };
        const auto vertex_shader = AssetManager::get()->create<AShader>("default_vertex_shader", "data/shaders/default.vs.glsl", vertex_config);

        const ShaderConfiguration fragment_config{
            .shader_stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .input_stage  = vertex_shader,
            .properties{
                // ShaderUserProperty::create<ShaderPropertyFloat>("p_test_2", 0.5f),
                // ShaderUserProperty::create<ShaderPropertyVec3>("p_position_3", glm::vec3(1, 2, 3)), // @TODO
                ShaderUserProperty::create<ShaderPropertyTextureSampler>("p_diffuse", dynamic_cast<ATexture*>(TAssetPtr<ATexture>("default_texture").get())),
            },
        };
        const auto fragment_shader = AssetManager::get()->create<AShader>("default_fragment_shader", "data/shaders/default.fs.glsl", fragment_config);

        auto material = AssetManager::get()->create<AMaterial>("test_my_mat", fragment_shader, std::vector<std::string>{"render_scene"});
    }

    // Create meshes
    primitive::create_primitive<primitive::CubePrimitive>("default_cube");
}

static void create_deferred_objects()
{
    // Deferred combine
    {
        const ShaderConfiguration vertex_config{
            .shader_stage = VK_SHADER_STAGE_VERTEX_BIT,
        };
        const auto vertex_shader = AssetManager::get()->create<AShader>("deferred_resolve_vertex_shader", "data/shaders/deferred_resolve.vert.glsl", vertex_config);

        const ShaderConfiguration fragment_config{
            .shader_stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .input_stage  = vertex_shader,
            .use_view_data_buffer = true,
            .properties{
                ShaderUserProperty::create<ShaderPropertyTextureSampler>("samplerAlbedo", dynamic_cast<ATexture*>(TAssetPtr<ATexture>("framebuffer_image-render_scene_0").get())),
                ShaderUserProperty::create<ShaderPropertyTextureSampler>("samplerNormal", dynamic_cast<ATexture*>(TAssetPtr<ATexture>("framebuffer_image-render_scene_1").get())),
                ShaderUserProperty::create<ShaderPropertyTextureSampler>("samplerPosition", dynamic_cast<ATexture*>(TAssetPtr<ATexture>("framebuffer_image-render_scene_depth").get())),
            },
        };
        auto fragment_shader = AssetManager::get()->create<AShader>("deferred_resolve_fragment_shader", "data/shaders/deferred_resolve.frag.glsl", fragment_config);

        AssetManager::get()->create<AMaterial>("deferred_resolve_material", fragment_shader, std::vector<std::string>{"combine_deferred"});
    }

    // Post process resolve
    {
        const ShaderConfiguration vertex_config{
            .shader_stage = VK_SHADER_STAGE_VERTEX_BIT,
        };
        const auto vertex_shader = AssetManager::get()->create<AShader>("post_process_resolve_vertex_shader", "data/shaders/post_process_resolve.vert.glsl", vertex_config);

        const ShaderConfiguration fragment_config{
            .shader_stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .input_stage  = vertex_shader,
            .properties{
                ShaderUserProperty::create<ShaderPropertyTextureSampler>("colorSampler", dynamic_cast<ATexture*>(TAssetPtr<ATexture>("framebuffer_image-combine_deferred_0").get())),
            },
        };
        auto fragment_shader = AssetManager::get()->create<AShader>("post_process_resolve_fragment_shader", "data/shaders/post_process_resolve.frag.glsl", fragment_config);

        AssetManager::get()->create<AMaterial>("post_process_resolve_material", fragment_shader, std::vector<std::string>{"post_processing_0"});
    }
}

void MainGameInterface::engine_load_resources()
{
    create_default_objects();
    create_deferred_objects();

    // Create scene
    root_scene = std::make_unique<Scene>();
    NMesh::register_component(root_scene.get());
    main_camera = root_scene->add_node<NCamera>("camera");
    controller  = std::make_unique<CameraBasicController>(main_camera, get_input_manager());

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
    // root_scene->add_node<NMesh>("cube", TAssetPtr<AMeshData>("default_cube"), TAssetPtr<AMaterial>("default_material"));

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
