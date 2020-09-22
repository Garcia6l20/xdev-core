#include <catch2/catch.hpp>

#include <xdev.hpp>
#include <xdev/template.hpp>

using namespace xdev;

static xvar g_context = R"({
    "who": "__world__",
    "dict": {
        "one": 1,
        "two": 2
    },
    "int_array": [1,2,3],
    "array_array": [
        { "a": [ [1,2,3], [1,2,3] ] },
        { "a": [ [], [] ] },
        { "a": [ ] }
    ]
})"_xjson;


TEST_CASE("TemplateTest.Basic") {
    auto temp = "Hello {{ who | replace('_', '#') | upper }} !!"_xtemplate;
    REQUIRE(temp.render(g_context) == "Hello ##WORLD## !!");

    REQUIRE("Hello {{ who | replace('_', '#') | upper }} !!"_xtemplate.render(g_context)
              == "Hello ##WORLD## !!");
    REQUIRE("Hello {{ who | replace('_','') | upper }} !!"_xtemplate.render(g_context)
              == "Hello WORLD !!");

    REQUIRE_THROWS_AS("Hello {{ nobody | replace('_','') | upper }} !!"_xtemplate.render(g_context), temp::Error);
}

TEST_CASE("TemplateTest.Loops") {
    auto res = xtemplate::Compile("{% for v in int_array %}{{v}}!{% endfor %}{% for v in int_array %}{{v}}!{% endfor %}")->render(g_context);
    REQUIRE(res == "1.000000!2.000000!3.000000!1.000000!2.000000!3.000000!");
    res = xtemplate::Compile("{% for elem in array_array %}_{% for i in elem.a %}-{% for u in i %}+{{ u }}+{% endfor %}-{% endfor %}_{% endfor %}")->render(g_context);
    REQUIRE(res == "_-+1.000000++2.000000++3.000000+--+1.000000++2.000000++3.000000+-__----___");

    // enfor missing
    REQUIRE_THROWS_AS(xtemplate::Compile("{% for elem in a %}_{% for i in elem.a %}-{% for u in i %}+{{ u }}+{% endfor %}-_{% endfor %}"), temp::Error);
}

TEST_CASE("TemplateTest.Conditions") {
    xtemplate::ptr if_test = xtemplate::Compile("{% if d %}Dict: ok={% if d.ok %}OK{% else %}KO{% endif %}{% else %}No dict{% endif %}");
    auto res = if_test->render(
        xdict{});
    REQUIRE(res == "No dict");
    res = if_test->process(
        xdict{ { "d", xdict{
        } } });
    REQUIRE(res == "Dict: ok=KO");
    res = if_test->render(
        xdict{ { "d", xdict{
            { "ok", false }
        } } });
    REQUIRE(res == "Dict: ok=KO");
    res = if_test->render(
        xdict{ { "d", xdict{
            { "ok", true }
        } } });
    REQUIRE(res == "Dict: ok=OK");
}

TEST_CASE("TemplateTest.Extending") {
    XResources::ptr test_res = XResources::Make();
    test_res->add("templates/compiled/main",
        xtemplate::Compile("{% block content1 %}parent content1 {{who}}{% endblock %} {% block content2 %}parent content2 {{who}}{% endblock %}"));
    auto res = xtemplate::Compile("{% extends templates/compiled/main %}{% block content1 %}who:{{who}}{% endblock %}", test_res)->render(g_context);
    REQUIRE(res == "who:__world__ parent content2 __world__");
}
