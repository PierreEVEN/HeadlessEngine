#pragma once
#include <vector>

namespace config {

	/**
	 * Application
	 */
	
	inline const size_t application_version_major = 1;
	inline const size_t application_version_minor = 0;
	inline const size_t application_version_patch = 0;

	inline const size_t engine_version_major = 1;
	inline const size_t engine_version_minor = 0;
	inline const size_t engine_version_patch = 0;
	
	inline const char* application_name = "Anonymous application";
	inline const char* engine_name = "Headless Engine";

	/**
	 * Vulkan
	 */
	
	inline const bool use_validation_layers = true;
        inline const std::vector required_validation_layers = {"VK_LAYER_KHRONOS_validation"};
	inline const std::vector required_device_extensions = { "VK_KHR_swapchain" };
	inline const uint32_t max_frame_in_flight = 2;

	inline const uint32_t max_descriptor_per_pool = 64;
	inline const uint32_t max_descriptor_per_type = 128;

	/**
	 * Engine
	 */

	inline const char* profiler_storage_path = "saved/profiler/";
	inline const char* log_storage_path = "saved/log/";
	
}
