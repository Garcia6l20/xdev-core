# xdev-core

> XDev framework core libraries

![Build linux](https://github.com/Garcia6l20/xdev-core/workflows/Build%20linux/badge.svg)
[![Conan packages](https://api.bintray.com/packages/6l20garcia/xdev/xdev-core%3A_/images/download.svg)](https://bintray.com/6l20garcia/xdev/xdev-core%3A_/_latestVersion)

The XDev framework aims to ease modern c++ development, providing
reflection, templating, serialization, and much more.

> It uses current implementation of the c++2a standard so it will
compile only with a own built gcc version from trunk branch (waiting for clang concepts library), but gcc-10 is [coming soon](https://gcc.gnu.org/gcc-10/changes.html) !

> It will compile only on linux plateforms
- I gave up with Windows support, too boring...
- MacOS should be fine but I'm not apple friendly too :)

It uses many moderncpp libraries to create an easy to use framework.

## What it does

The main stuff of this library are `XVariant` and `XObject<>` classes.
The `XVariant` class is designed to hold basic types (`XValue` [bool, int, float, ...]) but also dictionaries (`XDict`), lists (`XList`), functions (`XFunction`) and object (`XObject<>` derived types).

The `XVariant` api is intuitive (though you must know what you are handling...), it is quite similare to `std::variant` in an object oriented way.

If your `XVariant` holds a basic type you can access to it in a regular way:
```cpp
XVariant var = 41; // holds an integer
var++;
assert(var == 42);
```

Same for other stuff:
```cpp
XVariant list = {1, 2, 3., "4"}; // yes you can put any thing that can be hold by a XVariant
for (const auto& item: list) {
  fmt::print("{}\n", item);
}
// seamless serialization - yaml also supported
XVariant dict = R"({
  "The question": "What is the answer to the universe and everything ?",
  "The answer": 42
})"_xjson;
for (const auto& [k, v]: dict) {
  fmt::print("{} = {}\n", k, v);
}
XVariant func = [](const XDict& d, const XList& l) {
  fmt::print("The dict is: {}\n", d);
  fmt::print("The list is: {}\n", l);
};
func(dict, list);
```

### Templates

The `xdev-template` library is a twig-like template engine that interacts with `XVariant` objects.

Example is better than precept:
```cpp
XVariant dict = R"({
question: "What is the answer to the universe and everything ?"
answer: 42
list: [1, 2, 3., "4"]
})"_xyaml;
fmt::print("{}\n", XTemplate::Render(R"({
{{question}}
The answer is {{answer}} !
The list contains:
{% for item in list %}
  - {{item}}
{% endfor %}
})"));
), dict);
```

## What is used

core:
- [libfmt](https://github.com/fmtlib/fmt)
- [speedlog](https://github.com/gabime/spdlog)
- [ctti](https://github.com/Manu343726/ctti)
- [boost](https://github.com/boostorg/boost) - I'm planning to remove it, it's used json parsing (I'll parse it by my own [sorry for [nlohmann_json](https://github.com/nlohmann/json) but I dont want to wrap his object types to mines]) and templates expression [will be replaced by [chaiscript](https://github.com/ChaiScript/ChaiScript)]).

testing:
- [catch2](https://github.com/catchorg/Catch2)
- [gtest](https://github.com/google/googletest) - Also planned to remove (I prefer catch2).

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

## Performance

No benchmarks yet but I'll will provide it ASAP.

## Roadmap

- clang support (when c++2a concepts library will be available)
- benchmarks
- coverage
