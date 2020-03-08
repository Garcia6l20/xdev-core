#include <catch2/catch.hpp>
#include <xdev/xdev.hpp>

using namespace xdev;

SCENARIO("basic types are accessible as normal types", "[core.api.variant.v1.0]") {
    GIVEN("An basic string value") {
        xvar val = "42";
        WHEN("the value is compared") {
            THEN("the resluts should be consistant") {
                REQUIRE(val == "42");
            }
        }
    }
    GIVEN("An basic integer value") {
        xvar val = 42;
        WHEN("the value is compared") {
            THEN("the resluts should be consistant") {
                REQUIRE(val <= 42);
                REQUIRE(val > 41);
                REQUIRE(val >= 42);
                REQUIRE(val == 42);
                REQUIRE(val < 43);
            }
        }
    }
    GIVEN("An basic bool value") {
        xvar val = true;
        WHEN("the value is compared") {
            THEN("the resluts should be consistant") {
                REQUIRE(val == true);
                REQUIRE(val != false);
            }
        }
        WHEN("the value is negated") {
            val = !val;
            THEN("the resluts should be consistant") {
                REQUIRE(val == false);
                REQUIRE(val != true);
            }
        }
    }

    GIVEN("An integer variant") {
        xvar var = 42;
        WHEN("a new value is assigned") {
            var = 43;
            THEN("the variant is updated with the new value") {
                REQUIRE(var == 43);
            }
        }
        WHEN("the value is less compared") {
            auto is_less_true = var < 100;
            auto is_less_false = var < 0;
            auto is_less_rev_true = 0 < var;
            auto is_less_rev_false = 100 < var;
            THEN("the comparaison results shoul be exact") {
                REQUIRE(is_less_true == true);
                REQUIRE(is_less_false == false);
                REQUIRE(is_less_rev_true == true);
                REQUIRE(is_less_rev_false == false);
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
