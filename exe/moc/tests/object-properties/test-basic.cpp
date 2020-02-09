#include <xdev/xdev.hpp>
#include <test_object.h>
#include <gtest/gtest.h>

using namespace xdev;

int TestObject::InstanceCounter = 0;

inline filesystem::path source_dir() {
    return filesystem::path(__FILE__).parent_path();
}

TEST(MOCBasicObjectProperty, BasicUsage) {
    auto obj = XObjectBase::Create<TestObject>();
    ASSERT_EQ(obj->getProperty("ADoubleValue"), 1.0);
    ASSERT_THROW(obj->setProperty("ADoubleValue", "1.0"), XVariant::bad_convert);
    ASSERT_NO_THROW(obj->setProperty("ADoubleValue", 42.0));
    ASSERT_EQ(obj->getProperty("ADoubleValue"), 42.0);
    ASSERT_NE(obj->getProperty("ADoubleValue"), 21.0);
}

TEST(MOCBasicObjectProperty, ObjectProperty) {
    ASSERT_EQ(TestObject::InstanceCounter, 0);
    auto obj = XObjectBase::Create<TestObject>();
    auto sub = XObjectBase::Create<TestObject>();
    obj->setProperty("SubObject", sub);
    ASSERT_EQ(obj->getProperty("SubObject"), sub);
    ASSERT_TRUE(XObjectBase::is_xobject<TestObject>);
    ASSERT_EQ(obj->getProperty<TestObject>("SubObject")->getProperty<double>("ADoubleValue"), 1.0);
    ASSERT_THROW(obj->getProperty<TestObject>("SubObject")->setProperty("ADoubleValue", "1.0"), XVariant::bad_convert);
    ASSERT_NO_THROW(obj->getProperty<TestObject>("SubObject")->setProperty("ADoubleValue", 42.0));
    ASSERT_EQ(obj->getProperty<TestObject>("SubObject")->getProperty("ADoubleValue"), 42.0);
    ASSERT_NE(obj->getProperty<TestObject>("SubObject")->getProperty("ADoubleValue"), 21.0);
}

TEST(MOCBasicObjectProperty, AssertClean) {
    ASSERT_EQ(TestObject::InstanceCounter, 0);
}
