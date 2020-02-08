xdev_add_test(
    NAME test-compile
    FILES test_object.h
)

xdev_add_test(
    NAME test-basic
    FILES test_object.h
    XMOC
)
