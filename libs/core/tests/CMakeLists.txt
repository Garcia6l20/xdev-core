link_libraries(XDev::core)

set(MELT_TESTING_DEFAULT_BACKEND CATCH)

melt_library(xdev-test-catch2-main
  SOURCES catch2-main.cpp
  ALIAS XDev::catch2::main
  LIBRARIES CONAN_PKG::catch2
)
link_libraries(XDev::catch2::main)

xdev_discover_tests()

add_subdirectory(object)
add_subdirectory(variant)
