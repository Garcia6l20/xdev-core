xdev_add_test(json-compile
    SOURCES
      resources/test.json
)
xdev_add_test(json-load CATCH
    SOURCES
      resources/test.json
    RESOURCES_DIRS resources
    LIBRARIES XDev::rc
)
