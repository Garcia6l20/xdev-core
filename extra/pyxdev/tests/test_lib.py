#!/usr/bin/env python3
import pyxdev

lib = pyxdev.load('pyxdev-test-lib')


def main():
    print(lib.name())
    print(lib.path())
    obj = pyxdev.new('TestObject')
    print(obj.ADoubleValue)
    assert obj.ADoubleValue == 1.0
    obj.ADoubleValue = 2.0
    print(obj.ADoubleValue)
    assert obj.ADoubleValue == 2.0

    assert obj.AStringValue == "Original"
    print(obj.AStringValue)
    obj.AStringValue = "from python"
    print(obj.AStringValue)
    assert obj.AStringValue == "from python"


if __name__ == '__main__':
    main()
