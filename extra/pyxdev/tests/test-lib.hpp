#pragma once

#include <xdev/xdev.hpp>

using namespace xdev;

X(class) TestObject: public xdev::XObject<TestObject>
{
public:
    virtual void initialize() override
    {
        ADoubleValue = 1.0;
        *AStringValue = "Original";
        ObjectNum = ++*InstanceCounter;
        std::cout << "TestObject " << *ObjectNum << " created\n";
    }

    virtual void destroy() override
    {
        std::cout << "TestObject " << *ObjectNum << " destroyed\n";
        --*InstanceCounter;
    }
    property<double, ReadOnly>         AReadOnlyValue;
    property<double>                   ADoubleValue;
    property<string>                   AStringValue;
    property<int>                      ObjectNum;
    static property<int>               InstanceCounter;
};
#include <test-lib.xdev.hpp>
