//#include <catch2/catch.hpp>

#include <string_view>
#include <xdev/tuple-tools.hpp>

#include <ranges>


#include <xdev/ctt.hpp>
#include <xdev/str-tools.hpp>
#include <xdev/variant.hpp>

using namespace std::literals;

using namespace xdev;
using namespace xdev::ctt::literals;

int main() {
  spdlog::set_level(spdlog::level::debug);

  auto template_ = R"(
  head
{% for value in input -%}
  pre value {{ value }} post value
{% endfor -%}
  tail
  )"_ctt;

  fmt::print("input: \n{}\n", template_.input.view());

  fmt::print("tokens: \n");
  ct::foreach<decltype(template_)::toks_t>([]<typename TokenT>() {
    fmt::print("- {}\n", ctti::nameof<TokenT>().str());
    fmt::print("  - content : '{}'\n", TokenT::content);
  });

  fmt::print("\nblocks: \n");
  ct::foreach<decltype(template_)::blocks_t>(
    []<typename TokenT>() { fmt::print("- {}\n", ctti::nameof<TokenT>().str()); });

  fmt::print("\nrendering: \n");
  template_(xdict{{"input", xlist{1, 2, 3}}});

  fmt::print("\nresult: {}\n", template_(xdict{{"input", xlist{1, 2, 3}}}));
}
