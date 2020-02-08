xdev_add_test(
    NAME obj-props-test-compile
    FILES ../basic/test-compile.cpp test_object.h
)
xdev_add_test(
    NAME obj-props-test-basic
    FILES test-basic.cpp test_object.h
    XMOC
)
