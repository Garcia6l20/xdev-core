cmake_minimum_required(VERSION 3.14)

project(xdev-core-test-package)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

if(MSVC)
    add_compile_options("/std:c++latest")
else()
    add_compile_options("-fconcepts")
endif()

include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

add_executable(${PROJECT_NAME}
    main.cpp
)
target_link_libraries(${PROJECT_NAME} CONAN_PKG::xdev-core)
