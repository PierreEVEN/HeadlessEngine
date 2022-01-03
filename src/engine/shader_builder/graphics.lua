
declare_module("shader_builder", {"types"}, {"cpputils", "glm", "glslang", "spirv-tools", "spirv-reflect"})

if GRAPHIC_BACKEND == "VULKAN" then
	target("graphics")
		add_packages("vulkan-hpp", "vulkan-loader", "vulkan-memory-allocator", "vulkan-validationlayers")
end