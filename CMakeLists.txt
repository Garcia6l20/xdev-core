cmake_minimum_required(VERSION 3.14)

project(xdev-core)

add_custom_target(xdev-core-pkg
  SOURCES conanfile.py
)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# clangd support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND CMAKE_CXX_FLAGS -fconcepts)
list(APPEND CMAKE_CXX_REQUIRED_FLAGS -fconcepts)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

include(xdev)

if(MSVC)
    add_compile_options("/std:c++latest")
endif()


add_subdirectory(libs)
add_subdirectory(exe)

install(FILES cmake/xdev.cmake DESTINATION cmake)
