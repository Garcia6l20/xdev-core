cmake_minimum_required(VERSION 3.0)

project(pyxdev)

file(GLOB src_files src/xdev-*.cpp)
file(GLOB hdr_files_pub include/xdev-*.hpp)

conan_include_build_modules()
list(APPEND CMAKE_MODULE_PATH ${CONAN_PYBIND11_ROOT}/share/cmake/pybind11)
include(pybind11Tools)

pybind11_add_module(${PROJECT_NAME} SHARED ${hdr_files_pub} ${src_files})
target_link_libraries(${PROJECT_NAME} PUBLIC XDev::core CONAN_PKG::pybind11)
add_library(XDev::python ALIAS ${PROJECT_NAME})
add_subdirectory(tests)
