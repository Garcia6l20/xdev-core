#ifndef TEST_OBJECT_H
#define TEST_OBJECT_H

#include <xdev/xdev.hpp>

#include <iostream>
#include <xdev/xdev-object-pool.hpp>

using namespace xdev;

X(class) TestObject: public xobj<TestObject>
{
public:
    TestObject() {
        *ADoubleValue = 1.0;
        AReadOnlyValue(this) = 42.;
        *AStringValue = "Original";
        ObjectNum = ++*InstanceCounter;
        cout << "TestObject " << *ObjectNum << " created" << endl;
        onTriggered = [](double /*value*/) {
            cout << __PRETTY_FUNCTION__ << " called" << endl;
        };
        /*printThisTestObject = [](const TestObject::ptr& obj) {
            cout << __PRETTY_FUNCTION__ << " called" << endl;
            cout << obj->getProperty("AReadOnlyValue") << endl;
        };*/
    }

    virtual ~TestObject() override
    {
        cout << "TestObject " << *ObjectNum << " destroyed" << endl;
    }

    XMETADATA(TestMeta,         {"hello": "world"})

    property<double, ReadOnly>         AReadOnlyValue{this, 43.};
    property<double>                   ADoubleValue;
    property<string>                   AStringValue;
    property<int>                      ObjectNum;
    static property<int>               InstanceCounter;

    event<double>            trigger;
    function<void(double)>   onTriggered;

    XINVOKABLE
    void printThisTestObject(const TestObject::ptr& obj) {
        cout << *obj << endl;
    }

    XINVOKABLE
    bool doIt(double d, int i) {
        cout << "passed: " << d << " and " << i << endl;
        return true;
    }
};
#include <test_object.xdev.hpp>

#endif // TEST_OBJECT_H
