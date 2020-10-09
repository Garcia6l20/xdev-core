#include <catch2/catch.hpp>

#include <test_object.h>
#include <xdev.hpp>
#include <xdev/gsl.hpp>

using namespace xdev;

//const property<int> TestObject::AStaticConstValue = 42;

inline filesystem::path source_dir() { return filesystem::path(__FILE__).parent_path(); }

TEST_CASE("MOCBasicObject.BasicUsage") {
  auto _   = finally([] { REQUIRE(TestObject::InstanceCounter == 0); });
  auto obj = XObjectBase::Create<TestObject>();
  REQUIRE(obj->prop("ADoubleValue") == 1.0);
  REQUIRE_THROWS_AS(obj->prop("ADoubleValue") = "1.0", xvar::ConvertError);
  REQUIRE_THROWS_AS(obj->prop("AReadOnlyValue") = 2.0, XMetaPropertyBase::IllegalAccess);
  //ASSERT_THROW(obj->setProperty("AConstantValue", 2.0), XMetaPropertyBase::IllegalAccess);
  REQUIRE_NOTHROW(obj->prop("ADoubleValue") = 42.0);
  REQUIRE(obj->prop("ADoubleValue") == 42.0);
  REQUIRE(obj->prop("ADoubleValue") != 21.0);
  static_assert(xdev::XObjectPointer<decltype(obj)>, "fu");
  xvar test = obj;
  obj->call("printThisTestObject", obj);
  auto ref  = test.get<XObjectBase::ptr>()->cast<TestObject>();
  auto ref2 = test.get<TestObject>();
  REQUIRE(obj == ref2);
}

TEST_CASE("MOCBasicObject.Events") {
  auto _    = finally([] { REQUIRE(TestObject::InstanceCounter == 0); });
  auto obj1 = XObjectBase::Create<TestObject>();
  auto obj2 = XObjectBase::Create<TestObject>();
  obj1->connect("trigger", obj2, "onTriggered");
  obj1->trigger(42.0);
}

TEST_CASE("MOCBasicObject.Invokables") {
  auto _    = finally([] { REQUIRE(TestObject::InstanceCounter == 0); });
  auto obj1 = XObjectBase::Create<TestObject>();
  auto obj2 = XObjectBase::Create<TestObject>();
  obj2->setObjectName("Can you see me ???");
  obj1->call("printThisTestObject", obj2);
  obj1->call("doIt", 42., 42);
  REQUIRE(obj1->apply("doIt", std::make_tuple(42., 42)).get<bool>());
}
