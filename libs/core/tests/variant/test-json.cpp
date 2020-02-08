#include <xdev.hpp>
#include <gtest/gtest.h>
#include <iostream>

using namespace xdev;
using namespace std;

TEST(VariantJsonTest, HelloWorld) {
    auto data = XVariant::FromJSON(R"({
        "hello": "world"
    })");
    ASSERT_TRUE(data.is<XDict>());
    ASSERT_EQ(data.get<XDict>().at("hello"), "world");
}

TEST(VariantJsonTest, Bool) {
    auto data = XVariant::FromJSON(R"(true)");
    ASSERT_TRUE(data.is<bool>());
    ASSERT_TRUE(data.get<bool>());
    data = XVariant::FromJSON(R"(false)");
    ASSERT_TRUE(data.is<bool>());
    ASSERT_FALSE(data.get<bool>());
}

TEST(VariantJsonTest, Double) {
    auto data = XVariant::FromJSON(R"(42.0)");
    ASSERT_TRUE(data.is<double>());
    ASSERT_EQ(data, 42.);
}

TEST(VariantJsonTest, String) {
    auto data = XVariant::FromJSON(R"("testing")");
    ASSERT_TRUE(data.is<string>());
    ASSERT_EQ(data, "testing");
}

TEST(VariantJsonTest, Array) {
    auto array = XVariant::FromJSON(R"([false, 2, "3", 4.0])").get<XArray>();
    ASSERT_EQ(array[0], false);
    ASSERT_EQ(array[1], 2.0); // !!! int -> double
    ASSERT_EQ(array[2], "3");
    ASSERT_EQ(array[3], 4.0);
}

TEST(VariantJsonTest, InnerArray) {
    auto data = XVariant::FromJSON(R"({
       "array": [false, 2, "3", 4.0]
    })").get<XDict>();
    auto& array = data.at("array").get<XArray>();
    ASSERT_EQ(array[0], false);
    ASSERT_EQ(array[1], 2.0); // !!! int -> double
    ASSERT_EQ(array[2], "3");
    ASSERT_EQ(array[3], 4.0);
}


TEST(VariantJsonTest, DictAccess) {
    auto data = XVariant::FromJSON(R"({
       "dict": { "test": { "test": 42 } }
    })").get<XDict>();
    ASSERT_EQ(data.at("dict.test.test"), 42.);
    ASSERT_EQ(data.at("dict", "test", "test"), 42.);
}
