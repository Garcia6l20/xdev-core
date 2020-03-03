#define CATCH_CONFIG_MAIN // This tells the catch header to generate a main
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

/*
TEST(TestObject, Simple) {
    auto test = XClass::Create<TestObject>("TestObject");
    test->prop("intProp") = 42;
    spdlog::info("intProp: {:f}\n", test->prop("intProp"));
    ASSERT_EQ(test->prop("intProp"), 42);
    ASSERT_TRUE(test->prop("intProp").is<int>());
    auto var = test->prop("intProp").value(); // get a copy
    test->prop<int>("intProp")++;
    ASSERT_EQ(test->prop("intProp"), 43);
    ASSERT_EQ(var, 42);
}

TEST(TestObject, ROProperties) {
    auto test = XClass::Create<TestObject>("TestObject");
    spdlog::info("roIntProp: {:f}\n", test->prop("roIntProp"));
    ASSERT_EQ(test->prop("roIntProp"), 42);
    test->prop("roIntProp") = 42;
    ASSERT_TRUE(test->prop("roIntProp").is<int>());
    auto var = test->prop("roIntProp").value(); // get a copy
    test->prop<int>("roIntProp")++;
    ASSERT_EQ(test->prop("roIntProp"), 43);
    ASSERT_EQ(var, 42);
}

TEST(TestObject, Basic) {
    string name = "TestObject";
    auto test = XClass::Create<TestObject>(name);
    ASSERT_TRUE(test->prop("id").is<int>());
    auto& p = test->prop<int>("id");
    p = 43;
    ASSERT_EQ(test->prop<int>("id"), 43);

    test->prop("id") = 42;
    test->prop("id") = 42.; // OK, doubles are convertible to ints
    ASSERT_THROW(test->prop("id") = "42", XVariant::ConvertError);
    {
        auto test2 = XClass::Create("TestObject");
        test2->bind("id", test);
        ASSERT_TRUE(test->prop("id") == test2->prop("id"));
        test->connect("trigger", test2, "onTriggered");
        test->testTrigger();
        test2->prop("id") = -1;
    }
    test->prop("id") = 1;
    test->testTrigger();
    test->listen("id", [&test] (auto value) {
        ASSERT_EQ(test->prop("id"), value);
        cout << value << endl;
    });
    test->prop("id") = 55;
    auto test2 = XClass::Create<TestObject>(name);
    test->connect("trigger", test2, "onTriggered");
    test->trigger.connect(test->onTriggered);
    test->call("testTrigger");
    test->call("printTestObject", test);

    test->intProp = 2;
    int valtest = *test->intProp;
    ASSERT_EQ(test->intProp, valtest);

}*/
