cmake_minimum_required(VERSION 3.14)

project(xdev-core)

add_custom_target(xdev-core-pkg
  SOURCES conanfile.py
)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 10.0)
  list(APPEND CMAKE_CXX_FLAGS -fconcepts)
  # gcc-9 uses int for basic operator - use gcc-10
  list(APPEND MELT_EXTRA_FLAGS -Wno-error=conversion)
endif()

option(XDEV_MELTINGPOT_DEV "Use MetlingPot as submodule" OFF)
if(XDEV_MELTINGPOT_DEV)
  # as MP developer
  list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake/MeltingPot)
else()
  # as MP user
  if(NOT EXISTS "${CMAKE_BINARY_DIR}/MeltingPot.cmake")
    message(STATUS "Downloading MeltingPot.cmake from https://github.com/Garcia6l20/MeltingPot")
    file(DOWNLOAD "https://raw.githubusercontent.com/Garcia6l20/MeltingPot/v0.1.x/dist/MeltingPot.cmake" "${CMAKE_BINARY_DIR}/MeltingPot.cmake")
  endif()
  include(${CMAKE_BINARY_DIR}/MeltingPot.cmake)
endif()
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)
include(xdev)

conan_requires(
    "boost/1.71.0"
    "fmt/6.1.0"
    "lyra/1.3.0"
  OPTIONS
    "boost:header_only=True"
)

if(MSVC)
    add_compile_options("/std:c++latest")
endif()


add_subdirectory(libs)
add_subdirectory(exe)

install(FILES cmake/xdev.cmake DESTINATION cmake)
