cmake_minimum_required(VERSION 3.7)

project(xdev-rc)

xdev_library(${PROJECT_NAME}-lib
    HEADERS include/xdev/basic-rc.hpp
    SOURCES src/basic-rc.cpp
    LIBRARIES XDev::core CONAN_PKG::lyra CONAN_PKG::fmt
    ALIAS XDev::rc
    FOLDER xdev
)

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PUBLIC ${PROJECT_NAME}-lib)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER xdev)

add_subdirectory(tests)

install(TARGETS ${PROJECT_NAME}
    RUNTIME
      DESTINATION bin
      COMPONENT Development
)
