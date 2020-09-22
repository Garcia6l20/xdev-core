#include <catch2/catch.hpp>

#include <xdev/template-expressions.hpp>

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

TEST_CASE("TemplateTestExpression.Evaluation") {
    double test = temp::expressions::eval("i - 2", xdict{{"i", 1}});
    REQUIRE(test == -1.0);
}

TEST_CASE("TemplateTestExpression.Pipes") {
    REQUIRE(temp::Expression::Load("who|upper")->eval(g_context).get<string>() == "__WORLD__");
    REQUIRE(temp::Expression::Load("who | replace('_', '') | upper | lower")->eval(g_context).get<string>() == "world");
}

TEST_CASE("TemplateTestExpression.Comparaisons") {
    REQUIRE(temp::Expression::Load("dict|length")->eval(g_context).get<int>() == 2);
    REQUIRE(temp::Expression::Load("dict|length == 2")->eval(g_context).get<bool>() == true);
    REQUIRE(temp::Expression::Load("dict|length != 2")->eval(g_context).get<bool>() == false);
    REQUIRE(temp::Expression::Load("dict|length >= 2")->eval(g_context).get<bool>() == true);
    REQUIRE(temp::Expression::Load("dict|length <= 2")->eval(g_context).get<bool>() == true);
    REQUIRE(temp::Expression::Load("dict|length > 2")->eval(g_context).get<bool>() == false);
    REQUIRE(temp::Expression::Load("dict|length < 2")->eval(g_context).get<bool>() == false);
    REQUIRE(temp::Expression::Load("dict|length >= 3")->eval(g_context).get<bool>() == false);
    REQUIRE(temp::Expression::Load("dict|length <= 1")->eval(g_context).get<bool>() == false);
    REQUIRE(temp::Expression::Load("dict|length > 1")->eval(g_context).get<bool>() == true);
    REQUIRE(temp::Expression::Load("dict|length < 3")->eval(g_context).get<bool>() == true);
    REQUIRE(temp::Expression::Load("d.ok == true")->eval(g_context).get<bool>() == true);
}
