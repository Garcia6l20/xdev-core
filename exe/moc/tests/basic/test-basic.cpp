#include <xdev.hpp>
#include <test_object.h>
#include <gtest/gtest.h>

using namespace xdev;

property<int> TestObject::InstanceCounter = 0;
//const property<int> TestObject::AStaticConstValue = 42;

inline filesystem::path source_dir() {
    return filesystem::path(__FILE__).parent_path();
}


TEST(MOCBasicObject, BasicUsage) {
    auto obj = XObjectBase::Create<TestObject>();
    ASSERT_EQ(obj->getProperty("ADoubleValue"), 1.0);
    ASSERT_THROW(obj->setProperty("ADoubleValue", "1.0"), XVariant::ConvertError);
    ASSERT_THROW(obj->setProperty("AReadOnlyValue", 2.0), XMetaPropertyBase::IllegalAccess);
    //ASSERT_THROW(obj->setProperty("AConstantValue", 2.0), XMetaPropertyBase::IllegalAccess);
    ASSERT_NO_THROW(obj->setProperty("ADoubleValue", 42.0));
    ASSERT_EQ(obj->getProperty("ADoubleValue"), 42.0);
    ASSERT_NE(obj->getProperty("ADoubleValue"), 21.0);
    XVariant test = obj;
    obj->call("printThisTestObject", obj);
    TestObject::ptr ref = test.get<XObjectBase::ptr>()->cast<TestObject>();
    TestObject::ptr ref2 = test.get<TestObject>();
    ASSERT_EQ(obj, ref2);
}

TEST(MOCBasicObject, Events) {
    auto obj1 = XObjectBase::Create<TestObject>();
    auto obj2 = XObjectBase::Create<TestObject>();
    obj1->connect("trigger", obj2, "onTriggered");
    obj1->trigger(42.0);
}

TEST(MOCBasicObject, Invokables) {
    auto obj1 = XObjectBase::Create<TestObject>();
    auto obj2 = XObjectBase::Create<TestObject>();
    obj2->setObjectName("Can you see me ???");
    obj1->call("printThisTestObject", obj2);
    obj1->call("doIt", 42., 42);
    ASSERT_TRUE(obj1->apply("doIt", std::make_tuple(42., 42)).get<bool>());
}
