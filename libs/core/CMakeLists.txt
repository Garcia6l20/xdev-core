cmake_minimum_required(VERSION 3.7)

set(header_files
        include/xdev/std-concepts.hpp
        include/xdev/xdev.hpp
        include/xdev/xdev-app.hpp
        include/xdev/xdev-base64.hpp
        include/xdev/xdev-concepts.hpp
        include/xdev/xdev-concurrent.hpp
        include/xdev/xdev-core.hpp
        include/xdev/xdev-exception.hpp
        include/xdev/xdev-json.hpp
        include/xdev/xdev-library.hpp
        include/xdev/xdev-object.hpp
        include/xdev/xdev-object-pool.hpp
        include/xdev/xdev-properties.hpp
        include/xdev/xdev-resources.hpp
        include/xdev/xdev-tools.hpp
        include/xdev/xdev-typetraits.hpp
        include/xdev/xdev-variant.hpp
        include/xdev/xdev-variant-dict.hpp
        include/xdev/xdev-variant-fmt.hpp
        include/xdev/xdev-variant-function.hpp
        include/xdev/xdev-variant-list.hpp
        include/xdev/xdev-variant-value.hpp
        include/xdev/xdev-xclass.hpp
        include/xdev/xdev-yaml.hpp
)

set(inline_files
        inline/xdev/xdev-json.inl
        inline/xdev/xdev-object.inl
        inline/xdev/xdev-variant.inl
        inline/xdev/xdev-variant-dict.inl
        inline/xdev/xdev-variant-function.inl
        inline/xdev/xdev-variant-list.inl
        inline/xdev/xdev-variant-value.inl
        inline/xdev/xdev-yaml.inl
)

set(source_files
        src/xdev-app.cpp
        src/xdev-base64.cpp
        src/xdev-exception.cpp
        src/xdev-library.cpp
        src/xdev-object.cpp
        src/xdev-variant.cpp
        src/xdev-xclass.cpp
        )

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

xdev_library(${PROJECT_NAME} SHARED
    HEADERS ${header_files} ${inline_files}
    SOURCES ${source_files}
    SYSTEM_INCLUDE_DIRS ${ctti-fetch_SOURCE_DIR}/include
    INCLUDE_DIRS ${dlfcn-win32-fetch_SOURCE_DIR}
    LIBRARIES Threads::Threads dl CONAN_PKG::boost CONAN_PKG::fmt CONAN_PKG::spdlog
    DEFINITIONS BOOST_SYSTEM_NO_DEPRECATED
    CXX_STANDARD 20
    ALIAS XDev::core
    FOLDER xdev
    PCH <string> <variant> <vector> <map> <list> <type_traits> <memory> <typeinfo> <typeindex>
)

if(NOT WIN32)
    # target_compile_options(${PROJECT_NAME} PUBLIC -fconcepts)
    target_link_libraries(${PROJECT_NAME} PUBLIC stdc++fs)
endif()
