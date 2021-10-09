
file(GLOB_RECURSE SOURCES *.cpp *.h)
add_library(SceneImporter SHARED ${SOURCES})
configure_project(SceneImporter "${SOURCES}")

# Dependencies
target_link_libraries(SceneImporter HeadlessEngine)
target_include_directories(HeadlessEngine PUBLIC public)