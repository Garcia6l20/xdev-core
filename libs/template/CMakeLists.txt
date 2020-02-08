cmake_minimum_required(VERSION 3.0)

project(xdev-template)

set(Boost_USE_STATIC_LIBS        ON)
set(Boost_USE_MULTITHREADED      ON)

find_package(Boost 1.71 REQUIRED)

add_definitions(-DBOOST_SYSTEM_NO_DEPRECATED)

file(GLOB src_files src/xdev-*.cpp)
file(GLOB hdr_files_pub include/xdev-*.hpp)

xdev_library(${PROJECT_NAME} STATIC
    PUBLIC_HEADERS ${hdr_files_pub}
    SOURCES ${src_files}
    PUBLIC_LIBRARIES Boost::headers XDev::core
    CXX_STANDARD 20
    ALIAS XDev::template
    FOLDER xdev
)
