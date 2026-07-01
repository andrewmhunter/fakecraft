cmake_minimum_required(VERSION 3.24)
project(Fakecraft)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 23)

find_package(OpenGL REQUIRED OpenGL)
find_package(glfw3 3.4 REQUIRED)

add_executable(fakecraft)

if(MSVC)
    target_compile_options(fakecraft PRIVATE /Wall /O2)
else()
    target_compile_options(fakecraft PRIVATE -Wall -Wextra -O3)
endif()

target_sources(fakecraft PRIVATE
    src/main.cpp
    src/mesh.cpp
    src/block.cpp
    src/chunk.cpp
    src/world.cpp
	src/util.cpp
    src/worldgen.cpp
    src/chunk_mesh.cpp
    src/entity.cpp
    src/collision.cpp
	src/serialize.cpp
    src/logger.cpp
    src/hash.cpp
    src/point.cpp
    src/fileio.cpp
    src/timer.cpp
    src/graphics.cpp
	src/input.cpp
    src/text.cpp
    src/ini_parser.cpp
    src/config.cpp

    external/glad.c
    external/stb_implementation.c
)

target_link_libraries(fakecraft PRIVATE
    OpenGL::GL
    glfw
)

target_include_directories(fakecraft PRIVATE
    external/
)
