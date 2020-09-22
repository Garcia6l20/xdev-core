cmake_minimum_required(VERSION 3.0)

project(xdev-template)

file(GLOB src_files src/*.cpp)
file(GLOB hdr_files_pub include/xdev/*.hpp)

melt_library(${PROJECT_NAME} STATIC
    HEADERS ${hdr_files_pub}
    SOURCES ${src_files}
    LIBRARIES XDev::core
    CXX_STANDARD 20
    ALIAS XDev::template
    FOLDER xdev
)
