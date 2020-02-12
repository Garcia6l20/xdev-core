cmake_minimum_required(VERSION 3.0)

project(xdev-template)

file(GLOB src_files src/xdev-*.cpp)
file(GLOB hdr_files_pub include/xdev-*.hpp)

xdev_library(${PROJECT_NAME} STATIC
    PUBLIC_HEADERS ${hdr_files_pub}
    SOURCES ${src_files}
    PUBLIC_LIBRARIES XDev::core
    CXX_STANDARD 20
    ALIAS XDev::template
    FOLDER xdev
)
