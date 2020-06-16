xdev_add_test(compile
    SOURCES test_object.h
)

xdev_add_test(basic
    SOURCES test_object.h
    MOC
)

add_executable(${PROJECT_NAME}-benchmark-compile
        bench-compile.cpp)
target_link_libraries(${PROJECT_NAME}-benchmark-compile PRIVATE XDev::moc CONAN_PKG::benchmark)
