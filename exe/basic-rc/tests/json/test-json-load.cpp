#define CATCH_CONFIG_MAIN // This tells the catch header to generate a main

#include <catch2/catch.hpp>

#include <xdev/xdev-resources.hpp>

#include <resources/xdev-rc-test-json-load-resources.hpp>

using namespace xdev;

TEST_CASE("JSonResources", "Loading") {
    auto root = XdevRcTestJsonLoadResources->getJson("test.json");
    REQUIRE(root["the_response"] == 42.0);
}
