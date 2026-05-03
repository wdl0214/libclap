# Basic example
add_executable(basic basic.c)
target_link_libraries(basic PRIVATE clap_static)
target_compile_definitions(basic PRIVATE CLAP_STATIC)
set_clap_sanitizers(basic)

# Git style example
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/git_style.c)
    add_executable(git_style git_style.c)
    target_link_libraries(git_style PRIVATE clap_static)
    target_compile_definitions(git_style PRIVATE CLAP_STATIC)
    set_clap_sanitizers(git_style)
endif()

# Action demo example
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/action_demo.c)
    add_executable(action_demo action_demo.c)
    target_link_libraries(action_demo PRIVATE clap_static)
    target_compile_definitions(action_demo PRIVATE CLAP_STATIC)
    set_clap_sanitizers(action_demo)
endif()