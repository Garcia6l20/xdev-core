#include <xdev/xdev.hpp>
#include <xdev/xdev-yaml.hpp>
#include <gtest/gtest.h>
#include <iostream>
#include <cmath>

using namespace xdev;
using namespace std;

TEST(VariantYamlSpecTest, CoreTagResolution) {
    auto data = yaml::parse(R"(
A null: null
Booleans: [ true, True, false, FALSE ]
Integers: [ 0, 0o7, 0x3A, -19 ]
Floats: [ 0., -0.0, .5, +12e03, -2E+05 ]
Also floats: [ .inf, -.Inf, +.INF, .NAN ]
)");
    cout << data << endl;
    ASSERT_TRUE(data.is<XDict>());
    auto d = data.get<XDict>();
    ASSERT_TRUE(d["A null"].is<XNone>());

    auto booleans = d["Booleans"].get<XArray>();
    for (auto&&value: booleans) {
        ASSERT_TRUE(value.is<bool>());
        cout << value << endl;
    }
    ASSERT_EQ(booleans[0], true);
    ASSERT_EQ(booleans[1], true);
    ASSERT_EQ(booleans[2], false);
    ASSERT_EQ(booleans[3], false);

    auto integers = d["Integers"].get<XArray>();
    for (auto&&value: integers) {
        ASSERT_TRUE(value.is<int>());
        cout << value << endl;
    }
    ASSERT_EQ(integers[0], 0);
    ASSERT_EQ(integers[1], 7);
    ASSERT_EQ(integers[2], 0x3A);
    ASSERT_EQ(integers[3], -19);

    auto floats = d["Floats"].get<XArray>();
    for (auto&&value: floats) {
        ASSERT_TRUE(value.is<double>());
        cout << value << endl;
    }
    ASSERT_EQ(floats[0], 0.);
    ASSERT_EQ(floats[1], -0.);
    ASSERT_EQ(floats[2], .5);
    ASSERT_EQ(floats[3], +12e03);
    ASSERT_EQ(floats[4], -2E+05);

    auto also_floats = d["Also floats"].get<XArray>();
    for (auto&&value: also_floats) {
        ASSERT_TRUE(value.is<double>());
        cout << value << endl;
    }
    ASSERT_EQ(also_floats[0], numeric_limits<double>::infinity());
    ASSERT_EQ(also_floats[1], numeric_limits<double>::infinity());
    ASSERT_EQ(also_floats[2], numeric_limits<double>::infinity());
    ASSERT_TRUE(std::isnan(also_floats[3].get<double>()));
}
