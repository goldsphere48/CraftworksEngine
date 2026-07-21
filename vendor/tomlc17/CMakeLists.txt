add_library(tomlc17 STATIC
  tomlc17.c
)

target_include_directories(tomlc17 PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

if (MSVC)
  target_compile_definitions(tomlc17 PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()
