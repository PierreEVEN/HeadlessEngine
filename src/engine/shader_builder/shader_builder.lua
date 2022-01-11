
declare_module("shader_builder", {"types"}, {"cpputils", "glm", "glslang", "spirv-reflect", "directxshadercompilerfixed"})

target("shader_builder")
	set_group("engine_new")
	add_headerfiles("private/(**.h)")
	add_includedirs("private/backend/Nazara/Public/", {public = false})
	add_headerfiles("private/backend/Nazara/Public/(**.hpp)")
	add_headerfiles("private/backend/Nazara/Public/(**.inl)")
	add_defines("NAZARA_STATIC")
	
	
if GRAPHIC_BACKEND == "VULKAN" then
	target("shader_builder")
		add_packages("vulkan-hpp", "vulkan-loader", "vulkan-memory-allocator", "vulkan-validationlayers")
		add_defines("ENABLE_HLSL");
end