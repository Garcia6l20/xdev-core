#include <xdev.hpp>
#include <catch2/catch.hpp>
#include <iostream>

using namespace xdev;
using namespace std;

TEST_CASE("VariantTest.StrValue") {
  xvar val = "hello";
  REQUIRE(val.typeName() == "std::string");
//  REQUIRE(val.is<bool>());
//  REQUIRE(val == true);
//  REQUIRE_FALSE(val == false);
//  REQUIRE_FALSE(val != true);
//  REQUIRE(val.toString() == "true");
}

TEST_CASE("VariantTest.BoolValue") {
    xvar val = true;
    REQUIRE(val.typeName() == "bool");
    REQUIRE(val.is<bool>());
    REQUIRE(val == true);
    REQUIRE_FALSE(val == false);
    REQUIRE_FALSE(val != true);
    REQUIRE(val.toString() == "true");
}

TEST_CASE("VariantTest.IntValue") {
    xvar val = 42;
    REQUIRE(val.typeName() == "int");
    REQUIRE(val.is<int>());
    REQUIRE(val == 42);
    REQUIRE_FALSE(val != 42);
    REQUIRE(val != 43);
    REQUIRE_FALSE(val == 43);
    REQUIRE(val.toString() == "42");
}

TEST_CASE("VariantTest.DoubleValue") {
    xvar val = 42.0;
    REQUIRE(val.typeName() == "double");
    REQUIRE(val.is<double>());
    REQUIRE_FALSE(val.is<int>());
    REQUIRE_FALSE(val == 42);
    REQUIRE(val == 42.);
    REQUIRE(val != 42);
    REQUIRE_FALSE(val != 42.);
    REQUIRE(val != 43);
    REQUIRE(val != 43.);
    REQUIRE_FALSE(val == 43.);
}

TEST_CASE("VariantTest.Array") {
    xvar dumb = xlist{4, "2"};
    REQUIRE(dumb.typeName() == "xlist");
    REQUIRE(dumb.get<xlist>().size() == 2);
    xvar var = std::move(dumb);
    REQUIRE(dumb.is<xnone>());
    REQUIRE(dumb.typeName() == "xdev::variant::None");
    xlist &array = var.get<xlist>();
    REQUIRE(array.size() == 2);
    REQUIRE(array[0] == 4);
    REQUIRE(array[1] == "2");
    array.push("this", "is", "the", "response");
    REQUIRE(array[3] == "is");
    array.pop(2);
    array.push<xlist::Front>("42", ",");
    array.push<xlist::Front>("What", "is", "the", "answer ?");
    for(auto&&item: array) {
        REQUIRE(item.is<string>());
    }
    REQUIRE(array[4] == "42");
}

TEST_CASE("VariantTest.Dict") {
    xdict dict {{"4", 4}, {"2", 2.0}};
    dict["2"] = "two";
    string key = "2";
    xvar tmp = 55;
    dict[key] = tmp;
    tmp = dict[key];
    REQUIRE(tmp == dict[key]);
    dict["what ?"] = "test";
    REQUIRE(dict["what ?"] == "test");
}

TEST_CASE("VariantTest.IterationAssignment") {
    xvar iterable = xlist{1.0, 2.0, 3.0};
    xdict context = xdict{{"test", 0}};
    xdict for_context = context;
    string k = "key";
    for (auto&&item: iterable.get<xlist>()) {
        for_context[k] = item;
        REQUIRE(for_context[k] == item);
        REQUIRE_FALSE(context[k] == item);
    }
    REQUIRE(for_context[k] == iterable.get<xlist>().back());
    REQUIRE_FALSE(context[k] == iterable.get<xlist>().back());
}
