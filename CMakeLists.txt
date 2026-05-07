cmake_minimum_required(VERSION 3.20)

project(BlockWorld)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# GLFW
add_subdirectory(third_party/glfw/glfw-3.4)

# GLAD
add_library(glad
    third_party/glad/src/glad.c
)

target_include_directories(glad PUBLIC
    third_party/glad/include
)

# Source files
file(GLOB_RECURSE CLIENT_SOURCES
    src/client/*.cpp
    src/core/*.cpp
    src/renderer/*.cpp
    src/world/*.cpp
    src/player/*.cpp
)

add_executable(BlockWorldClient ${CLIENT_SOURCES})

target_include_directories(BlockWorldClient PRIVATE
    src
    third_party/glad/include
    third_party/glfw/glfw-3.4/include
    third_party/glm
    third_party/nlohmann
    third_party/stb
)

target_link_libraries(BlockWorldClient
    glfw
    glad
    opengl32
)

if (WIN32)
    target_compile_definitions(BlockWorldClient PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

add_custom_command(TARGET BlockWorldClient POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/resources
    $<TARGET_FILE_DIR:BlockWorldClient>/resources
)