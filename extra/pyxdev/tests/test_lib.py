#!/usr/bin/env python3
import pyxdev

lib = pyxdev.load('pyxdev-test-lib')


def main():
    print(lib.name())
    print(lib.path())
    d = pyxdev.Dict({
        "response": 42
    })
    d["hello"] = "world"
    assert d["hello"] == "world"
    assert d["response"] == 42

    obj = pyxdev.new('TestObject')
    obj.dict = d
    obj.dict["the answer is"] = 42
    print(str(obj.dict))
    obj.call_me()
    print(obj.say_my_name("nobody"))

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

    print(obj.dict["hello"])
    assert obj.dict["hello"] == "world"
    obj.dict["the answer"] = 42
    assert obj.dict["the answer"] == 42


if __name__ == '__main__':
    main()
