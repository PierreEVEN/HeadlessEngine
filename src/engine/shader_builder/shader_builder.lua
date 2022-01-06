
declare_module("shader_builder", {"types"}, {"cpputils", "glm", "glslang", "spirv-reflect"})

target("shader_builder")
	set_group("engine_new")
	add_headerfiles("private/(**.h)")
	
	
if GRAPHIC_BACKEND == "VULKAN" then
	target("shader_builder")
		add_packages("vulkan-hpp", "vulkan-loader", "vulkan-memory-allocator", "vulkan-validationlayers")
		add_defines("ENABLE_HLSL");
end