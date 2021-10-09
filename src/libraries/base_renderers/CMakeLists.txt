
file(GLOB_RECURSE SOURCES *.cpp *.h)
add_library(BaseRenderers SHARED ${SOURCES})
configure_project(BaseRenderers "${SOURCES}")

# Dependencies
target_link_libraries(BaseRenderers HeadlessEngine)
target_include_directories(HeadlessEngine PUBLIC public)