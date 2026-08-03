include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
  DOWNLOAD_EXTRACT_TIMESTAMP FALSE
  EXCLUDE_FROM_ALL
  SYSTEM
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(engine_tests
  math_utils_test.cpp
  vector_test.cpp
  matrix_test.cpp
  quat_test.cpp
  transforms_test.cpp
)

target_link_libraries(engine_tests PRIVATE
  cw_engine
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(engine_tests)
