find_package(openvds CONFIG REQUIRED)
find_package(FFTW3 CONFIG REQUIRED)

add_library(cppcore
  attribute.cpp
  axis.cpp
  axis_type.cpp
  boundingbox.cpp
  cppapi_data.cpp
  cppapi_metadata.cpp
  datahandle.hpp
  datahandle.cpp
  direction.cpp
  metadatahandle.cpp
  regularsurface.cpp
  subcube.cpp
  subvolume.cpp
)

target_include_directories(cppcore
  PUBLIC ${CMAKE_SOURCE_DIR}/internal/core
)

target_link_libraries(cppcore
  PUBLIC openvds::openvds
)

target_link_libraries(cppcore
  PUBLIC FFTW3::fftw3
)

find_package(Boost REQUIRED)
target_include_directories(cppcore
  PRIVATE ${Boost_INCLUDE_DIRS}
)

if(BUILD_CCORE)
  add_library(ccore
    capi.cpp
  )

  target_link_libraries(ccore
    PUBLIC cppcore
  )
endif()
