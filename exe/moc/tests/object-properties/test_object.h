#ifndef TEST_OBJECT_H
#define TEST_OBJECT_H

#include <xdev/xdev.hpp>

#include <iostream>
#include <xdev/xdev-object-pool.h>

using namespace xdev;

X(class) TestObject: public XObject<TestObject>
{
public:
    virtual void initialize() override
    {
        ADoubleValue = 1.0;
        AStringValue = "Original";
        ObjectNum = ++InstanceCounter;
        cout << "TestObject " << ObjectNum << " created" << endl;
    }

    virtual void destroy() override
    {
        cout << "TestObject " << ObjectNum << " destroyed" << endl;
        --InstanceCounter;
    }
    X(double) ADoubleValue;

    X(string) AStringValue;

    X(TestObject::ptr) SubObject;

    X(int) ObjectNum;

    X(static int) InstanceCounter;
};
#include <test_object.xdev.hpp>

#endif // TEST_OBJECT_H
