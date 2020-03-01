#!/usr/bin/env python3
import pyxdev

lib = pyxdev.load('pyxdev-test-lib')


def main():
    print(lib.name())
    print(lib.path())
    obj = pyxdev.new('TestObject')
    print(obj.double_value)
    assert obj.double_value == 1.0
    obj.double_value = 2.0
    print(obj.double_value)
    assert obj.double_value == 2.0

    assert obj.string_value == "Original"
    print(obj.string_value)
    obj.string_value = "from python"
    print(obj.string_value)
    assert obj.string_value == "from python"


if __name__ == '__main__':
    main()
