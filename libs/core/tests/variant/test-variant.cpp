#include <xdev/xdev.hpp>
#include <catch2/catch.hpp>
#include <iostream>

using namespace xdev;
using namespace std;

TEST_CASE("VariantTest.BoolValue") {
    XVariant val = true;
    REQUIRE(val.typeName() == "bool");
    REQUIRE(val.is<bool>());
    REQUIRE(val == true);
    REQUIRE(val == false);
    REQUIRE(val != true);
    REQUIRE(val.toString() == "true");
}

TEST_CASE("VariantTest.IntValue") {
    XVariant val = 42;
    REQUIRE(val.typeName() == "int");
    REQUIRE(val.is<int>());
    REQUIRE(val == 42);
    REQUIRE_FALSE(val != 42);
    REQUIRE(val != 43);
    REQUIRE_FALSE(val == 43);
    REQUIRE(val.toString() == "42");
}

TEST_CASE("VariantTest.DoubleValue") {
    XVariant val = 42.0;
    REQUIRE(val.typeName() == "double");
    REQUIRE(val.is<double>());
    REQUIRE_FALSE(val.is<int>());
    REQUIRE_FALSE(val == 42);
    REQUIRE(val == 42.);
    REQUIRE(val != 42);
    REQUIRE_FALSE(val != 42.);
    REQUIRE(val != 43);
    REQUIRE(val != 43.);
    REQUIRE(val == 43.);
}

TEST_CASE("VariantTest.Array") {
    XVariant dumb = XList{4, "2"};
    REQUIRE(dumb.typeName() == "XList");
    REQUIRE(dumb.get<XList>().size() == 2);
    XVariant var = std::move(dumb);
    REQUIRE(dumb.is<XNone>());
    REQUIRE(dumb.typeName() == "xdev::variant::None");
    XList &array = var.get<XList>();
    REQUIRE(array.size() == 2);
    REQUIRE(array[0] == 4);
    REQUIRE(array[1] == "2");
    array.push("this", "is", "the", "response");
    REQUIRE(array[3] == "is");
    array.pop(2);
    array.push<XList::Front>("42", ",");
    array.push<XList::Front>("What", "is", "the", "answer ?");
    for(auto&&item: array) {
        REQUIRE(item.is<string>());
    }
    REQUIRE(array[4] == "42");
}

TEST_CASE("VariantTest.Dict") {
    XDict dict {{"4", 4}, {"2", 2.0}};
    dict["2"] = "two";
    string key = "2";
    XVariant tmp = 55;
    dict[key] = tmp;
    tmp = dict[key];
    REQUIRE(tmp == dict[key]);
    dict["what ?"] = "test";
    REQUIRE(dict["what ?"] == "test");
}

TEST_CASE("VariantTest.IterationAssignment") {
    XVariant iterable = XList{1.0, 2.0, 3.0};
    XDict context = XDict{{"test", 0}};
    XDict for_context = context;
    string k = "key";
    for (auto&&item: iterable.get<XList>()) {
        for_context[k] = item;
        REQUIRE(for_context[k] == item);
        REQUIRE(context[k] == item);
    }
    REQUIRE(for_context[k] == iterable.get<XList>().back());
    REQUIRE_FALSE(context[k] == iterable.get<XList>().back());
}
