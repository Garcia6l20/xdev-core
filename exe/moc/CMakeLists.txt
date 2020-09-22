cmake_minimum_required(VERSION 3.7)

project(xdev-moc)
#
# include(FetchContent)
# FetchContent_Declare(cppast
#     GIT_REPOSITORY https://github.com/foonathan/cppast.git
#     GIT_TAG b155d6abccdf97b6940a0543e93354cb05a2ed04
# )
# FetchContent_MakeAvailable(cppast)

xdev_library(${PROJECT_NAME}-lib
    HEADERS
      include/xdev/moc.hpp
    SOURCES
      src/moc.cpp
    LIBRARIES XDev::template CONAN_PKG::lyra CONAN_PKG::fmt CONAN_PKG::ctre
    ALIAS XDev::moc
    RESOURCES_DIRS resources
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
