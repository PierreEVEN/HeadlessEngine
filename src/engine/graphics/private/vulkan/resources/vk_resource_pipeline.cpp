

#include "vk_resource_pipeline.h"

#include "vulkan/vk_allocator.h"
#include "vulkan/vk_device.h"
#include "vulkan/vk_errors.h"
#include "vulkan/vk_helper.h"
#include "vulkan/vk_types.h"

namespace gfx::vulkan
{
static VkPolygonMode vk_polygon_mode(shader_builder::EPolygonMode polygon_mode)
{
    switch (polygon_mode)
    {
    case shader_builder::EPolygonMode::Point:
        return VK_POLYGON_MODE_POINT;
    case shader_builder::EPolygonMode::Line:
        return VK_POLYGON_MODE_LINE;
    case shader_builder::EPolygonMode::Fill:
        return VK_POLYGON_MODE_FILL;
    default:
        LOG_FATAL("unhandled case");
    }
}

static VkPrimitiveTopology vk_topology(shader_builder::ETopology topology)
{
    switch (topology)
    {
    case shader_builder::ETopology::Points:
        return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    case shader_builder::ETopology::Lines:
        return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    case shader_builder::ETopology::Triangles:
        return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    default:
        LOG_FATAL("unhandled case");
    }
}

static VkFrontFace vk_front_face(shader_builder::EFrontFace front_face)
{
    switch (front_face)
    {
    case shader_builder::EFrontFace::Clockwise:
        return VK_FRONT_FACE_CLOCKWISE;
    case shader_builder::EFrontFace::CounterClockwise:
        return VK_FRONT_FACE_COUNTER_CLOCKWISE;
    default:
        LOG_FATAL("unhandled case");
    }
}

static VkCullModeFlags vk_cull_mode(shader_builder::ECulling culling)
{
    switch (culling)
    {
    case shader_builder::ECulling::None:
        return VK_CULL_MODE_NONE;
    case shader_builder::ECulling::Front:
        return VK_CULL_MODE_FRONT_BIT;
    case shader_builder::ECulling::Back:
        return VK_CULL_MODE_BACK_BIT;
    case shader_builder::ECulling::Both:
        return VK_CULL_MODE_FRONT_AND_BACK;
    default:
        LOG_FATAL("unhandled case");
    }
}

PipelineResource_VK::PipelineResource_VK(const std::string& name, const CreateInfos& create_infos)
    : vertex_stage(create_infos.vertex_stage), fragment_stage(create_infos.fragment_stage), pipeline_layout(create_infos.pipeline_layout), render_pass(create_infos.render_pass)
{
    std::vector<VkVertexInputAttributeDescription> vertex_attribute_description;

    uint32_t vertex_input_size = 0;
    for (const auto& input_property : create_infos.vertex_inputs)
    {
        if (input_property.location == -1)
            continue;

        vertex_attribute_description.emplace_back(VkVertexInputAttributeDescription{
            .location = static_cast<uint32_t>(input_property.location),
            .format   = vk_texture_format_to_engine(input_property.type.format),
            .offset   = input_property.offset,
        });

        vertex_input_size += get_format_channel_count(input_property.type.format) * get_format_bytes_per_pixel(input_property.type.format);
    }

    const VkVertexInputBindingDescription bindingDescription{
        .binding   = 0,
        .stride    = vertex_input_size,
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
    };

    const VkPipelineVertexInputStateCreateInfo vertex_input_state{
        .sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount   = static_cast<uint32_t>(bindingDescription.stride > 0 ? 1 : 0),
        .pVertexBindingDescriptions      = bindingDescription.stride > 0 ? &bindingDescription : nullptr,
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(vertex_attribute_description.size()),
        .pVertexAttributeDescriptions    = vertex_attribute_description.data(),
    };

    const VkPipelineInputAssemblyStateCreateInfo input_assembly{
        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology               = vk_topology(create_infos.shader_properties.topology),
        .primitiveRestartEnable = VK_FALSE,
    };

    const VkPipelineViewportStateCreateInfo viewport_state{
        .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount  = 1,
    };

    const VkPipelineRasterizationStateCreateInfo rasterizer{
        .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable        = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode             = vk_polygon_mode(create_infos.shader_properties.polygon_mode),
        .cullMode                = vk_cull_mode(create_infos.shader_properties.culling),
        .frontFace               = vk_front_face(create_infos.shader_properties.front_face),
        .depthBiasEnable         = VK_FALSE,
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp          = 0.0f,
        .depthBiasSlopeFactor    = 0.0f,
        .lineWidth               = create_infos.shader_properties.line_width,
    };

    const VkPipelineMultisampleStateCreateInfo multisampling{
        .sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable   = VK_FALSE,
        .minSampleShading      = 1.0f,
        .pSampleMask           = nullptr,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable      = VK_FALSE,
    };

    const VkPipelineDepthStencilStateCreateInfo depth_stencil{
        .sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable       = create_infos.shader_properties.depth_test,
        .depthWriteEnable      = create_infos.shader_properties.depth_test,
        .depthCompareOp        = VK_COMPARE_OP_LESS,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable     = VK_FALSE,
        .minDepthBounds        = 0.0f,
        .maxDepthBounds        = 1.0f,
    };

    std::vector<VkPipelineColorBlendAttachmentState> color_blend_attachment;
    for (uint32_t i = 0; i < render_pass->color_attachment_count; ++i)
    {
        color_blend_attachment.emplace_back(VkPipelineColorBlendAttachmentState{
            .blendEnable         = create_infos.shader_properties.alpha_mode == shader_builder::EAlphaMode::Opaque ? VK_FALSE : VK_TRUE,
            .srcColorBlendFactor = create_infos.shader_properties.alpha_mode == shader_builder::EAlphaMode::Opaque ? VK_BLEND_FACTOR_ONE : VK_BLEND_FACTOR_SRC_ALPHA,
            .dstColorBlendFactor = create_infos.shader_properties.alpha_mode == shader_builder::EAlphaMode::Opaque ? VK_BLEND_FACTOR_ZERO : VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
            .colorBlendOp        = VK_BLEND_OP_ADD,
            .srcAlphaBlendFactor = create_infos.shader_properties.alpha_mode == shader_builder::EAlphaMode::Opaque ? VK_BLEND_FACTOR_ONE : VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
            .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
            .alphaBlendOp        = VK_BLEND_OP_ADD,
            .colorWriteMask      = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        });
    }

    std::vector<VkPipelineShaderStageCreateInfo> shaderStages = {
        {
            .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage  = VK_SHADER_STAGE_VERTEX_BIT,
            .module = create_infos.vertex_stage->shader_module,
            .pName  = "main",
        },
        {
            .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage  = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = create_infos.fragment_stage->shader_module,
            .pName  = "main",
        },
    };

    VkPipelineColorBlendStateCreateInfo color_blending{
        .sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .attachmentCount = static_cast<uint32_t>(color_blend_attachment.size()),
        .pAttachments    = color_blend_attachment.data(),
    };

    std::vector dynamic_states_array = {VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_VIEWPORT};
    if (create_infos.shader_properties.line_width != 1.0f)
        dynamic_states_array.emplace_back(VK_DYNAMIC_STATE_LINE_WIDTH);

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
        .layout              = create_infos.pipeline_layout->pipeline_layout,
        .renderPass          = render_pass->render_pass,
        .subpass             = 0,
        .basePipelineHandle  = VK_NULL_HANDLE,
        .basePipelineIndex   = -1,
    };

    VK_CHECK(vkCreateGraphicsPipelines(get_device(), VK_NULL_HANDLE, 1, &pipelineInfo, get_allocator(), &pipeline), "Failed to create material graphic pipeline");
    debug_set_object_name(name, pipeline);
}

PipelineResource_VK::~PipelineResource_VK()
{
    vkDestroyPipeline(get_device(), pipeline, get_allocator());
}

PipelineLayoutResource_VK::PipelineLayoutResource_VK(const std::string& name, const CreateInfos& create_infos) : descriptor_set_layout(create_infos.descriptor_set_layout)
{
    std::vector<VkPushConstantRange> push_constants = {};
    if (create_infos.vertex_push_constant_size > 0)
    {
        push_constants.emplace_back(VkPushConstantRange{
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            .offset     = 0,
            .size       = create_infos.vertex_push_constant_size,
        });
    }
    if (create_infos.fragment_push_constant_size > 0)
    {
        push_constants.emplace_back(VkPushConstantRange{
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .offset     = 0,
            .size       = create_infos.fragment_push_constant_size,
        });
    }

    const VkPipelineLayoutCreateInfo pipeline_layout_infos{
        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount         = 1,
        .pSetLayouts            = &create_infos.descriptor_set_layout->descriptor_set_layout,
        .pushConstantRangeCount = static_cast<uint32_t>(push_constants.size()),
        .pPushConstantRanges    = push_constants.data(),
    };
    VK_CHECK(vkCreatePipelineLayout(get_device(), &pipeline_layout_infos, nullptr, &pipeline_layout), "Failed to create pipeline layout");
    debug_set_object_name(name, pipeline_layout);
}

PipelineLayoutResource_VK::~PipelineLayoutResource_VK()
{
    vkDestroyPipelineLayout(get_device(), pipeline_layout, get_allocator());
}

ShaderModuleResource_VK::ShaderModuleResource_VK(const std::string& name, const CreateInfos& create_infos)
{
    const VkShaderModuleCreateInfo vertex_create_infos{
        .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext    = nullptr,
        .flags    = 0,
        .codeSize = create_infos.spirv_code.size() * sizeof(uint32_t),
        .pCode    = create_infos.spirv_code.data(),
    };
    VK_CHECK(vkCreateShaderModule(get_device(), &vertex_create_infos, get_allocator(), &shader_module), "failed to create vertex shader module");
    debug_set_object_name(name, shader_module);
}

ShaderModuleResource_VK::~ShaderModuleResource_VK()
{
    vkDestroyShaderModule(get_device(), shader_module, get_allocator());
}
} // namespace gfx::vulkan