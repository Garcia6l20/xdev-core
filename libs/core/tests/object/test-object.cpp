#include <xdev/xdev.hpp>
#include <iostream>
#include <cassert>
#include <xdev/xdev-xclass.hpp>
#include <gtest/gtest.h>

#include "testobject.h"

using namespace xdev;
using namespace std;

TEST(TestObject, Simple) {
    auto test = XClass::Create<TestObject>("TestObject");
    test->prop("intProp") = 42;
    ASSERT_EQ(test->prop("intProp"), 42);
    ASSERT_TRUE(test->prop("intProp").is<int>());
    auto var = test->prop("intProp").value(); // get a copy
    test->prop<int>("intProp")++;
    ASSERT_EQ(test->prop("intProp"), 43);
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

}
