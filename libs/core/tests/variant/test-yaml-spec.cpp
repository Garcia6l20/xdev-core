#include <catch2/catch.hpp>

#include <xdev/xdev.hpp>
#include <xdev/xdev-yaml.hpp>

SCENARIO("Core tag resolution should be handled", "[YamlSpecs.CoreTagResolution]") {
    auto data = R"(
A null: null
Booleans: [ true, True, false, FALSE ]
Integers: [ 0, 0o7, 0x3A, -19 ]
Floats: [ 0., -0.0, .5, +12e03, -2E+05 ]
Also floats: [ .inf, -.Inf, +.INF, .NAN ]
)"_xyaml;
    REQUIRE(data.is<xdict>());
    REQUIRE(data["A null"].is<xnone>());

    auto booleans = data["Booleans"].get<xlist>();
    for (auto&&value: booleans) {
        REQUIRE(value.is<bool>());
    }
    REQUIRE(booleans[0] == true);
    REQUIRE(booleans[1] == true);
    REQUIRE(booleans[2] == false);
    REQUIRE(booleans[3] == false);

    auto integers = data["Integers"].get<xlist>();
    for (auto&&value: integers) {
        REQUIRE(value.is<int>());
    }
    REQUIRE(integers[0] == 0);
    REQUIRE(integers[1] == 7);
    REQUIRE(integers[2] == 0x3A);
    REQUIRE(integers[3] == -19);

    auto floats = data["Floats"].get<xlist>();
    for (auto&&value: floats) {
        REQUIRE(value.is<double>());
    }
    REQUIRE(floats[0] == 0.);
    REQUIRE(floats[1] == -0.);
    REQUIRE(floats[2] == .5);
    REQUIRE(floats[3] == +12e03);
    REQUIRE(floats[4] == -2E+05);

    auto also_floats = data["Also floats"].get<xlist>();
    for (auto&&value: also_floats) {
        REQUIRE(value.is<double>());
    }
    REQUIRE(also_floats[0] == std::numeric_limits<double>::infinity());
    REQUIRE(also_floats[1] == std::numeric_limits<double>::infinity());
    REQUIRE(also_floats[2] == std::numeric_limits<double>::infinity());
    REQUIRE(std::isnan(also_floats[3].get<double>()));
}
