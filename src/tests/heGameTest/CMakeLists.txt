
file(GLOB_RECURSE SOURCES *.cpp *.h)
add_executable(heGameTest ${SOURCES})
configure_project(heGameTest ${SOURCES})
target_link_libraries(heGameTest HeadlessEngine)

set_target_properties(heGameTest PROPERTIES FOLDER Tests)
target_include_directories(heGameTest PUBLIC public)