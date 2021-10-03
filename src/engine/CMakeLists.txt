
file(GLOB_RECURSE SOURCES *.cpp *.h)
add_library(HeadlessEngine SHARED ${SOURCES})
configure_project(HeadlessEngine "${SOURCES}")

# VMA
target_include_directories(HeadlessEngine PUBLIC ${VulkanMemoryAllocator_SOURCE_DIR}/include)

# VULKAN
set(VULKAN_PATH $ENV{VULKAN_SDK})
target_include_directories(HeadlessEngine PUBLIC ${VULKAN_PATH}/Include)
IF (WIN32)
	target_link_libraries(HeadlessEngine ${VULKAN_PATH}/Lib/vulkan-1.lib)
ELSE()
	target_link_libraries(HeadlessEngine -lvulkan)
ENDIF()

# Dependencies
target_link_libraries(HeadlessEngine Utils)
target_link_libraries(HeadlessEngine JobSystem)
target_link_libraries(HeadlessEngine assimp)
target_link_libraries(HeadlessEngine glfw)
target_link_libraries(HeadlessEngine cpputils)
target_link_libraries(HeadlessEngine SPIRV-Cross_lib)
target_link_libraries(HeadlessEngine glslang_lib)
target_link_libraries(HeadlessEngine glm)
target_link_libraries(HeadlessEngine imgui_interface)
target_link_libraries(HeadlessEngine imguizmo_interface)
target_include_directories(HeadlessEngine PUBLIC ${stb_SOURCE_DIR})

target_include_directories(HeadlessEngine PUBLIC public)