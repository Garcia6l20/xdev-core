#pragma once

#include <xdev.hpp>

using namespace xdev;

X(class) TestObject: public XObject<TestObject> {
public:
    XINVOKABLE
    int whatIsTheAnswer() {
        return 42;
    }
};
#include <test-object.xdev.hpp>
