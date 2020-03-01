xdev_library(pyxdev-test-lib SHARED
  SOURCES test-lib.hpp test-lib.cpp
  LIBRARIES XDev::core
  MOC
)

add_test(NAME pyxdev-test-lib
  COMMAND python3.7 ${CMAKE_CURRENT_LIST_DIR}/test_lib.py
  WORKING_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
)
set_tests_properties(pyxdev-test-lib PROPERTIES
  ENVIRONMENT PYTHONPATH=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
)
