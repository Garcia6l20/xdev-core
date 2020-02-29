cmake_minimum_required(VERSION 3.7)

file(GLOB_RECURSE header_files include/*.hpp)
file(GLOB_RECURSE source_files src/*.cpp)
file(GLOB_RECURSE inline_files inline/*.inl)

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
FetchContent_GetProperties(ctti-fetch)
if(NOT ctti-fetch_POPULATED)
  FetchContent_Populate(ctti-fetch)
  add_subdirectory(${ctti-fetch_SOURCE_DIR} ${ctti-fetch_BINARY_DIR})
endif()
install(DIRECTORY ${ctti_SOURCE_DIR}/include/ctti DESTINATION include)

xdev_library(${PROJECT_NAME} STATIC
    HEADERS ${header_files} ${inline_files}
    SOURCES ${source_files}
    SYSTEM_INCLUDE_DIRS ${ctti-fetch_SOURCE_DIR}/include
    INCLUDE_DIRS ${dlfcn-win32-fetch_SOURCE_DIR}
    LIBRARIES Threads::Threads dl CONAN_PKG::boost CONAN_PKG::fmt
    DEFINITIONS BOOST_SYSTEM_NO_DEPRECATED
    CXX_STANDARD 20
    ALIAS XDev::core
    FOLDER xdev
)

if(NOT WIN32)
    # target_compile_options(${PROJECT_NAME} PUBLIC -fconcepts)
    target_link_libraries(${PROJECT_NAME} PUBLIC stdc++fs)
endif()
