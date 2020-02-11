#include <xdev/xdev.hpp>
#include <gtest/gtest.h>
#include <iostream>

using namespace xdev;
using namespace std;

TEST(VariantTest, BoolValue) {
    XVariant val = true;
    ASSERT_EQ(val.typeName(), "bool");
    ASSERT_TRUE(val.is<bool>());
    ASSERT_TRUE(val == true);
    ASSERT_FALSE(val == false);
    ASSERT_FALSE(val != true);
    ASSERT_EQ(val.toString(), "true");
}

TEST(VariantTest, IntValue) {
    XVariant val = 42;
    ASSERT_EQ(val.typeName(), "int");
    ASSERT_TRUE(val.is<int>());
    ASSERT_TRUE(val == 42);
    ASSERT_FALSE(val != 42);
    ASSERT_TRUE(val != 43);
    ASSERT_FALSE(val == 43);
    ASSERT_EQ(val.toString(), "42");
}

TEST(VariantTest, DoubleValue) {
    XVariant val = 42.0;
    ASSERT_EQ(val.typeName(), "double");
    ASSERT_TRUE(val.is<double>());
    ASSERT_FALSE(val.is<int>());
    ASSERT_FALSE(val == 42);
    ASSERT_EQ(val, 42.);
    ASSERT_TRUE(val != 42);
    ASSERT_FALSE(val != 42.);
    ASSERT_TRUE(val != 43);
    ASSERT_TRUE(val != 43.);
    ASSERT_FALSE(val == 43.);
}

TEST(VariantTest, Array) {
    XVariant dumb = XArray{4, "2"};
    ASSERT_EQ(dumb.typeName(), "XArray");
    ASSERT_EQ(dumb.get<XArray>().size(), 2);
    XVariant var = std::move(dumb);
    ASSERT_TRUE(dumb.is<XNone>());
    ASSERT_EQ(dumb.typeName(), "xdev::variant::None");
    XArray &array = var.get<XArray>();
    ASSERT_EQ(array.size(), 2);
    ASSERT_EQ(array[0], 4);
    ASSERT_EQ(array[1], "2");
    array.push("this", "is", "the", "response");
    ASSERT_EQ(array[3], "is");
    array.pop(2);
    array.push<XArray::Front>("42", ",");
    array.push<XArray::Front>("What", "is", "the", "answer ?");
    for(auto&&item: array) {
        ASSERT_TRUE(item.is<string>());
    }
    ASSERT_EQ(array[4], "42");
}

TEST(VariantTest, Dict) {
    XDict dict {{"4", 4}, {"2", 2.0}};
    dict["2"] = "two";
    string key = "2";
    XVariant tmp = 55;
    dict[key] = tmp;
    tmp = dict[key];
    ASSERT_EQ(tmp, dict[key]);
    dict["what ?"] = "test";
    ASSERT_EQ(dict["what ?"], "test");
}

TEST(VariantTest, IterationAssignment) {
    XVariant iterable = XArray{1.0, 2.0, 3.0};
    XDict context = XDict{{"test", 0}};
    XDict for_context = context;
    string k = "key";
    for (auto&&item: iterable.get<XArray>()) {
        for_context[k] = item;
        ASSERT_EQ(for_context[k], item);
        ASSERT_NE(context[k], item);
    }
    ASSERT_EQ(for_context[k], iterable.get<XArray>().back());
    ASSERT_NE(context[k], iterable.get<XArray>().back());
}
