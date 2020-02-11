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

list(APPEND CMAKE_MODULE_PATH ${CONAN_XDEV-CORE_ROOT}/cmake)
include(xdev)

add_executable(${PROJECT_NAME}
    test-object.hpp
    main.cpp
)
xdev_moc_target(${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} CONAN_PKG::xdev-core)
