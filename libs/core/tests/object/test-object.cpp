#include <xdev/xdev.hpp>
#include <iostream>
#include <cassert>
#include <xdev/xdev-xclass.hpp>
#include <gtest/gtest.h>

#include "testobject.h"

using namespace xdev;
using namespace std;

TEST(TestObject, Basic) {
    string name = "TestObject";
    auto test = XClass::Create<TestObject>(name);
    ASSERT_EQ(test->objectName(), name + "#1");
    cout << test << endl;
    test->prop("id") = 42;
    test->prop("id") = 42.; // OK, doubles are convertible to ints
    ASSERT_THROW(test->prop("id") = "42", XVariant::ConvertError);
    {
        auto test2 = XClass::Create("TestObject");
        ASSERT_EQ(test2->objectName(), name + "#2");
        test2->bind("id", test);
        ASSERT_TRUE(test->prop("id") == test2->prop("id"));
        test->connect("trigger", test2, "onTriggered");
        test->testTrigger();
        test2->setProperty("id", -1);
    }
    test->setProperty("id", 1);
    test->testTrigger();
    test->listen("id", [&test] (auto value) {
        ASSERT_EQ(test->prop("id"), value);
        cout << value << endl;
    });
    test->prop("id") = 55;
    auto test2 = XClass::Create<TestObject>(name);
    ASSERT_EQ(test2->objectName(), name + "#3");
    test->connect("trigger", test2, "onTriggered");
    test->trigger.connect(test->onTriggered);
    test->call("testTrigger");
    test->call("printTestObject", test);

    test->intProp = 2;
    int valtest = *test->intProp;
    ASSERT_EQ(test->intProp, valtest);

}
