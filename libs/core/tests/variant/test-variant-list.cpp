#include <catch2/catch.hpp>
#include <xdev/xdev.hpp>

using namespace xdev;

SCENARIO("list types are accessible as normal lists", "[core.api.variant.v1.0]") {
    GIVEN("A simple list") {
        xvar val = xlist{1, 2, "3"};
        WHEN("elements are accesseed") {
            auto v1 = val[0];
            auto v2 = val[1];
            auto v3 = val[2];
            THEN("the resluts should be consistant") {
                REQUIRE(v1 == 1);
                REQUIRE(v2 == 2);
                REQUIRE(v3 == "3");
            }
        }
    }
}

SCENARIO("list types should be comparable", "[core.api.variant.v1.0]") {
    GIVEN("A simple list") {
        xvar val = xlist{1, 2, "3"};
        WHEN("the list is compared with actual values") {
            auto result = val == xlist{1, 2, "3"};
            THEN("the resluts should be consistant") {
                REQUIRE(result == true);
            }
        }
        WHEN("the list is compared with other values") {
            auto result = val == xlist{1, 42, "3"};
            THEN("the resluts should be consistant") {
                REQUIRE(result == false);
            }
        }
    }
}
