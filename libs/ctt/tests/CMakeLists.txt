link_libraries(XDev::ctt) # XDev::catch2::main)

add_executable(xdev-ctt-test-basic test-basic.cpp)
option(STL_DEV "Use local stl headers" OFF)
if(STL_DEV)
  set(GCC_ROOT /home/sylvain/Projects/gcc)
  set(GCC_BUILD_ROOT ${GCC_ROOT}/build/x86_64-pc-linux-gnu)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -nostdinc++ -I${GCC_ROOT}/libstdc++-v3/include/std -I${GCC_ROOT}/libstdc++-v3/libsupc++ -I${GCC_ROOT}/libstdc++-v3/include -I${GCC_BUILD_ROOT}/libstdc++-v3/include -I${GCC_BUILD_ROOT}/libstdc++-v3/include/x86_64-pc-linux-gnu")
endif()

add_executable(xdev-ctt-test-string test-string.cpp)
target_link_libraries(xdev-ctt-test-string XDev::catch2::main)

add_executable(xdev-ctt-test-tuple test-tuple.cpp)

add_executable(xdev-ctt-test-evaluators test-evaluators.cpp)
target_link_libraries(xdev-ctt-test-evaluators XDev::catch2::main)

add_executable(test-constexpr-string test-constexpr-string.cpp)

