#pragma once

#include <xdev/xdev.hpp>

using namespace xdev;

X(class) TestObject: public xdev::XObject<TestObject>
{
public:
    virtual void initialize() override
    {
        double_value = 1.0;
        *string_value = "Original";
        object_num = ++*InstanceCounter;
        std::cout << "TestObject " << *object_num << " created\n";
        *dict = {
            {"hello", "world"}
        };
    }

    virtual void destroy() override
    {
        std::cout << "TestObject " << *object_num << " destroyed\n";
    }
    property<double, ReadOnly>          ro_value;
    property<double>                    double_value;
    property<string>                    string_value;
    property<int>                       object_num;
    static property<int>                InstanceCounter;
    property<XObjectBase::ptr>          sub_object;
    property<XDict>                     dict;
    property<XArray>                    array;
};
#include <test-lib.xdev.hpp>
