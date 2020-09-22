#include <catch2/catch.hpp>

#include <xdev.hpp>
#include <xdev/yaml.hpp>

using namespace xdev;

SCENARIO("basic type should be handled correctly") {
    GIVEN("a parsed yaml string") {
      xvar var = R"("42")"_xyaml;
      spdlog::info("tested: {:f}", var);
      THEN("the result should be as expected") {
        REQUIRE(var.is<std::string>() == true);
        REQUIRE(var == "42");
      }
    }
    GIVEN("a parsed yaml integer") {
      xvar var = "42"_xyaml;
      spdlog::info("tested: {:f}", var);
      THEN("the result should be as expected") {
        REQUIRE(var.is<int>() == true);
        REQUIRE(var == 42);
      }
    }
    GIVEN("a parsed yaml double") {
      xvar var = "42.0"_xyaml;
      spdlog::info("tested: {:f}", var);
      THEN("the result should be as expected") {
        REQUIRE(var.is<double>() == true);
        REQUIRE(var == 42.0);
      }
    }
    GIVEN("a parsed yaml simple map") {
      xvar var = R"(
   string: "42"
   int: 42
   double: 42.
  )"_xyaml;
      spdlog::info("tested: {:f}", var);
      THEN("the result should be as expected") {
        REQUIRE(var["string"].is<std::string>());
        REQUIRE(var["string"] == "42");
        REQUIRE(var["int"].is<int>());
        REQUIRE(var["int"] == 42);
        REQUIRE(var["double"].is<double>());
        REQUIRE(var["double"] == 42.);
      }
    }
    GIVEN("a parsed yaml map with 2 levels") {
      xvar var = R"(
   root:
      string: "42"
      double: 42.
  )"_xyaml;
      spdlog::info("tested: {:f}", var);
      THEN("the result should be as expected") {
        REQUIRE(var.is<xdict>());
        REQUIRE(var["root.string"].is<std::string>());
        REQUIRE(var["root"]["string"] == "42");
        REQUIRE(var["root"]["double"].is<double>());
        REQUIRE(var["root"]["double"] == 42.);
      }
    }
    GIVEN("a parsed yaml map with 3 levels") {
      xvar var = R"(
   root:
      sub:
          string: "42"
          double: 42.
      sub2:
          int: 42
          bool: true
          inline_list: ["hello", "world", 42]
          list:
              - hello
              - world
              - 42
  )"_xyaml;
      spdlog::info("tested: {:f}", var);
      THEN("the result should be as expected") {
        REQUIRE(var.is<xdict>());
        REQUIRE(var["root"].is<xdict>());
        REQUIRE(var["root.sub"].is<xdict>());
        REQUIRE(var["root.sub"]
                == xdict {
                  {"string", "42"},
                  {"double", 42.},
                });
        REQUIRE(var["root.sub2"].is<xdict>());
        REQUIRE(var["root.sub2"]["int"].is<int>());
        REQUIRE(var["root.sub2"]["int"] == 42);
        REQUIRE(var["root.sub2"]["bool"].is<bool>());
        REQUIRE(var["root.sub2"]["bool"] == true);
        REQUIRE(var["root.sub2"]["inline_list"].is<xlist>());
        REQUIRE(var["root.sub2"]["inline_list"] == xlist {"hello", "world", 42});
        REQUIRE(var["root.sub2"]["list"].is<xlist>());
        REQUIRE(var["root.sub2"]["list"] == xlist {"hello", "world", 42});
      }
    }
  GIVEN("a yaml lists of objects") {
    xvar var = R"(
- string: "42"
  double: 42.
- string: "43"
  double: 43.
)"_xyaml;
    spdlog::info("tested: {:f}", var);
    THEN("the result should be as expected") {
      REQUIRE(var.is<xlist>());
      REQUIRE(var[0].is<xdict>());
      REQUIRE(var[0]["string"] == "42");
      REQUIRE(var[0]["double"] == 42.);
      REQUIRE(var[1]["string"] == "43");
      REQUIRE(var[1]["double"] == 43.);
    }
  }
}
