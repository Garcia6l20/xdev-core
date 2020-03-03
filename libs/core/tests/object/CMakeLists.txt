xdev_add_test(dumb)

xdev_add_test(object CATCH
    SOURCES testobject.cpp testobject.h
    EXTRA_ARGS -s
)
