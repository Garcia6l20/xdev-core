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
  std::vector<int> test;
  auto template_ = R"(
  head
  {% for value in input %}
  pre value {{ value }} post value
  {% endfor %}
  tail
  )"_ctt;
  //  static_assert(ctre::match<R"(\s?for\s+(\w+)\s+in\s+(\w+)\s?)">("for t in test"));
  //  static_assert(ctre::match<R"(\s?endfor\s?)">(" endfor"));

  //  constexpr auto template_ = R"({% for t in test %}{% endfor %})"_ctt;

  fmt::print("input: \n{}\n", template_.input.view());

  fmt::print("tokens: \n");
  ct::foreach<decltype(template_)::toks_t>([]<typename TokenT>() {
    fmt::print("- {}\n", ctti::nameof<TokenT>().str());
    fmt::print("  - content : '{}'\n", TokenT::content);
  });

  fmt::print("\nblocks: \n");
  ct::foreach<decltype(template_)::blocks_t>([]<typename TokenT>() {
    fmt::print("- {}\n", ctti::nameof<TokenT>().str());
  });

  fmt::print("\nrendering: \n");
  template_(xdict{
    {"input", xlist {1, 2, 3}}});

  fmt::print("\nresult: {}\n", template_(xdict{
                                           {"input", xlist{1,2,3}}
                               }));
}
