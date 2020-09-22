#include "testobject.h"

#include <xdev/xclass.hpp>

TestObjectStaticClass::TestObjectStaticClass():
    XStaticClass("TestObject")
{
    _properties.emplace("strProp", XMetaProperty<string, ReadOnly>("strProp"));
    _properties.emplace("intProp", XMetaProperty<int>("intProp"));
    _properties.emplace("roIntProp", XMetaProperty<int, ReadOnly>("roIntProp"));
    _properties.emplace("value", XMetaProperty<double>("value"));
    _properties.emplace("id", XMetaProperty<int>("id"));
}

TestObjectStaticClass::~TestObjectStaticClass() {
}

XObjectBase::ptr TestObjectStaticClass::Create() const
{
    auto instance = Make<TestObject>();
    weak_ptr<TestObject> weak_instance = instance;

    // properties
    objectMetadata(instance.get()).properties.emplace("strProp", instance->strProp);
    objectMetadata(instance.get()).properties.emplace("intProp", instance->intProp);
    objectMetadata(instance.get()).properties.emplace("roIntProp", instance->roIntProp);
    objectMetadata(instance.get()).properties.emplace("value", instance->value);
    objectMetadata(instance.get()).properties.emplace("id", instance->id);

    // functions
    if (!instance->onTriggered) {
        throw XException("uninitialized function 'onTriggered' in object 'TestObject'");
    }
    objectMetadata(instance.get()).functions.emplace("onTriggered", instance->onTriggered);
    objectMetadata(instance.get()).functions.emplace("testTrigger", instance->testTrigger);

    // invokables
    objectMetadata(instance.get()).functions
            .emplace("printTestObject", function<void(XObjectBase::ptr)>([weak_instance](const XObjectBase::ptr& obj) {
                         weak_instance.lock()->printTestObject(obj->cast<TestObject>());
                     }));

    // events
    objectMetadata(instance.get()).events.emplace("trigger", instance->trigger);

    instance->initialize();
    return instance;
}

TestObjectStaticClass::ptr TestObjectStaticClass::_instance = []() {
    shared_ptr<TestObjectStaticClass> instance(new TestObjectStaticClass(), [](auto* pointer){
        XClass::UnRegister(pointer);
        delete pointer;
    });
    XClass::Register(instance, &TestObject::CreateObject);
    return instance;
}();

TestObject::BaseStaticClass& TestObject::_XStaticClass = *TestObjectStaticClass::_instance;


TestObject::TestObject():
    value(42.),
    id(1),
    forceCrashIfFreed(make_shared<int>(0)),
    destroyed(false)
{
    onTriggered = [this](double&&val) {
        cout << __PRETTY_FUNCTION__ << " " << objectName() << " got " << val << endl;
        this->value = val;
        if (destroyed)
            throw runtime_error("object has been destroyed");
        *forceCrashIfFreed = int(val);
    };
    testTrigger = [this]() {
        *forceCrashIfFreed = 0;
        trigger(*value);
    };
    cout << objectName() << " created" << endl;
}

TestObject::~TestObject() {
    cout << objectName() << " destroyed" << endl;
    destroyed = true;
    // memset(this, 0, sizeof(TestObject));
}

void TestObject::printTestObject(const TestObject::ptr& obj)
{
    cout << __PRETTY_FUNCTION__ << ": object: " << *obj << endl;
}
