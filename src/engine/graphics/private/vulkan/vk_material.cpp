#include "vulkan/vk_material.h"

#include "vk_errors.h"
#include "vk_render_pass.h"
#include "vk_texture.h"
#include "vulkan/vk_allocator.h"
#include "vulkan/vk_device.h"

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
    }
}

void MasterMaterial_VK::create_modules(const shader_builder::CompilationResult& compilation_results)
{
    for (auto& pass : compilation_results.passes)
    {
        const RenderPassID pass_id = RenderPassID::get(pass.first);

        if (!pass_id)
            continue;

        MaterialPassData pass_data;

        VkShaderModuleCreateInfo vertex_create_infos{
            .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = pass.second.vertex.spirv.size() * sizeof(uint32_t),
            .pCode    = pass.second.vertex.spirv.data(),
        };
        VK_CHECK(vkCreateShaderModule(get_device(), &vertex_create_infos, get_allocator(), &pass_data.vertex_module), "failed to create vertex shader module");

        VkShaderModuleCreateInfo fragment_create_infos{
            .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = pass.second.fragment.spirv.size() * sizeof(uint32_t),
            .pCode    = pass.second.fragment.spirv.data(),
        };
        VK_CHECK(vkCreateShaderModule(get_device(), &fragment_create_infos, get_allocator(), &pass_data.fragment_module), "failed to create fragment shader module");

        per_pass_data[pass_id] = pass_data;
    }
}

void MasterMaterial_VK::clear()
{
    for (auto& pass : per_pass_data)
    {
        vkDestroyPipeline(get_device(), pass.pipeline, get_allocator());
        vkDestroyPipelineLayout(get_device(), pass.layout, get_allocator());
        vkDestroyShaderModule(get_device(), pass.vertex_module, get_allocator());
        vkDestroyShaderModule(get_device(), pass.fragment_module, get_allocator());
        for (const auto& desc_set_layout : pass.descriptor_set_layout)
            vkDestroyDescriptorSetLayout(get_device(), desc_set_layout, get_allocator());
    }
    per_pass_data.clear();
}

MasterMaterial_VK::~MasterMaterial_VK()
{
    clear();
}

static VkDescriptorType vk_descriptor_type(shader_builder::EBindingType type)
{
    switch (type)
    {
    case shader_builder::EBindingType::SAMPLER:
        return VK_DESCRIPTOR_TYPE_SAMPLER;
    case shader_builder::EBindingType::COMBINED_IMAGE_SAMPLER:
        return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    case shader_builder::EBindingType::SAMPLED_IMAGE:
        return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    case shader_builder::EBindingType::STORAGE_IMAGE:
        return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    case shader_builder::EBindingType::UNIFORM_TEXEL_BUFFER:
        return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
    case shader_builder::EBindingType::STORAGE_TEXEL_BUFFER:
        return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
    case shader_builder::EBindingType::UNIFORM_BUFFER:
        return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    case shader_builder::EBindingType::STORAGE_BUFFER:
        return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    case shader_builder::EBindingType::UNIFORM_BUFFER_DYNAMIC:
        return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    case shader_builder::EBindingType::STORAGE_BUFFER_DYNAMIC:
        return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
    case shader_builder::EBindingType::INPUT_ATTACHMENT:
        return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        break;
        LOG_FATAL("unhandled case");
    }
}

void MasterMaterial_VK::rebuild_material(const shader_builder::CompilationResult& compilation_results)
{
    MasterMaterial::rebuild_material(compilation_results);

    clear();
    create_modules(compilation_results);
    
    for (auto& pass_data : per_pass_data)
    {
        MaterialPassData&                 pass                    = pass_data.second;
        const shader_builder::PassResult& pass_compilation_result = compilation_results.passes.find(RenderPassID::get(pass_data)->second;
        const RenderPass_VK*              render_pass             = dynamic_cast<RenderPass_VK*>(RenderPass::find(pass_data));

        std::vector<VkDescriptorSetLayoutBinding> pipeline_bindings = {};

        for (const auto& binding : pass_compilation_result.vertex.reflection.bindings)
        {
            pipeline_bindings.emplace_back(VkDescriptorSetLayoutBinding{
                .binding            = binding.binding,
                .descriptorType     = vk_descriptor_type(binding.descriptor_type),
                .descriptorCount    = 1,
                .stageFlags         = VK_SHADER_STAGE_VERTEX_BIT,
                .pImmutableSamplers = nullptr,
            });
        }

        for (const auto& binding : pass_compilation_result.fragment.reflection.bindings)
        {
            pipeline_bindings.emplace_back(VkDescriptorSetLayoutBinding{
                .binding            = binding.binding,
                .descriptorType     = vk_descriptor_type(binding.descriptor_type),
                .descriptorCount    = 1,
                .stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT,
                .pImmutableSamplers = nullptr,
            });
        }

        for (auto& desc : pass.descriptor_set_layout)
        {
            VkDescriptorSetLayoutCreateInfo layout_infos{
                .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                .bindingCount = static_cast<uint32_t>(pipeline_bindings.size()),
                .pBindings    = pipeline_bindings.data(),
            };
            VK_CHECK(vkCreateDescriptorSetLayout(get_device(), &layout_infos, get_allocator(), &desc), "Failed to create descriptor set layout");
        }

        std::vector<VkPipelineShaderStageCreateInfo> shaderStages = {
            {
                .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .stage  = VK_SHADER_STAGE_VERTEX_BIT,
                .module = pass.vertex_module,
                .pName  = "main",
            },
            {
                .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .stage  = VK_SHADER_STAGE_FRAGMENT_BIT,
                .module = pass.fragment_module,
                .pName  = "main",
            },
        };

        VkPipelineLayoutCreateInfo pipeline_layout_infos{
            .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount         = static_cast<uint32_t>(pass.descriptor_set_layout.get_max_instance_count()),
            .pSetLayouts            = &pass.descriptor_set_layout[0],
            .pushConstantRangeCount = 0,
            .pPushConstantRanges    = nullptr,
        };
        VK_CHECK(vkCreatePipelineLayout(get_device(), &pipeline_layout_infos, nullptr, &pass.layout), "Failed to create pipeline layout");

        VkVertexInputBindingDescription bindingDescription{
            .binding   = 0,
            .stride    = pass_compilation_result.vertex.reflection.input_size,
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
        };

        std::vector<VkVertexInputAttributeDescription> vertex_attribute_description;

        for (const auto& input_property : pass_compilation_result.vertex.reflection.inputs)
        {
            vertex_attribute_description.emplace_back(VkVertexInputAttributeDescription{
                .location = input_property.location,
                .format   = Texture_VK::vk_texture_format_to_engine(input_property.type.format),
                .offset   = input_property.offset,
            });
        }

        VkPipelineVertexInputStateCreateInfo vertex_input_state{
            .sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .vertexBindingDescriptionCount   = static_cast<uint32_t>(bindingDescription.stride > 0 ? 1 : 0),
            .pVertexBindingDescriptions      = bindingDescription.stride > 0 ? &bindingDescription : nullptr,
            .vertexAttributeDescriptionCount = static_cast<uint32_t>(vertex_attribute_description.size()),
            .pVertexAttributeDescriptions    = vertex_attribute_description.data(),
        };

        VkPipelineInputAssemblyStateCreateInfo input_assembly{
            .sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .topology               = vk_topology(compilation_results.properties.topology),
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
            .polygonMode             = vk_polygon_mode(compilation_results.properties.polygon_mode),
            .cullMode                = vk_cull_mode(compilation_results.properties.culling),
            .frontFace               = vk_front_face(compilation_results.properties.front_face),
            .depthBiasEnable         = VK_FALSE,
            .depthBiasConstantFactor = 0.0f,
            .depthBiasClamp          = 0.0f,
            .depthBiasSlopeFactor    = 0.0f,
            .lineWidth               = compilation_results.properties.line_width,
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
            .depthTestEnable       = compilation_results.properties.depth_test,
            .depthWriteEnable      = compilation_results.properties.depth_test,
            .depthCompareOp        = VK_COMPARE_OP_LESS,
            .depthBoundsTestEnable = VK_FALSE,
            .stencilTestEnable     = VK_FALSE,
            .minDepthBounds        = 0.0f,
            .maxDepthBounds        = 1.0f,
        };

        std::vector<VkPipelineColorBlendAttachmentState> color_blend_attachment;
        for (const auto& attachment : render_pass->get_config().color_attachments)
        {
            color_blend_attachment.emplace_back(VkPipelineColorBlendAttachmentState{
                .blendEnable         = compilation_results.properties.alpha_mode == shader_builder::EAlphaMode::Opaque ? VK_FALSE : VK_TRUE,
                .srcColorBlendFactor = compilation_results.properties.alpha_mode == shader_builder::EAlphaMode::Opaque ? VK_BLEND_FACTOR_ONE : VK_BLEND_FACTOR_SRC_ALPHA,
                .dstColorBlendFactor = compilation_results.properties.alpha_mode == shader_builder::EAlphaMode::Opaque ? VK_BLEND_FACTOR_ZERO : VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                .colorBlendOp        = VK_BLEND_OP_ADD,
                .srcAlphaBlendFactor = compilation_results.properties.alpha_mode == shader_builder::EAlphaMode::Opaque ? VK_BLEND_FACTOR_ONE : VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
                .alphaBlendOp        = VK_BLEND_OP_ADD,
                .colorWriteMask      = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
            });
        }

        VkPipelineColorBlendStateCreateInfo color_blending{
            .sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .attachmentCount = static_cast<uint32_t>(color_blend_attachment.size()),
            .pAttachments    = color_blend_attachment.data(),
        };

        std::vector dynamic_states_array = {VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_VIEWPORT};
        if (compilation_results.properties.line_width != 1.0f)
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
            .layout              = pass.layout,
            .renderPass          = render_pass->get(),
            .subpass             = 0,
            .basePipelineHandle  = VK_NULL_HANDLE,
            .basePipelineIndex   = -1,
        };

        VK_CHECK(vkCreateGraphicsPipelines(get_device(), VK_NULL_HANDLE, 1, &pipelineInfo, get_allocator(), &pass.pipeline), "Failed to create material graphic pipeline");
    }
}
} // namespace gfx::vulkan
