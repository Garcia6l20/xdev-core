link_libraries(XDev::catch2::main)

melt_executable(xvariant-api-v1.0
    SOURCES test-xvariant-api-v1.0.cpp
)
target_link_libraries(xvariant-api-v1.0 CONAN_PKG::catch2 CONAN_PKG::spdlog CONAN_PKG::gsl-lite)
target_include_directories(xvariant-api-v1.0 PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/inline
    ${ctti-fetch_SOURCE_DIR}/include
)

#xdev_add_test(xvariant-api-v1.0 CATCH
#  EXTRA_ARGS -s
#)
