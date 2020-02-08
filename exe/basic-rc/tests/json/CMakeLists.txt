xdev_add_test(NAME json-compile-test
    FILES resources/test.json
)
xdev_add_test(NAME json-load-test
    FILES resources/test.json
    XRES_DIRS resources
)
