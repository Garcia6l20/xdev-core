#include "testobject.h"

#include <xdev/xdev-xclass.hpp>

TestObjectStaticClass::TestObjectStaticClass():
    XStaticClass("TestObject")
{
    _properties.insert({"strProp", XMetaProperty<string, ReadOnly>("strProp")});
    _properties.insert({"intProp", XMetaProperty<int>("intProp")});
    _properties.insert({"value", XMetaProperty<double>("value")});
    _properties.insert({"id", XMetaProperty<int>("id")});
}

TestObjectStaticClass::~TestObjectStaticClass() {
}

XObjectBase::ptr TestObjectStaticClass::Create() const
{
    auto instance = Make<TestObject>();
    weak_ptr<TestObject> weak_instance = instance;

    // properties
    objectMetadata(instance.get()).properties.insert({"strProp", instance->strProp});
    objectMetadata(instance.get()).properties.insert({"intProp", instance->intProp});
    objectMetadata(instance.get()).properties.insert({"value", instance->value});
    objectMetadata(instance.get()).properties.insert({"id", instance->id});

    // functions
    if (!instance->onTriggered) {
        throw XException("uninitialized function 'onTriggered' in object 'TestObject'");
    }
    objectMetadata(instance.get()).functions.insert({"onTriggered", instance->onTriggered});
    objectMetadata(instance.get()).functions.insert({"testTrigger", instance->testTrigger});

    // invokables
    objectMetadata(instance.get()).functions
            .insert({"printTestObject", function<void(XObjectBase::ptr)>([&instance](const XObjectBase::ptr& obj) {
                         instance->printTestObject(obj->cast<TestObject>());
                     })});

    // events
    objectMetadata(instance.get()).events.insert({"trigger", instance->trigger});

    instance->initialize();
    return std::move(instance);
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
    onTriggered = [this](double&&value) {
        cout << __PRETTY_FUNCTION__ << " " << objectName() << " got " << value << endl;
        this->value = value;
        if (destroyed)
            throw runtime_error("object has been destroyed");
        *forceCrashIfFreed = value;
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
