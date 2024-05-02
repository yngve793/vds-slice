# obtain gtest as recommended
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.13.0.zip
)
FetchContent_MakeAvailable(googletest)

add_executable(cppcoretests
  cppapi_test.cpp
  datahandle_fence_test.cpp
  datahandle_metadata_test.cpp
  datahandle_slice_test.cpp
  datahandle_test.cpp
  regularsurface_test.cpp
  subvolume_test.cpp
)

target_link_libraries(cppcoretests
  PRIVATE cppcore
  PRIVATE GTest::gtest_main
  PRIVATE GTest::gmock_main
)

configure_file(../../testdata/10_samples/10_samples_default.vds . COPYONLY)
configure_file(../../testdata/10_samples/10_double_value.vds . COPYONLY)

configure_file(../../testdata/cube_intersection/regular_8x3_cube.vds . COPYONLY)
configure_file(../../testdata/cube_intersection/shift_4_8x3_cube.vds . COPYONLY)
configure_file(../../testdata/cube_intersection/big_shift_8_32x3_cube.vds . COPYONLY)

include(GoogleTest)
gtest_discover_tests(cppcoretests)
