#include <xdev.hpp>
#include <gtest/gtest.h>
#include <iostream>

#include <concepts>

using namespace xdev;
using namespace std;

namespace priv = xdev::variant::priv;

template <class T, template <class...> class Template>
struct is_specialization : std::false_type {};

template <template <class...> class Template, class... Args>
struct is_specialization<Template<Args...>, Template> : std::true_type {};

TEST(VariantFunctionsTest, Nominal) {
    XVariant func = XFunction{[](const XArray& args) {
        for (auto arg: args) {
            cout << arg << endl;
        }
        return args;
    }};

    auto are = "are";
    cout << func.get<XFunction>()("hello", "how", are, "you ?", 42) << endl;
}

TEST(VariantFunctionsTest, Lambda) {

    auto lambda = [](double d, double i) {
        return d + i;
    };

    XVariant func = XFunction(lambda);

    auto are = "are";
    ASSERT_NEAR(func.get<XFunction>()(41.8, 0.2).get<double>(), 42., 0.001);
}
