#include <catch2/catch.hpp>

#include <xdev/xdev.hpp>
#include <iostream>
#include <cassert>
#include <xdev/xdev-xclass.hpp>

#include <spdlog/spdlog.h>
#include <xdev/xdev-variant-fmt.hpp>

#include "testobject.h"

using namespace xdev;
using namespace std;

SCENARIO("object properties shall be access checked", "[object.properties]") {
    GIVEN("An object with an integer property") {
        auto test = XClass::Create<TestObject>("TestObject");
        test->prop("intProp") = 42;
        REQUIRE(test->prop("intProp").is<int>());
        REQUIRE(test->prop("intProp") == 42);
        REQUIRE(*test->prop<int>("intProp") == 42);
        WHEN("the value is changed using variant access") {
            test->prop("intProp") = 43;
            THEN("the value sould be changed") {
                REQUIRE(test->prop("intProp").is<int>());
                REQUIRE(test->prop("intProp") == 43);
                REQUIRE(*test->prop<int>("intProp") == 43);
            }
        }
        WHEN("the value incremented using reference access") {
            (*test->prop<int>("intProp"))++;
            THEN("the value sould be changed") {
                REQUIRE(test->prop("intProp").is<int>());
                REQUIRE(test->prop("intProp") == 43);
                REQUIRE(*test->prop<int>("intProp") == 43);
            }
        }
    }
    GIVEN("An object object with a read-only integer property") {
        auto test = XClass::Create<TestObject>("TestObject");
        auto& p = test->prop("roIntProp");
        REQUIRE(p.is<int>());
        REQUIRE(p == 42);
        WHEN("an illegal access is done") {
            auto illegal_access = [&] {
                p = 43;
            };
            THEN("an exceptions should be raised") {
                REQUIRE_THROWS_AS(illegal_access(), XPropertyBase::IllegalAccess);
            }
        }
    }
}

SCENARIO("object properies should notify listeners", "[object.properties]") {
    GIVEN("An object with an integer property and a bounded listener") {
        auto test = XClass::Create<TestObject>("TestObject");
        test->prop("intProp") = 42;
        REQUIRE(test->prop("intProp").is<int>());
        REQUIRE(test->prop("intProp") == 42);
        REQUIRE(*test->prop<int>("intProp") == 42);
        int result = -1;
        test->prop<int>("intProp").listen([&result](int value) {
            result = value;
        });
        WHEN("the property is changed") {
            test->prop("intProp") = 42;
            THEN("the corresponding listener should be called") {
                REQUIRE(result == 42);
            }
        }
    }
}

SCENARIO("object can be hold by xvar", "[object.variant_compat]") {
    GIVEN("an xvar holding an object") {
        xvar var = XClass::Create<TestObject>("TestObject");
//        REQUIRE(var["intProp"].is<int>());
//        REQUIRE(var["intProp"] == 42);
    }
    GIVEN("an xobj copied to an xvarian holding an object") {
        auto obj = XClass::Create<TestObject>("TestObject");
        static_assert(xdev::XObjectPointer<decltype(obj)>, "xdev::XObjectPointer is wrong");
        static_assert(not xdev::XValueConvertible<decltype(obj)>, "xdev::XValueConvertible is wrong");
        xvar var = obj;
//        REQUIRE(var["intProp"].is<int>());
//        REQUIRE(var["intProp"] == 42);
    }
}
