
#include "deferred_renderer.h"



#include "assets/asset_material.h"
#include "assets/asset_material_instance.h"
#include "rendering/graphics.h"
#include "rendering/swapchain_config.h"

namespace DeferredRenderer
{

RendererConfiguration create_configuration()
{
    EventRenderRenderPass deferred_combine_render;
    EventRenderRenderPass deferred_post_process_rendering;

    deferred_combine_render.add_lambda([&](SwapchainFrame* render_context) {
        //@TODO fix crash when no vertex buffer is bound
        TAssetPtr<AMaterialInstance> material("deferred_resolve_material");
        if (!material)
        {
            LOG_WARNING("deferred_resolve_material is not valid");
            return;
        }
        material->update_descriptor_sets(render_context->render_pass, render_context->view, render_context->image_index);

        auto* pipeline = material->get_material_base()->get_pipeline(render_context->render_pass);

        vkCmdBindDescriptorSets(render_context->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *pipeline->get_pipeline_layout(), 0, 1,
                                &(*material->get_descriptor_sets(render_context->render_pass))[render_context->image_index].descriptor_set, 0, nullptr);

        vkCmdBindPipeline(render_context->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->get_pipeline());
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

        auto* pipeline = material->get_material_base()->get_pipeline(render_context->render_pass);

        vkCmdBindDescriptorSets(render_context->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *pipeline->get_pipeline_layout(), 0, 1,
                                &(*material->get_descriptor_sets(render_context->render_pass))[render_context->image_index].descriptor_set, 0, nullptr);

        vkCmdBindPipeline(render_context->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->get_pipeline());
        vkCmdDraw(render_context->command_buffer, 3, 1, 0, 0);
    });

    return RendererConfiguration(
        {
            RenderPassSettings{
                .pass_name = "render_scene",
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

void DeferredRenderer::create_deferred_assets()
{
    // Deferred combine
    {
        const ShaderInfos vertex_config{
            .shader_stage = VK_SHADER_STAGE_VERTEX_BIT,
        };
        const auto vertex_shader = AssetManager::get()->create<AShader>("deferred_resolve_vertex_shader", "data/shaders/deferred_resolve.vert.glsl", vertex_config);

        const ShaderInfos fragment_config{
            .shader_stage         = VK_SHADER_STAGE_FRAGMENT_BIT,
            .use_view_data_buffer = true,
            .textures{
                TextureProperty{.binding_name = "samplerAlbedo", .texture = TAssetPtr<ATexture>("framebuffer_image-render_scene_0")},
                TextureProperty{.binding_name = "samplerNormal", .texture = TAssetPtr<ATexture>("framebuffer_image-render_scene_1")},
                TextureProperty{.binding_name = "samplerPosition", .texture = TAssetPtr<ATexture>("framebuffer_image-render_scene_depth")},
            },
        };
        auto fragment_shader = AssetManager::get()->create<AShader>("deferred_resolve_fragment_shader", "data/shaders/deferred_resolve.frag.glsl", fragment_config, vertex_shader);

        MaterialInfos material_infos{
            .vertex_stage    = vertex_shader,
            .fragment_stage  = fragment_shader,
            .renderer_passes = {"combine_deferred"},
        };
        const auto material = AssetManager::get()->create<AMaterialBase>("deferred_resolve_material_base", material_infos);
        AssetManager::get()->create<AMaterialInstance>("deferred_resolve_material", material);
    }

    // Post process resolve
    {
        const ShaderInfos vertex_config{
            .shader_stage = VK_SHADER_STAGE_VERTEX_BIT,
        };
        const auto vertex_shader = AssetManager::get()->create<AShader>("post_process_resolve_vertex_shader", "data/shaders/post_process_resolve.vert.glsl", vertex_config);

        const ShaderInfos fragment_config{
            .shader_stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .textures{
                TextureProperty{.binding_name = "colorSampler", .texture = TAssetPtr<ATexture>("framebuffer_image-combine_deferred_0")},
            },
        };
        auto fragment_shader = AssetManager::get()->create<AShader>("post_process_resolve_fragment_shader", "data/shaders/post_process_resolve.frag.glsl", fragment_config, vertex_shader);

        MaterialInfos material_infos{
            .vertex_stage    = vertex_shader,
            .fragment_stage  = fragment_shader,
            .renderer_passes = {"post_processing_0"},
        };
        const auto material = AssetManager::get()->create<AMaterialBase>("post_process_resolve_material_base", material_infos);
        AssetManager::get()->create<AMaterialInstance>("post_process_resolve_material", material);
    }
}
} // namespace DeferredRenderer