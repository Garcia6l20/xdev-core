#include <catch2/catch.hpp>
#include <spdlog/spdlog.h>

#include <xdev/xdev-variant.hpp>
#include <xdev/xdev-variant-fmt.hpp>

#include <cstring>
#include <list>
#include <algorithm>

using namespace xdev::variant;
using namespace xdev;

SCENARIO("dict types are accessible as normal dicts", "[core.api.variant.v1.0]") {
    GIVEN("A simple dict") {
        xvar val = XDict{
            {1, "1"},
            {"1", 1},
            {"3", "tree"},
        };
        WHEN("elements are accesseed") {
            auto v1 = val["1"];
            auto v1_ = val[1];
            auto v3 = val["3"];
            THEN("the resluts should be consistant") {
                REQUIRE(v1 == 1);
                REQUIRE(v1_ == "1");
                REQUIRE(v3 == "tree");
            }
        }
        WHEN("a new element is added") {
            val["new"] = true;
            THEN("the new element exists") {
                REQUIRE(val["new"] == true);
            }
            AND_THEN("other elements are still present") {
                REQUIRE(val["1"] == 1);
                REQUIRE(val[1] == "1");
                REQUIRE(val["3"] == "tree");
            }
        }
        WHEN("the dict is updated") {
            val.update({
                {"question", "what is the answer to life the universe and everything"},
                {"response", 42},
            });
            THEN("the new elements exists") {
                REQUIRE(val["question"] == "what is the answer to life the universe and everything");
                REQUIRE(val["response"] == 42);
            }
            AND_THEN("other elements are still present") {
                REQUIRE(val["1"] == 1);
                REQUIRE(val[1] == "1");
                REQUIRE(val["3"] == "tree");
            }
        }
    }
}

SCENARIO("list types are accessible as normal lists", "[core.api.variant.v1.0]") {
    GIVEN("A simple list") {
        xvar val = XList{1, 2, "3"};
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
