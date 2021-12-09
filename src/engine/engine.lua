

add_requires("glfw", "glm", "cpputils", "vulkan-hpp", "vulkan-loader", "vulkan-validationlayers", "vulkan-memory-allocator", "glslang", "spirv-cross", "imgui docking", "imguizmo")

declare_module("engine", {"config", "utils", "job_system"}, {"glfw", "glm", "cpputils", "vulkan-hpp", "vulkan-memory-allocator", "glslang", "spirv-cross", "vulkan-loader", "vulkan-validationlayers", "imgui", "imguizmo"})