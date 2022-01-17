#pragma once

#include "rendering/vulkan/utils.h"

namespace vulkan_common
{

void vulkan_init();
void vulkan_shutdown();

void create_instance();
void create_validation_layers();

void destroy_validation_layers();
void destroy_instance();

inline VkAllocationCallbacks* allocation_callback = nullptr;
inline VkInstance             instance;

void set_msaa_sample_count(uint32_t in_sample_count);
uint32_t get_msaa_sample_count();

} // namespace vulkan_common