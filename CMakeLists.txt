cmake_minimum_required(VERSION 3.14)

project(xdev-core)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
list(APPEND CMAKE_CXX_FLAGS -fconcepts)
list(APPEND CMAKE_CXX_REQUIRED_FLAGS -fconcepts)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

option(CONAN_CONFIG "Use this option to generate conan_profile" OFF)
include(conan_check)
if(CONAN_CONFIG)
    return()
endif()

include(xdev)

if(MSVC)
    add_compile_options("/std:c++latest")
endif()


add_subdirectory(libs)
add_subdirectory(exe)

install(FILES cmake/xdev.cmake DESTINATION cmake)
