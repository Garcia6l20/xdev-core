# xdev-core

> XDev framework core libraries

![Build linux](https://github.com/Garcia6l20/xdev-core/workflows/Build%20linux/badge.svg)
[![Conan packages](https://api.bintray.com/packages/6l20garcia/xdev/xdev-core%3A_/images/download.svg)](https://bintray.com/6l20garcia/xdev/xdev-core%3A_/_latestVersion)

The XDev framework aims to ease modern c++ development, providing
reflection, templating, serialization, and much more.

## Building

- [optional]: Add xdev-center conan's repository:
```bash
conan remote add xdev-center https://api.bintray.com/conan/6l20garcia/xdev
```

- Linux:
```bash
mkdir build && cd build
cmake -DXDEV_UNIT_TESTING=ON -DCMAKE_BUILD_TYPE=<BUILD_TYPE> ..
cmake --build .
ctest
```
