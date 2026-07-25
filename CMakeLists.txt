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
  engine/logger/log.cpp
  engine/graphics/renderer.cpp
  engine/graphics/opengl/opengl_windows.cpp
  engine/graphics/opengl/opengl_renderer.cpp
  engine/graphics/opengl/gl_functions.cpp
  engine/filesystem/filesystem.cpp
  engine/filesystem/file_provider.cpp
  engine/assets/assets_manager.cpp
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

add_executable(cw
  engine/engine/main.cpp
)

target_link_libraries(cw PRIVATE cw_engine)

execute_process(
  COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_BINARY_DIR}/compile_commands.json
  ${CMAKE_SOURCE_DIR}/compile_commands.json
)

if (CW_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
