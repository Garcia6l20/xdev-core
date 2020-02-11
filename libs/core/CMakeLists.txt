cmake_minimum_required(VERSION 3.7)

file(GLOB_RECURSE header_files include/*.hpp)
file(GLOB_RECURSE source_files src/*.cpp)
file(GLOB_RECURSE inline_files inline/*.inl)

set(Boost_USE_STATIC_LIBS        ON)
set(Boost_USE_MULTITHREADED      ON)

find_package(Boost REQUIRED)

add_definitions(-DBOOST_SYSTEM_NO_DEPRECATED)

include(FetchContent)

project(xdev-core)

find_package(Threads REQUIRED)

if(WIN32)
    FetchContent_Declare(dlfcn-win32-fetch
        GIT_REPOSITORY https://github.com/dlfcn-win32/dlfcn-win32.git
        GIT_TAG 974b39c7a481e2b2cf767090ebd6c104ad16c4b3
    )
    FetchContent_MakeAvailable(dlfcn-win32-fetch)
    list(APPEND _extra_include_dirs ${dlfcn-win32-fetch_SOURCE_DIR})
endif()

FetchContent_Declare(ctti-fetch
    GIT_REPOSITORY https://github.com/Manu343726/ctti.git
    GIT_TAG d7e9828b82ce7a6321465fbd84f9bccb772c7f43
)
FetchContent_MakeAvailable(ctti-fetch)
install(DIRECTORY ${ctti_SOURCE_DIR}/include/ctti DESTINATION include)

xdev_library(${PROJECT_NAME} STATIC
    PUBLIC_HEADERS ${header_files} ${inline_files}
    SOURCES ${source_files}
    PUBLIC_INCLUDE_DIRS ${ctti-fetch_SOURCE_DIR}/include ${dlfcn-win32-fetch_SOURCE_DIR}
    PUBLIC_LIBRARIES Threads::Threads ctti dl Boost::headers CONAN_PKG::fmt
    CXX_STANDARD 20
    ALIAS XDev::core
    FOLDER xdev
)

if(NOT WIN32)
    target_compile_options(${PROJECT_NAME} PUBLIC -fconcepts)
    target_link_libraries(${PROJECT_NAME} PUBLIC stdc++fs)
endif()

include(CheckIncludeFileCXX)

check_include_file_cxx(concepts XDEV_HAS_CONCEPTS_HEADER)
if(XDEV_HAS_CONCEPTS_HEADER)
    message(FATAL_ERROR "concepts library seems implemented... consider removing std-concepts")
    target_compile_definitions(${PROJECT_NAME} PUBLIC XDEV_HAS_CONCEPTS_HEADER=1)
endif()
