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
    src/level/block.cpp
    src/level/chunk.cpp
    src/level/world.cpp
    src/level/worldgen.cpp
    src/level/collision.cpp
    src/level/chunk_mesh.cpp
    src/level/octree.cpp

    src/engine/config.cpp
    src/engine/logger.cpp
	src/engine/input.cpp
    src/engine/resource_manager.cpp
    
    src/graphics/mesh.cpp
    src/graphics/graphics.cpp
    src/graphics/text.cpp
    
    src/util/hash.cpp
    src/util/point.cpp
    src/util/fileio.cpp
	src/util/util.cpp
    src/util/timer.cpp
    
    src/entities/entity.cpp
    src/entities/entity_model.cpp
    
    src/serialization/ini_parser.cpp
    src/serialization/serialize.cpp
    
    src/main.cpp

    external/glad.c
    external/stb_implementation.c
)

target_link_libraries(fakecraft PRIVATE
    OpenGL::GL
    glfw
)

target_include_directories(fakecraft PRIVATE
    src/
    external/
)
