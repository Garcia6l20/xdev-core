#include <catch2/catch.hpp>
#include <xdev/xdev.hpp>

using namespace xdev;

SCENARIO("list types are accessible as normal lists", "[core.api.variant.v1.0]") {
  GIVEN("A simple list") {
    xvar val = xlist {1, 2, "3"};
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

SCENARIO("list should accept conversions from other containers", "[core.api.variant.v1.0]") {
  GIVEN("A simple list") {
    std::vector<std::string> list {"hello", "world"};
    xvar val = xlist(list);
    WHEN("elements are accesseed") {
      auto v1 = val[0];
      auto v2 = val[1];
      THEN("the resluts should be consistant") {
        REQUIRE(v1 == "hello");
        REQUIRE(v2 == "world");
      }
    }
  }
}

SCENARIO("list types should be comparable", "[core.api.variant.v1.0]") {
  GIVEN("A simple list") {
    xvar val = xlist {1, 2, "3"};
    WHEN("the list is compared with actual values") {
      auto result = val == xlist {1, 2, "3"};
      THEN("the resluts should be consistant") { REQUIRE(result == true); }
    }
    WHEN("the list is compared with other values") {
      auto result = val == xlist {1, 42, "3"};
      THEN("the resluts should be consistant") { REQUIRE(result == false); }
    }
  }
}

SCENARIO("list types should be iterable", "[core.api.variant.v1.0]") {
  GIVEN("A simple list") {
    xvar val = xlist {1, 2, "3"};
    THEN("a normal iteration can be realized") {
      for (auto &&item : val.get<xlist>()) { fmt::print("- {:f}\n", item); }
    }
  }
}
