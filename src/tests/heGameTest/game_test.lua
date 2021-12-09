
add_requires("glfw", "cpputils", "vulkan-loader", "glm", "imgui docking", "vulkan-memory-allocator", "spirv-cross", "assimp")

declare_module("game_test", {"engine", "base_renderers", "scene_importer"}, {"glfw", "cpputils", "vulkan-loader", "glm", "imgui", "vulkan-memory-allocator", "spirv-cross", "assimp"}, true)