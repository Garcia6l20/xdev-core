cmake_minimum_required(VERSION 3.14)

project(xdev-core)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
list(APPEND CMAKE_CXX_FLAGS -fconcepts)
list(APPEND CMAKE_CXX_REQUIRED_FLAGS -fconcepts)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

include(conan_check)
include(xdev)

if(MSVC)
    add_compile_options("/std:c++latest")
endif()


add_subdirectory(libs)
add_subdirectory(exe)
# add_subdirectory(examples)
