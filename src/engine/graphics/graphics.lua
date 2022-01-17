
declare_module("graphics", {"application", "types", "shader_builder"}, {"cpputils", "glm"})

target("graphics")
	set_group("engine_new")
	add_headerfiles("private/vulkan/**.h")

if GRAPHIC_BACKEND == "VULKAN" then
	target("graphics")
		add_packages("vulkan-loader", "vulkan-memory-allocator", "vulkan-validationlayers")
else 
	target("graphics")
		del_files("private/vulkan/**.cpp")
end

if APPLICATION_BACKEND ~= "WIN32" then
target("graphics")
	del_files("private/vulkan/win32/**.cpp")
end