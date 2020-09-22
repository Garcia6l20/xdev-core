#include <catch2/catch.hpp>
#include <xdev.hpp>

using namespace xdev;

TEST_CASE("VariantJsonTest.HelloWorld") {
    auto data = R"({
        "hello": "world"
    })"_xjson;
    REQUIRE(data.is<xdict>());
    REQUIRE(data["hello"] == "world");
}

TEST_CASE("VariantJsonTest.Bool") {
    auto data = xvar::FromJSON(R"(true)");
    REQUIRE(data.is<bool>());
    REQUIRE(data == true);
    data = xvar::FromJSON(R"(false)");
    REQUIRE(data.is<bool>());
    REQUIRE(data == false);
}

TEST_CASE("VariantJsonTest.Double") {
    auto data = xvar::FromJSON(R"(42.0)");
    REQUIRE(data.is<double>());
    REQUIRE(data == 42.);
}

TEST_CASE("VariantJsonTest.String") {
    auto data = xvar::FromJSON(R"("testing")");
    REQUIRE(data.is<std::string>());
    REQUIRE(data == "testing");
}

TEST_CASE("VariantJsonTest.Array") {
    auto array = xvar::FromJSON(R"([false, 2, "3", 4.0])").get<xlist>();
    REQUIRE(array[0] == false);
    REQUIRE(array[1] == 2.0); // !!! int -> double
    REQUIRE(array[2] == "3");
    REQUIRE(array[3] == 4.0);
}

TEST_CASE("VariantJsonTest.InnerArray") {
    auto data = xvar::FromJSON(R"({
       "array": [false, 2, "3", 4.0]
    })");
    auto& array = data["array"].get<xlist>();
    REQUIRE(array[0] == false);
    REQUIRE(array[1] == 2.0); // !!! int -> double
    REQUIRE(array[2] == "3");
    REQUIRE(array[3] == 4.0);
}


TEST_CASE("VariantJsonTest.DictAccess") {
    auto data = xvar::FromJSON(R"({
       "dict": { "test": { "test": 42 } }
    })");
    REQUIRE(data["dict.test.test"] == 42.);
    REQUIRE(data["dict"]["test"]["test"] == 42.);
}
