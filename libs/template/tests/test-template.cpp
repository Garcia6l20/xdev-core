#include <xdev/xdev-template.hpp>
#include <gtest/gtest.h>

using namespace xdev;

static XDict g_context = XVariant::FromJSON(R"({
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
})").get<XDict>();


TEST(TemplateTest, Basic) {
    ASSERT_EQ(XTemplate::Compile("Hello {{ who | replace('_', '#') | upper }} !!")->process(g_context),
              "Hello ##WORLD## !!");
    ASSERT_EQ(XTemplate::Compile("Hello {{ who | replace('_','') | upper }} !!")->process(g_context),
              "Hello WORLD !!");

    ASSERT_THROW(XTemplate::Compile("Hello {{ nobody | replace('_','') | upper }} !!")->process(g_context), temp::Error);
}

TEST(TemplateTest, Loops) {
    auto res = XTemplate::Compile("{% for v in int_array %}{{v}}!{% endfor %}{% for v in int_array %}{{v}}!{% endfor %}")->process(g_context);
    ASSERT_EQ(res, "1.000000!2.000000!3.000000!1.000000!2.000000!3.000000!");
    res = XTemplate::Compile("{% for elem in array_array %}_{% for i in elem.a %}-{% for u in i %}+{{ u }}+{% endfor %}-{% endfor %}_{% endfor %}")->process(g_context);
    ASSERT_EQ(res, "_-+1.000000++2.000000++3.000000+--+1.000000++2.000000++3.000000+-__----___");

    // enfor missing
    ASSERT_THROW(XTemplate::Compile("{% for elem in a %}_{% for i in elem.a %}-{% for u in i %}+{{ u }}+{% endfor %}-_{% endfor %}"), temp::Error);
}

TEST(TemplateTest, Conditions) {
    XTemplate::ptr if_test = XTemplate::Compile("{% if d %}Dict: ok={% if d.ok %}OK{% else %}KO{% endif %}{% else %}No dict{% endif %}");
    auto res = if_test->process(
        XDict{});
    ASSERT_EQ(res, "No dict");
    res = if_test->process(
        XDict{ { "d", XDict{
        } } });
    ASSERT_EQ(res, "Dict: ok=KO");
    res = if_test->process(
        XDict{ { "d", XDict{
            { "ok", false }
        } } });
    ASSERT_EQ(res, "Dict: ok=KO");
    res = if_test->process(
        XDict{ { "d", XDict{
            { "ok", true }
        } } });
    ASSERT_EQ(res, "Dict: ok=OK");
}

TEST(TemplateTest, Extending) {
    XResources::ptr test_res = XResources::Make();
    test_res->add("templates/compiled/main",
        XTemplate::Compile("{% block content1 %}parent content1 {{who}}{% endblock %} {% block content2 %}parent content2 {{who}}{% endblock %}"));
    auto res = XTemplate::Compile("{% extends templates/compiled/main %}{% block content1 %}who:{{who}}{% endblock %}", test_res)->process(g_context);
    ASSERT_EQ(res, "who:__world__ parent content2 __world__");
}
