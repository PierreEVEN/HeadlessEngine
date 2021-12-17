
declare_module("graphics_test", {"graphics"}, {"cpputils", "vulkan-validationlayers"}, true)

target("graphics_test")
	set_group("tests")