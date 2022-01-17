
declare_module("game_test", {"engine", "base_renderers", "scene_importer"}, {"glfw", "cpputils", "vulkan-loader", "glm", "imgui", "vulkan-memory-allocator", "spirv-cross", "assimp"}, true)

target("game_test")
	set_group("tests")