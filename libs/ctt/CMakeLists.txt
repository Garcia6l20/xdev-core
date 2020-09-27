project(xdev-ctt)

add_library(${PROJECT_NAME} INTERFACE)
target_link_libraries(${PROJECT_NAME} INTERFACE XDev::core CONAN_PKG::ctre)
target_include_directories(${PROJECT_NAME} INTERFACE include inline)
add_library(XDev::ctt ALIAS ${PROJECT_NAME})

add_subdirectory(tests)