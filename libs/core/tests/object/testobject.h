#pragma once

// #include <testobject.xdev.hpp>
#include <xdev/xdev-object.hpp>

using namespace xdev;

class TestObjectStaticClass: public XStaticClass
{
public:
    using ptr = shared_ptr<TestObjectStaticClass>;
    TestObjectStaticClass();
    virtual ~TestObjectStaticClass() override;
    virtual XObjectBase::ptr Create() const override;
private:

    static TestObjectStaticClass::ptr _instance;
    friend class TestObject;
};

X(class) TestObject : public XObjectBase
{
    _XOBJECT_BASE_EXTEND(TestObject, TestObjectStaticClass)

public:
    TestObject();
    virtual ~TestObject() override;

    event<double>                   trigger;

    function<void(double)>          onTriggered;
    function<void(void)>            testTrigger;
    XINVOKABLE void printTestObject(const TestObject::ptr&);

    property<string, ReadOnly>    strProp;
    property<int> intProp;
    property<double> value;
    property<int>    id;

    shared_ptr<int> forceCrashIfFreed;

    bool destroyed;
};
