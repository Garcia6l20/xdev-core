#link_libraries(XDev::core)

melt_library(xdev-test-catch2-main
  SOURCES catch2-main.cpp
  ALIAS XDev::catch2::main
  LIBRARIES CONAN_PKG::catch2
)

xdev_discover_tests()

add_subdirectory(object)
add_subdirectory(variant)

add_subdirectory(api)
