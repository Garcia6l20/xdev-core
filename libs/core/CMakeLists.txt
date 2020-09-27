cmake_minimum_required(VERSION 3.7)

set(header_files
  include/xdev/std-concepts.hpp
  include/xdev.hpp
  include/xdev/app.hpp
  include/xdev/base64.hpp
  include/xdev/concepts.hpp
  include/xdev/concurrent.hpp
  include/xdev/core.hpp
  include/xdev/exception.hpp
  include/xdev/json.hpp
  include/xdev/library.hpp
  include/xdev/object.hpp
  include/xdev/object-pool.hpp
  include/xdev/properties.hpp
  include/xdev/resources.hpp
  include/xdev/tools.hpp
  include/xdev/typetraits.hpp
  include/xdev/variant.hpp
  include/xdev/variant-dict.hpp
  include/xdev/variant-fmt.hpp
  include/xdev/variant-function.hpp
  include/xdev/variant-list.hpp
  include/xdev/variant-value.hpp
  include/xdev/xclass.hpp
  include/xdev/yaml.hpp
  )

set(inline_files
  inline/xdev/json.inl
  inline/xdev/object.inl
  inline/xdev/variant.inl
  inline/xdev/variant-dict.inl
  inline/xdev/variant-function.inl
  inline/xdev/variant-list.inl
  inline/xdev/variant-value.inl
  inline/xdev/yaml.inl
  )

set(source_files
  src/app.cpp
  src/base64.cpp
  src/exception.cpp
  src/library.cpp
  src/object.cpp
  src/variant.cpp
  src/xclass.cpp
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
    # PCH <string> <variant> <vector> <map> <list> <type_traits> <memory> <typeinfo> <typeindex>
)

if(NOT WIN32)
    # target_compile_options(${PROJECT_NAME} PUBLIC -fconcepts)
    target_link_libraries(${PROJECT_NAME} PUBLIC stdc++fs)
  target_compile_options(${PROJECT_NAME} PRIVATE -Wfatal-errors)
endif()

target_compile_definitions(${PROJECT_NAME} PUBLIC "$<IF:$<CONFIG:DEBUG>,XDEV_DEBUG=true,DEV_DEBUG=false>")
