cmake_minimum_required(VERSION 3.21)

project(Clearwater
  LANGUAGES CXX C
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(CW_BUILD_TESTS "Build unit tests" ON)

add_subdirectory(vendor/tomlc17)

add_library(cw_engine STATIC
  engine/engine/engine.cpp
  engine/platform/platform_windows.cpp
  engine/input/input.cpp
  engine/logger/log.cpp
  engine/graphics/graphics.cpp
  engine/graphics/null/null_graphics.cpp
  engine/graphics/opengl/opengl_platform_win32.cpp
  engine/graphics/opengl/opengl_graphics.cpp
  engine/graphics/opengl/gl_functions.cpp
  engine/render/render.cpp
  engine/render/pipeline.cpp
  engine/render/material.cpp
  engine/render/mesh.cpp
  engine/render/constant_buffer.cpp
  engine/filesystem/filesystem.cpp
  engine/filesystem/file_provider.cpp
  engine/assets/assets_manager.cpp
  engine/assets/resource_cache.cpp
  engine/assets/resources.cpp
  engine/assets/res_pipeline.cpp
  engine/assets/res_material.cpp
  engine/memory/memory_utils.cpp
)

target_include_directories(cw_engine PUBLIC
  ${CMAKE_SOURCE_DIR}/engine
  ${CMAKE_SOURCE_DIR}/vendor
)

target_link_libraries(cw_engine PUBLIC tomlc17)

target_compile_definitions(cw_engine PUBLIC
  $<$<CONFIG:Debug>:CW_BUILD_DEBUG=1>
  $<$<CONFIG:Debug>:CW_ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets">
  _CRT_SECURE_NO_WARNINGS
)

if (WIN32)
  target_compile_definitions(cw_engine PUBLIC CW_PLATFORM_WINDOWS)
  target_link_libraries(cw_engine PUBLIC user32.lib gdi32.lib opengl32.lib)
endif()

if (MSVC)
  target_compile_options(cw_engine PUBLIC /Zc:preprocessor)
endif()

add_executable(game
  engine/engine/main.cpp
  game/game.cpp
)

target_link_libraries(game PRIVATE cw_engine)

execute_process(
  COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_BINARY_DIR}/compile_commands.json
  ${CMAKE_SOURCE_DIR}/compile_commands.json
)

if (CW_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
