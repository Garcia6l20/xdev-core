cmake_minimum_required(VERSION 3.13)


option(XDEV_USE_BUILDER "Use builder" OFF)
if(XDEV_USE_BUILDER)
    add_subdirectory(docker)
    return()
endif()

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

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)
include(xdev)

conan_requires(
    "boost/1.74.0"
    "fmt/7.0.1"
    "lyra/1.4.0"
    "spdlog/1.7.0"
    "pybind11/2.5.0"
    "ctre/2.8.2"
    "benchmark/1.5.0"
  BUILD outdated
)

conan_package(
  AUTHOR "Garcia Sylvain <garcia.6l20@gmail.com>"
  LICENSE "MIT"
  URL "https://github.com/Garcia6l20/xdev-core"
  TOPICS serialization reflection templating
  DESCRIPTION "The XDev framework aims to ease modern c++ development, providing"
              "reflection, templating, serialization, and much more."
  BUILD_MODULES cmake/xdev.cmake
)

if(MSVC)
    add_compile_options("/std:c++latest")
endif()

add_subdirectory(libs)
add_subdirectory(exe)

install(FILES cmake/xdev.cmake DESTINATION cmake)
