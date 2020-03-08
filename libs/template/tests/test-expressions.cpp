#include <xdev/xdev-template-expressions.hpp>
#include <gtest/gtest.h>

using namespace xdev;

static xdict g_context = xvar::FromJSON(R"({
    "who": "__world__",
    "dict": {
        "one": 1,
        "two": 2
    },
    "d": {
        "ok": true
    }
})").get<xdict>();

TEST(TemplateTestExpression, Evaluation) {
    double test = temp::expressions::eval("i - 2", xdict{{"i", 1}});
    ASSERT_EQ(test, -1.0);
}

TEST(TemplateTestExpression, Pipes) {
    ASSERT_EQ(temp::Expression::Load("who|upper")->eval(g_context).get<string>(), "__WORLD__");
    ASSERT_EQ(temp::Expression::Load("who | replace('_', '') | upper | lower")->eval(g_context).get<string>(), "world");
}

TEST(TemplateTestExpression, Comparaisons) {
    ASSERT_EQ(temp::Expression::Load("dict|length")->eval(g_context).get<int>(), 2);
    ASSERT_EQ(temp::Expression::Load("dict|length == 2")->eval(g_context).get<bool>(), true);
    ASSERT_EQ(temp::Expression::Load("dict|length != 2")->eval(g_context).get<bool>(), false);
    ASSERT_EQ(temp::Expression::Load("dict|length >= 2")->eval(g_context).get<bool>(), true);
    ASSERT_EQ(temp::Expression::Load("dict|length <= 2")->eval(g_context).get<bool>(), true);
    ASSERT_EQ(temp::Expression::Load("dict|length > 2")->eval(g_context).get<bool>(), false);
    ASSERT_EQ(temp::Expression::Load("dict|length < 2")->eval(g_context).get<bool>(), false);
    ASSERT_EQ(temp::Expression::Load("dict|length >= 3")->eval(g_context).get<bool>(), false);
    ASSERT_EQ(temp::Expression::Load("dict|length <= 1")->eval(g_context).get<bool>(), false);
    ASSERT_EQ(temp::Expression::Load("dict|length > 1")->eval(g_context).get<bool>(), true);
    ASSERT_EQ(temp::Expression::Load("dict|length < 3")->eval(g_context).get<bool>(), true);
    ASSERT_EQ(temp::Expression::Load("d.ok == true")->eval(g_context).get<bool>(), true);
}
