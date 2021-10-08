

#include "rendering/vulkan/material_pipeline.h"

#include "assets/asset_mesh_data.h"
#include "assets/asset_shader.h"
#include "rendering/graphics.h"
#include "rendering/renderer/render_pass.h"
#include "rendering/renderer/renderer.h"
#include "rendering/swapchain_config.h"
#include "rendering/vulkan/common.h"
#include "rendering/vulkan/utils.h"

#include <vulkan/vulkan.h>

MaterialPipeline::MaterialPipeline(const PipelineInfos& pipeline_infos, const std::string& render_pass, const std::vector<VkDescriptorSetLayoutBinding>& layout_bindings, const std::vector<TAssetPtr<AShader>>& stages)
{
    /**
     * Create descriptor sets
     */

    /** Create descriptor set layout */
    descriptor_set_layout.resize(Graphics::get()->get_swapchain_config()->get_image_count());
    for (uint32_t i = 0; i < Graphics::get()->get_swapchain_config()->get_image_count(); ++i)
    {
        VkDescriptorSetLayoutCreateInfo layout_infos{
            .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .bindingCount = static_cast<uint32_t>(layout_bindings.size()),
            .pBindings    = layout_bindings.data(),
        };
        VK_ENSURE(vkCreateDescriptorSetLayout(Graphics::get()->get_logical_device(), &layout_infos, vulkan_common::allocation_callback, &descriptor_set_layout[i]), "Failed to create descriptor set layout");
    }
    /**
     * Create pipeline layout
     */

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{
        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount         = static_cast<uint32_t>(descriptor_set_layout.size()),
        .pSetLayouts            = descriptor_set_layout.data(),
        .pushConstantRangeCount = 0,
        .pPushConstantRanges    = nullptr,
    };
    VK_ENSURE(vkCreatePipelineLayout(Graphics::get()->get_logical_device(), &pipelineLayoutInfo, nullptr, &pipeline_layout), "Failed to create pipeline layout");

    /**
     * Create pipeline
     */
    VertexInputInfo                              vertex_inputs = Vertex::get_attribute_descriptions();

    std::vector<VkPipelineShaderStageCreateInfo> shaderStages = {};
    for (const auto& stage : stages)
    {
        shaderStages.emplace_back(VkPipelineShaderStageCreateInfo{
            .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage  = stage->get_shader_stage(),
            .module = stage->get_shader_module(),
            .pName  = "main",
        });

        if (stage->get_shader_config().vertex_inputs)
            vertex_inputs = stage->get_shader_config().vertex_inputs.value();
    }
    
    auto* pass_configuration = Graphics::get()->get_renderer()->get_render_pass_configuration(render_pass);

    if (!pass_configuration)
        LOG_FATAL("pass configuration is null");

    VkVertexInputBindingDescription bindingDescription{
        .binding   = 0,
        .stride    = vertex_inputs.vertex_structure_size,
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
    };

    const auto vertex_attribute_description = vertex_inputs.get_attributes();

    VkPipelineVertexInputStateCreateInfo vertex_input_state{
        .sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount   = 1,
        .pVertexBindingDescriptions      = &bindingDescription,
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(vertex_attribute_description.size()),
        .pVertexAttributeDescriptions    = vertex_attribute_description.data(),
    };

    VkPipelineInputAssemblyStateCreateInfo input_assembly{
        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology               = pipeline_infos.topology,
        .primitiveRestartEnable = VK_FALSE,
    };

    VkPipelineViewportStateCreateInfo viewport_state{
        .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount  = 1,
    };

    VkPipelineRasterizationStateCreateInfo rasterizer{
        .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable        = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode             = pipeline_infos.polygon_mode,
        .cullMode                = VK_CULL_MODE_BACK_BIT,
        .frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .depthBiasEnable         = VK_FALSE,
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp          = 0.0f,
        .depthBiasSlopeFactor    = 0.0f,
        .lineWidth               = pipeline_infos.wireframe_lines_width,
    };

    VkPipelineMultisampleStateCreateInfo multisampling{
        .sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples  = pass_configuration->sample_count,
        .sampleShadingEnable   = static_cast<VkBool32>(pass_configuration->sample_count > 1 ? VK_TRUE : VK_FALSE),
        .minSampleShading      = 1.0f,
        .pSampleMask           = nullptr,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable      = VK_FALSE,
    };

    VkPipelineDepthStencilStateCreateInfo depth_stencil{
        .sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable       = pipeline_infos.depth_test,
        .depthWriteEnable      = pipeline_infos.depth_test,
        .depthCompareOp        = VK_COMPARE_OP_LESS,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable     = VK_FALSE,
        .minDepthBounds        = 0.0f,
        .maxDepthBounds        = 1.0f,
    };

    std::vector<VkPipelineColorBlendAttachmentState> color_blend_attachment;
    for (const auto& attachment : pass_configuration->color_attachments)
    {
        color_blend_attachment.emplace_back(VkPipelineColorBlendAttachmentState{
            .blendEnable         = VK_FALSE,
            .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
            .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
            .colorBlendOp        = VK_BLEND_OP_ADD,
            .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
            .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
            .alphaBlendOp        = VK_BLEND_OP_ADD,
            .colorWriteMask      = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        });
    }

    VkPipelineColorBlendStateCreateInfo color_blending{
        .sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable   = VK_FALSE,
        .logicOp         = VK_LOGIC_OP_COPY,
        .attachmentCount = static_cast<uint32_t>(color_blend_attachment.size()),
        .pAttachments    = color_blend_attachment.data(),
        .blendConstants  = {0.0f, 0.f, 0.f, 0.f},
    };

    std::vector                      dynamic_states_array = {VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_LINE_WIDTH};
    VkPipelineDynamicStateCreateInfo dynamic_states{
        .sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = static_cast<uint32_t>(dynamic_states_array.size()),
        .pDynamicStates    = dynamic_states_array.data(),
    };

    VkGraphicsPipelineCreateInfo pipelineInfo{
        .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount          = static_cast<uint32_t>(shaderStages.size()),
        .pStages             = shaderStages.data(),
        .pVertexInputState   = &vertex_input_state,
        .pInputAssemblyState = &input_assembly,
        .pViewportState      = &viewport_state,
        .pRasterizationState = &rasterizer,
        .pMultisampleState   = &multisampling,
        .pDepthStencilState  = &depth_stencil,
        .pColorBlendState    = &color_blending,
        .pDynamicState       = &dynamic_states,
        .layout              = pipeline_layout,
        .renderPass          = Graphics::get()->get_renderer()->get_render_pass(render_pass)->get_render_pass(),
        .subpass             = 0,
        .basePipelineHandle  = VK_NULL_HANDLE,
        .basePipelineIndex   = -1,
    };

    VK_ENSURE(vkCreateGraphicsPipelines(Graphics::get()->get_logical_device(), VK_NULL_HANDLE, 1, &pipelineInfo, vulkan_common::allocation_callback, &pipeline), "Failed to create material graphic pipeline");
}

MaterialPipeline::~MaterialPipeline()
{
    if (pipeline_layout != VK_NULL_HANDLE)
        vkDestroyPipelineLayout(Graphics::get()->get_logical_device(), pipeline_layout, vulkan_common::allocation_callback);
    pipeline_layout = VK_NULL_HANDLE;

    if (pipeline != VK_NULL_HANDLE)
        vkDestroyPipeline(Graphics::get()->get_logical_device(), pipeline, vulkan_common::allocation_callback);
    pipeline = VK_NULL_HANDLE;

    for (const auto& descriptor_layout : descriptor_set_layout)
        vkDestroyDescriptorSetLayout(Graphics::get()->get_logical_device(), descriptor_layout, vulkan_common::allocation_callback);
    descriptor_set_layout.clear();
}

VkDescriptorSetLayout* MaterialPipeline::get_descriptor_sets_layouts()
{
    return descriptor_set_layout.data();
}

VkPipelineLayout* MaterialPipeline::get_pipeline_layout()
{
    return &pipeline_layout;
}

VkPipeline MaterialPipeline::get_pipeline() const
{
    return pipeline;
}