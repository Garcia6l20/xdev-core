link_libraries(XDev::catch2::main)

xdev_add_test(xvariant-api-v1.0 CATCH
  EXTRA_ARGS -s
)
