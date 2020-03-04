#include <catch2/catch.hpp>

#include <xdev/xdev.hpp>

using namespace xdev;

SCENARIO("basic type are accessible as normal types", "[core.api.variant.v1.0]") {
    GIVEN("An integer variant") {
        XVariant var = 42;
        WHEN("a new value is assigned") {
            var = 43;
            THEN("the variant is updated with the new value") {
                REQUIRE(var == 43);
            }
        }
        WHEN("the value is less compared") {
            auto is_less_true = var < 100;
            auto is_less_false = var < 0;
//            auto is_less_rev_true = 0 < var;
//            auto is_less_rev_false = 100 < var;
            THEN("the comparaison results shoul be exact") {
                REQUIRE(is_less_true == true);
                REQUIRE(is_less_false == false);
//                REQUIRE(is_less_rev_true == true);
//                REQUIRE(is_less_rev_false == false);
            }
        }
        WHEN("it is post-incremented") {
            auto returned = var++;
            THEN("the variant is incremented and the returned value is the previous one") {
                REQUIRE(var == 43);
                REQUIRE(returned == 42);
            }
        }
        WHEN("it is pre-incremented") {
            auto returned = ++var;
            THEN("the variant is incremented and the returned value is the variant it-self") {
                REQUIRE(var == 43);
                REQUIRE(returned == var);
            }
        }
        WHEN("it is post-decremented") {
            auto returned = var--;
            THEN("the variant is decremented and the returned value is the previous one") {
                REQUIRE(var == 41);
                REQUIRE(returned == 42);
            }
        }
        WHEN("it is pre-decremented") {
            auto& returned = --var;
            THEN("the variant is decremented and the returned value is the variant it-self") {
                REQUIRE(var == 41);
                REQUIRE(returned == var);
            }
        }
    }
}
