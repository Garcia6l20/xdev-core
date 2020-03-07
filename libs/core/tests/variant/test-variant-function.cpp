#include <catch2/catch.hpp>
#include <xdev/xdev.hpp>

SCENARIO("function types are accessible as normal functions", "[core.api.variant.v1.0]") {
    GIVEN("A simple function") {
        xvar var = xfn{[](int value) {
            return "the response is " + std::to_string(value);
        }};
        WHEN("the function is called") {
            auto result = var(42);
            THEN("the resluts should be consistant") {
                REQUIRE(result == "the response is 42");
            }
        }
    }
}
