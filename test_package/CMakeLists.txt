cmake_minimum_required(VERSION 3.14)

project(xdev-core-test-package)

if(MSVC)
    add_compile_options("/std:c++latest")
endif()

include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

add_executable(${PROJECT_NAME}
    main.cpp
)
target_link_libraries(${PROJECT_NAME} CONAN_PKG::xdev-core)
