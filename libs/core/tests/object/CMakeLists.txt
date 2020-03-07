xdev_add_test(object
    SOURCES testobject.cpp testobject.h
    EXTRA_ARGS -s
    LIBRARIES XDev::catch2::main
)
