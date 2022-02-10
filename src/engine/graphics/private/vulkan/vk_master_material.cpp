#include "vulkan/vk_master_material.h"

#include "vk_errors.h"
#include "vk_render_pass.h"
#include "vk_types.h"
#include "vulkan/vk_allocator.h"
#include "vulkan/vk_device.h"
#include "vulkan/vk_helper.h"

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

DescriptorSetLayoutResource_VK::DescriptorSetLayoutResource_VK(const std::string& name, const CI_DescriptorSetLayout& create_infos)
{
    std::vector<VkDescriptorSetLayoutBinding> bindings;
    for (const auto& binding : create_infos.vertex_reflection_data.bindings)
    {
        bindings.emplace_back(VkDescriptorSetLayoutBinding{
            .binding            = binding.binding,
            .descriptorType     = MasterMaterial_VK::vk_descriptor_type(binding.descriptor_type),
            .descriptorCount    = 1,
            .stageFlags         = VK_SHADER_STAGE_VERTEX_BIT,
            .pImmutableSamplers = nullptr,
        });
    }

    for (const auto& binding : create_infos.fragment_reflection_data.bindings)
    {
        bindings.emplace_back(VkDescriptorSetLayoutBinding{
            .binding            = binding.binding,
            .descriptorType     = MasterMaterial_VK::vk_descriptor_type(binding.descriptor_type),
            .descriptorCount    = 1,
            .stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = nullptr,
        });
    }

    VkDescriptorSetLayoutCreateInfo layout_infos{
        .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext        = nullptr,
        .flags        = 0,
        .bindingCount = static_cast<uint32_t>(bindings.size()),
        .pBindings    = bindings.data(),
    };
    VK_CHECK(vkCreateDescriptorSetLayout(get_device(), &layout_infos, get_allocator(), &descriptor_set_layout), "Failed to create descriptor set layout");
    debug_set_object_name(name, descriptor_set_layout);
}

DescriptorSetLayoutResource_VK::~DescriptorSetLayoutResource_VK()
{
    vkDestroyDescriptorSetLayout(get_device(), descriptor_set_layout, get_allocator());
}

PipelineResource_VK::PipelineResource_VK(const std::string& name, const CI_Pipeline& create_infos) : parameters(create_infos)
{
    const RenderPass_VK* render_pass = dynamic_cast<RenderPass_VK*>(RenderPass::find(create_infos.pass_id));

    std::vector<VkVertexInputAttributeDescription> vertex_attribute_description;
    const auto                                     inputs = parameters.material_options.input_stage_override ? parameters.material_options.input_stage_override.value() : create_infos.vertex_inputs;

    uint32_t vertex_input_size = 0;
    for (const auto& input_property : inputs)
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

    VkVertexInputBindingDescription bindingDescription{
        .binding   = 0,
        .stride    = vertex_input_size,
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
    };

    VkPipelineVertexInputStateCreateInfo vertex_input_state{
        .sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount   = static_cast<uint32_t>(bindingDescription.stride > 0 ? 1 : 0),
        .pVertexBindingDescriptions      = bindingDescription.stride > 0 ? &bindingDescription : nullptr,
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(vertex_attribute_description.size()),
        .pVertexAttributeDescriptions    = vertex_attribute_description.data(),
    };

    VkPipelineInputAssemblyStateCreateInfo input_assembly{
        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology               = vk_topology(create_infos.compilation_results.properties.topology),
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
        .polygonMode             = vk_polygon_mode(create_infos.compilation_results.properties.polygon_mode),
        .cullMode                = vk_cull_mode(create_infos.compilation_results.properties.culling),
        .frontFace               = vk_front_face(create_infos.compilation_results.properties.front_face),
        .depthBiasEnable         = VK_FALSE,
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp          = 0.0f,
        .depthBiasSlopeFactor    = 0.0f,
        .lineWidth               = create_infos.compilation_results.properties.line_width,
    };

    VkPipelineMultisampleStateCreateInfo multisampling{
        .sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable   = VK_FALSE,
        .minSampleShading      = 1.0f,
        .pSampleMask           = nullptr,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable      = VK_FALSE,
    };

    VkPipelineDepthStencilStateCreateInfo depth_stencil{
        .sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable       = create_infos.compilation_results.properties.depth_test,
        .depthWriteEnable      = create_infos.compilation_results.properties.depth_test,
        .depthCompareOp        = VK_COMPARE_OP_LESS,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable     = VK_FALSE,
        .minDepthBounds        = 0.0f,
        .maxDepthBounds        = 1.0f,
    };

    std::vector<VkPipelineColorBlendAttachmentState> color_blend_attachment;
    for ([[maybe_unused]] const auto& attachment : render_pass->get_config().color_attachments)
    {
        color_blend_attachment.emplace_back(VkPipelineColorBlendAttachmentState{
            .blendEnable         = create_infos.compilation_results.properties.alpha_mode == shader_builder::EAlphaMode::Opaque ? VK_FALSE : VK_TRUE,
            .srcColorBlendFactor = create_infos.compilation_results.properties.alpha_mode == shader_builder::EAlphaMode::Opaque ? VK_BLEND_FACTOR_ONE : VK_BLEND_FACTOR_SRC_ALPHA,
            .dstColorBlendFactor = create_infos.compilation_results.properties.alpha_mode == shader_builder::EAlphaMode::Opaque ? VK_BLEND_FACTOR_ZERO : VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
            .colorBlendOp        = VK_BLEND_OP_ADD,
            .srcAlphaBlendFactor = create_infos.compilation_results.properties.alpha_mode == shader_builder::EAlphaMode::Opaque ? VK_BLEND_FACTOR_ONE : VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
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
    if (create_infos.compilation_results.properties.line_width != 1.0f)
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
        .renderPass          = render_pass->get()->render_pass,
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

PipelineLayoutResource_VK::PipelineLayoutResource_VK(const std::string& name, const CI_PipelineLayout& create_infos) : parameters(create_infos)
{
    std::vector<VkPushConstantRange> push_constants = {};
    if (create_infos.vertex_reflection_data.push_constant)
    {
        push_constants.emplace_back(VkPushConstantRange{
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            .offset     = 0,
            .size       = parameters.vertex_reflection_data.push_constant->structure_size,
        });
    }
    if (create_infos.fragment_reflection_data.push_constant)
    {
        push_constants.emplace_back(VkPushConstantRange{
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .offset     = 0,
            .size       = parameters.fragment_reflection_data.push_constant->structure_size,
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

ShaderModuleResource_VK::ShaderModuleResource_VK(const std::string& name, const CI_ShaderModule& create_infos)
{
    VkShaderModuleCreateInfo vertex_create_infos{
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

MasterMaterial_VK::~MasterMaterial_VK()
{
    clear();
}

void MasterMaterial_VK::create_modules(const shader_builder::CompilationResult& compilation_results)
{
    for (auto& pass : compilation_results.passes)
    {
        if (!RenderPassID::exists(pass.first))
            continue;

        MaterialPassData& pass_data = per_pass_data.init(RenderPassID::get(pass.first));
        pass_data.vertex_module     = TGpuHandle<ShaderModuleResource_VK>("test sm vertex", ShaderModuleResource_VK::CI_ShaderModule{
                                                                                            .spirv_code = pass.second.vertex.spirv,
                                                                                        });
        pass_data.fragment_module     = TGpuHandle<ShaderModuleResource_VK>("test sm fragment", ShaderModuleResource_VK::CI_ShaderModule{
                                                                                              .spirv_code = pass.second.fragment.spirv,
                                                                                          });
    }
}

void MasterMaterial_VK::clear()
{
    per_pass_data.clear();
}

void MasterMaterial_VK::rebuild_material(const shader_builder::CompilationResult& compilation_results)
{
    MasterMaterial::rebuild_material(compilation_results);
    clear();

    create_modules(compilation_results);

    auto pass_data = per_pass_data.begin();
    for (; pass_data != per_pass_data.end(); ++pass_data)
    {
        std::vector<VkDescriptorSetLayoutBinding> pipeline_bindings = {};

        const auto vertex_reflection_data   = get_vertex_reflection(pass_data.id());
        const auto fragment_reflection_data = get_fragment_reflection(pass_data.id());

        pass_data->descriptor_set_layout = TGpuHandle<DescriptorSetLayoutResource_VK>("desc set layout test ", DescriptorSetLayoutResource_VK::CI_DescriptorSetLayout{
                                                                                                                   .vertex_reflection_data   = vertex_reflection_data,
                                                                                                                   .fragment_reflection_data = fragment_reflection_data,
                                                                                                               });
        pass_data->pipeline_layout       = TGpuHandle<PipelineLayoutResource_VK>("test pipeline layout", PipelineLayoutResource_VK::CI_PipelineLayout{
                                                                                                       .vertex_reflection_data   = vertex_reflection_data,
                                                                                                       .fragment_reflection_data = fragment_reflection_data,
                                                                                                       .descriptor_set_layout    = pass_data->descriptor_set_layout,
                                                                                                   });
        pass_data->pipeline              = TGpuHandle<PipelineResource_VK>("test pipeline", PipelineResource_VK::CI_Pipeline{
                                                                                   .vertex_stage        = pass_data->vertex_module,
                                                                                   .fragment_stage      = pass_data->fragment_module,
                                                                                   .pipeline_layout     = pass_data->pipeline_layout,
                                                                                   .compilation_results = compilation_results,
                                                                                   .pass_id             = pass_data.id(),
                                                                                   .material_options    = material_options,
                                                                                   .vertex_inputs       = get_vertex_reflection(pass_data.id()).inputs,
                                                                               });
    }
}
} // namespace gfx::vulkan
