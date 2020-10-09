#include <catch2/catch.hpp>

#include <string_view>
#include <xdev/tuple-tools.hpp>

#include <xdev/ctt.hpp>
#include <xdev/str-tools.hpp>
#include <xdev/variant.hpp>

using namespace std::literals;

using namespace xdev;
using namespace xdev::ctt::literals;

template <typename TemplateT>
void print_infos(TemplateT const&template_) {

  fmt::print("input: \n{}\n", template_.input.view());

  fmt::print("tokens: \n");
  ct::foreach<typename TemplateT::toks_t>([]<typename TokenT>() {
    fmt::print("- {}\n", ctti::nameof<TokenT>().str());
    fmt::print("  - content : '{}'\n", TokenT::content);
  });

  fmt::print("\nblocks: \n");
  ct::foreach<typename TemplateT::blocks_t>(
    []<typename TokenT>() { fmt::print("- {}\n", ctti::nameof<TokenT>().str()); });
}

SCENARIO("ctt can loop through lists", "[xdev-ctt][list]") {
  spdlog::set_level(spdlog::level::debug);

  auto template_ = R"(head
{%- for value in input -%}
 pre {{ value }} post
{%- endfor -%}
 tail)"_ctt;

  print_infos(template_);

  auto result = template_(xdict{{"input", xlist{1, 2, 3}}});
  fmt::print("\nresult: {}\n", result);
  REQUIRE(result == "head pre 1 post pre 2 post pre 3 post tail");
}

SCENARIO("ctt can if statements", "[xdev-ctt][list]") {
  spdlog::set_level(spdlog::level::debug);

  auto template_ = R"(head
{%- if test -%}
YES
{%- endif -%}
 tail)"_ctt;

  print_infos(template_);

  auto result = template_(xdict{{"test", true}});
  fmt::print("\nresult: {}\n", result);
  REQUIRE(result == "head pre 1 post pre 2 post pre 3 post tail");
}
