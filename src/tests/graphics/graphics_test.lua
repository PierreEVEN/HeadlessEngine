
declare_module("graphics_test", {"graphics", "ecs"}, {"cpputils", "vulkan-validationlayers", "glm"}, true)

target("graphics_test")
	set_group("tests")