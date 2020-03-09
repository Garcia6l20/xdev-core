#include <xdev/xdev.hpp>
#include <test_object.h>
#include <gtest/gtest.h>

using namespace xdev;

//const property<int> TestObject::AStaticConstValue = 42;

inline filesystem::path source_dir() {
    return filesystem::path(__FILE__).parent_path();
}

TEST(MOCBasicObject, BasicUsage) {
    auto _ = tools::finally{[]{
       ASSERT_EQ(TestObject::InstanceCounter, 0);
    }};
    auto obj = XObjectBase::Create<TestObject>();
    ASSERT_EQ(obj->prop("ADoubleValue"), 1.0);
    ASSERT_THROW(obj->prop("ADoubleValue") = "1.0", xvar::ConvertError);
    ASSERT_THROW(obj->prop("AReadOnlyValue") = 2.0, XMetaPropertyBase::IllegalAccess);
    //ASSERT_THROW(obj->setProperty("AConstantValue", 2.0), XMetaPropertyBase::IllegalAccess);
    ASSERT_NO_THROW(obj->prop("ADoubleValue") = 42.0);
    ASSERT_EQ(obj->prop("ADoubleValue"), 42.0);
    ASSERT_NE(obj->prop("ADoubleValue"), 21.0);
    static_assert(xdev::XObjectPointer<decltype(obj)>, "fu");
    xvar test = obj;
    obj->call("printThisTestObject", obj);
    auto ref = test.get<XObjectBase::ptr>()->cast<TestObject>();
    auto ref2 = test.get<TestObject>();
    ASSERT_EQ(obj, ref2);
}

TEST(MOCBasicObject, Events) {
    auto _ = tools::finally{[]{
       ASSERT_EQ(TestObject::InstanceCounter, 0);
    }};
    auto obj1 = XObjectBase::Create<TestObject>();
    auto obj2 = XObjectBase::Create<TestObject>();
    obj1->connect("trigger", obj2, "onTriggered");
    obj1->trigger(42.0);
}

TEST(MOCBasicObject, Invokables) {
    auto _ = tools::finally{[]{
       ASSERT_EQ(TestObject::InstanceCounter, 0);
    }};
    auto obj1 = XObjectBase::Create<TestObject>();
    auto obj2 = XObjectBase::Create<TestObject>();
    obj2->setObjectName("Can you see me ???");
    obj1->call("printThisTestObject", obj2);
    obj1->call("doIt", 42., 42);
    ASSERT_TRUE(obj1->apply("doIt", std::make_tuple(42., 42)).get<bool>());
}
