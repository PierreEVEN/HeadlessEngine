

#include "rendering/vulkan/material_pipeline.h"

#include "assets/asset_mesh_data.h"
#include "engine_interface.h"
#include "rendering/gfx_context.h"
#include "rendering/renderer/render_pass.h"
#include "rendering/vulkan/common.h"
#include "rendering/vulkan/descriptor_pool.h"
#include "rendering/vulkan/utils.h"

#include <vulkan/vulkan.h>

void MaterialPipeline::rebuild()
{
    if (should_recreate)
    {
        destroy();
        create_descriptor_sets(layout_bindings);
        create_pipeline_layout();
        create_pipeline();
        should_recreate = false;
    }
}

void MaterialPipeline::destroy()
{
    if (pipeline_layout != VK_NULL_HANDLE)
        vkDestroyPipelineLayout(GfxContext::get()->logical_device, pipeline_layout, vulkan_common::allocation_callback);
    if (pipeline != VK_NULL_HANDLE)
        vkDestroyPipeline(GfxContext::get()->logical_device, pipeline, vulkan_common::allocation_callback);
    if (descriptor_set_layout != VK_NULL_HANDLE)
        vkDestroyDescriptorSetLayout(GfxContext::get()->logical_device, descriptor_set_layout, vulkan_common::allocation_callback);

    pipeline_layout       = VK_NULL_HANDLE;
    pipeline              = VK_NULL_HANDLE;
    descriptor_sets       = {};
    descriptor_set_layout = VK_NULL_HANDLE;
}

void MaterialPipeline::create_pipeline_layout()
{

    /** Pipeline layout */
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{
        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount         = 1,
        .pSetLayouts            = &descriptor_set_layout,
        .pushConstantRangeCount = static_cast<uint32_t>(push_constant_range ? 1 : 0),
        .pPushConstantRanges    = push_constant_range.get(),
    };
    VK_ENSURE(vkCreatePipelineLayout(GfxContext::get()->logical_device, &pipelineLayoutInfo, nullptr, &pipeline_layout), "Failed to create pipeline layout");
}

void MaterialPipeline::create_pipeline()
{
    VK_CHECK(descriptor_set_layout, "Descriptor set layout should be initialized before graphic pipeline");
    VK_CHECK(IEngineInterface::get()->get_window()->get_render_pass(), "Render pass should be initialized before graphic pipeline");

    VK_CHECK(vertex_module, "vertex module should not be null");
    VK_CHECK(fragment_module, "fragment module should not be null");

    /** AShader pipeline */
    VkPipelineShaderStageCreateInfo vertex_shader_stage{
        .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage  = VK_SHADER_STAGE_VERTEX_BIT,
        .module = vertex_module->get_shader_module(),
        .pName  = "main",
    };

    VkPipelineShaderStageCreateInfo fragment_shader_stage{
        .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage  = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = fragment_module->get_shader_module(),
        .pName  = "main",
    };

    // PUSH CONSTANTS
    VkPipelineShaderStageCreateInfo shaderStages[] = {vertex_shader_stage, fragment_shader_stage};

    auto bindingDescription    = Vertex::get_binding_description();
    auto attributeDescriptions = Vertex::get_attribute_descriptions();

    VkPipelineVertexInputStateCreateInfo vertex_input_state{
        .sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount   = 1,
        .pVertexBindingDescriptions      = &bindingDescription,
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
        .pVertexAttributeDescriptions    = attributeDescriptions.data(),
    };

    VkPipelineInputAssemblyStateCreateInfo input_assembly{
        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology               = topology,
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
        .polygonMode             = polygon_mode,
        .cullMode                = VK_CULL_MODE_BACK_BIT,
        .frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .depthBiasEnable         = VK_FALSE,
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp          = 0.0f,
        .depthBiasSlopeFactor    = 0.0f,
        .lineWidth               = wireframe_lines_width,
    };

    VkPipelineMultisampleStateCreateInfo multisampling{
        .sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples  = static_cast<VkSampleCountFlagBits>(vulkan_common::get_msaa_sample_count()),
        .sampleShadingEnable   = static_cast<VkBool32>(vulkan_common::get_msaa_sample_count() > 1 ? VK_TRUE : VK_FALSE),
        .minSampleShading      = 1.0f,     // Optional
        .pSampleMask           = nullptr,  // Optional
        .alphaToCoverageEnable = VK_FALSE, // Optional
        .alphaToOneEnable      = VK_FALSE, // Optional
    };

    VkPipelineDepthStencilStateCreateInfo depth_stencil{
        .sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable       = static_cast<VkBool32>(depth_test ? VK_TRUE : VK_FALSE),
        .depthWriteEnable      = static_cast<VkBool32>(depth_test ? VK_TRUE : VK_FALSE),
        .depthCompareOp        = VK_COMPARE_OP_LESS,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable     = VK_FALSE,
        .minDepthBounds        = 0.0f, // Optional
        .maxDepthBounds        = 1.0f, // Optional
    };

    VkPipelineColorBlendAttachmentState color_blend_attachment{
        .blendEnable         = VK_FALSE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,  // Optional
        .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO, // Optional
        .colorBlendOp        = VK_BLEND_OP_ADD,      // Optional
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,  // Optional
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO, // Optional
        .alphaBlendOp        = VK_BLEND_OP_ADD,      // Optional
        .colorWriteMask      = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    };

    VkPipelineColorBlendStateCreateInfo color_blending{
        .sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable   = VK_FALSE,
        .logicOp         = VK_LOGIC_OP_COPY, // Optional
        .attachmentCount = 1,
        .pAttachments    = &color_blend_attachment,
        .blendConstants  = {0.0f, 0.f, 0.f, 0.f}, // Optional
    };

    VkDynamicState                   dynamic_states_array[] = {VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_LINE_WIDTH};
    VkPipelineDynamicStateCreateInfo dynamic_states{
        .sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = 2,
        .pDynamicStates    = dynamic_states_array,
    };

    VkGraphicsPipelineCreateInfo pipelineInfo{
        .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount          = 2,
        .pStages             = shaderStages,
        .pVertexInputState   = &vertex_input_state,
        .pInputAssemblyState = &input_assembly,
        .pViewportState      = &viewport_state,
        .pRasterizationState = &rasterizer,
        .pMultisampleState   = &multisampling,
        .pDepthStencilState  = &depth_stencil, // Optional
        .pColorBlendState    = &color_blending,
        .pDynamicState       = &dynamic_states, // Optional
        .layout              = pipeline_layout,
        .renderPass          = IEngineInterface::get()->get_window()->get_render_pass()->get_render_pass(),
        .subpass             = 0,
        .basePipelineHandle  = VK_NULL_HANDLE, // Optional
        .basePipelineIndex   = -1,             // Optional
    };

    VK_ENSURE(vkCreateGraphicsPipelines(GfxContext::get()->logical_device, VK_NULL_HANDLE, 1, &pipelineInfo, vulkan_common::allocation_callback, &pipeline), "Failed to create material graphic pipeline");
}

void MaterialPipeline::mark_dirty()
{
    should_recreate = true;
}

void MaterialPipeline::create_descriptor_sets(std::vector<VkDescriptorSetLayoutBinding> layoutBindings)
{
    /** Create descriptor set layout */
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
    layoutInfo.pBindings    = layoutBindings.data();
    VK_ENSURE(vkCreateDescriptorSetLayout(GfxContext::get()->logical_device, &layoutInfo, vulkan_common::allocation_callback, &descriptor_set_layout), "Failed to create descriptor set layout");

    /** Allocate descriptor set */
    std::vector<VkDescriptorSetLayout> layouts(IEngineInterface::get()->get_window()->get_swapchain()->get_image_count(), descriptor_set_layout);
    descriptor_sets.resize(IEngineInterface::get()->get_window()->get_swapchain()->get_image_count());
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorSetCount = IEngineInterface::get()->get_window()->get_swapchain()->get_image_count();
    allocInfo.pSetLayouts        = layouts.data();
    allocInfo.descriptorPool     = VK_NULL_HANDLE;
    IEngineInterface::get()->get_window()->get_descriptor_pool()->alloc_memory(allocInfo);
    VK_ENSURE(vkAllocateDescriptorSets(GfxContext::get()->logical_device, &allocInfo, descriptor_sets.data()), "Failed to allocate descriptor sets");
}

MaterialPipeline::~MaterialPipeline()
{
    destroy();
}

void MaterialPipeline::set_vertex_module(std::shared_ptr<ShaderModule> in_module)
{
    vertex_module = in_module;
    mark_dirty();
}

void MaterialPipeline::set_fragment_module(std::shared_ptr<ShaderModule> in_module)
{
    fragment_module = in_module;
    mark_dirty();
}

void MaterialPipeline::enable_depth_test(bool b_in_depth_test)
{
    depth_test = b_in_depth_test;
    mark_dirty();
}

void MaterialPipeline::set_wireframe(bool b_in_wireframe)
{
    wireframe = b_in_wireframe;
    mark_dirty();
}

void MaterialPipeline::set_wireframe_width(float in_width)
{
    wireframe_lines_width = in_width;
    mark_dirty();
}

void MaterialPipeline::set_push_constant_ranges(std::shared_ptr<VkPushConstantRange> in_push_constants)
{
    push_constant_range = in_push_constants;
    mark_dirty();
}

void MaterialPipeline::set_topology(VkPrimitiveTopology in_topology)
{
    topology = in_topology;
    mark_dirty();
}

void MaterialPipeline::set_polygon_mode(VkPolygonMode in_mode)
{
    polygon_mode = in_mode;
    mark_dirty();
}

void MaterialPipeline::set_layout_bindings(const std::vector<VkDescriptorSetLayoutBinding>& in_bindings)
{
    layout_bindings = in_bindings;
    mark_dirty();
}
