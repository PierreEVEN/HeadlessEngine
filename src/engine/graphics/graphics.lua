
declare_module("graphics", {"application", "types"}, {"cpputils", "glm"})

target("graphics")
	set_group("engine_new")

if GRAPHIC_BACKEND == "VULKAN" then
	target("graphics")
		add_packages("vulkan-hpp", "vulkan-loader", "vulkan-memory-allocator", "vulkan-validationlayers")
else 
	target("graphics")
		del_files("private/vulkan/**.cpp")
end

if APPLICATION_BACKEND ~= "WIN32" then
target("graphics")
	del_files("private/vulkan/win32/**.cpp")
end