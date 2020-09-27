//#include <catch2/catch.hpp>

#include <string_view>
#include <xdev/tuple-tools.hpp>

#include <ranges>

#include <ctre.hpp>

#include <xdev/ct/string.hpp>

using namespace std::literals;

namespace xdev {

  namespace rng = std::ranges;
  namespace views = std::views;

  namespace tokens {
    struct base_token {
      constexpr base_token(size_t start, size_t end) noexcept
          : start{start}
          , end{end} {}
      size_t start = 0;
      size_t end = 0;
    };
    struct control : base_token {
      using base_token::base_token;
      static constexpr std::string_view start_tok = "{%";
      static constexpr std::string_view end_tok = "%}";
    };

    struct render : base_token {
      using base_token::base_token;
      static constexpr std::string_view start_tok = "{{";
      static constexpr std::string_view end_tok = "}}";
    };

    struct comment : base_token {
      using base_token::base_token;
      static constexpr std::string_view start_tok = "{#";
      static constexpr std::string_view end_tok = "#}";
    };

    using tokens_t = std::tuple<control, render, comment>;
  }// namespace tokens

  template <ct::string input, size_t pos>
  constexpr auto match_start() {
    if constexpr (pos < input.size() - 1) {
      return tt::foreach<tokens::tokens_t>([]<typename ElemT>() {
        if constexpr (input[pos] == ElemT::start_tok[0] and
                      input[pos + 1] == ElemT::start_tok[1]) {
          return pos;
        }
      });
    }
  }
  template <ct::string input, size_t pos, size_t start>
  constexpr auto match_end() {
    //    if constexpr (pos < input.size() - 1) {
    return tt::foreach<tokens::tokens_t>([]<typename ElemT>() {
      if constexpr (input[pos] == ElemT::end_tok[0] and
                    input[pos + 1] == ElemT::end_tok[1]) {
        return ElemT{start, pos + 1};
      }
    });
    //    }
  }

  template <ct::string input, size_t pos>
  constexpr auto next_start() {
    if constexpr (pos < input.size()) {
      auto match = [] { return match_start<input, pos>(); };
      if constexpr (std::is_void_v<decltype(match())>) {
        return next_start<input, pos + 1>();
      } else {
        // matched !
        return match();
      }
    }
  }

  template <ct::string input, size_t pos, size_t start>
  constexpr auto next_end() {
    if constexpr (pos < input.size()) {
      auto match = [] { return match_end<input, pos, start>(); };
      if constexpr (std::is_void_v<decltype(match())>) {
        return next_end<input, pos + 1, start>();
      } else {
        // matched !
        return match();
      }
    }
  }


  template <ct::string input, size_t pos>
  constexpr auto next_token() {
    if constexpr (pos < input.size() - 2) {
      auto start_pos = [] { return next_start<input, pos>(); };
      if constexpr (not std::is_void_v<decltype(start_pos())>) {
        // we have a token to find !
        constexpr auto start = next_start<input, pos>();
        auto get_next_end = [] { return next_end<input, start + 1, start>(); };
        static_assert(not std::is_void_v<decltype(get_next_end())>, "unterminated token");
        auto end = get_next_end();
        return end;
      }
    }
  }

  template <ct::string input, size_t pos = 0>
  constexpr auto parse_tokens_impl(tt::is_tuple auto &&output) {
    if constexpr (pos < input.size() - 1) {
      auto next = [] {
        return next_token<input, pos>();
      };
      using NextT = decltype(next());
      if constexpr (std::is_void_v<NextT>) {
        return xfwd(output);
      } else {
        // token available here
        constexpr auto tok = next();
        return parse_tokens_impl<input, tok.end>(tt::push_back(xfwd(output), tok));
      }
    } else {
      return xfwd(output);
    }
  }

  template <ct::string input>
  constexpr auto parse_tokens() {
    return parse_tokens_impl<input>(std::tuple<>{});
  }

    namespace blocks {
        struct for_control {
            template <ct::string input>
            static constexpr auto try_load(tokens::control const& tok) {

            }
        };
        struct endfor_control {
            template <ct::string input>
            static constexpr auto try_load(tokens::control const& tok) {

            }
        };

        struct render {
            template <size_t N>
            constexpr render(const ct::string<N> &input, const tokens::render &tok) noexcept
                    : begin{input.begin() + tok.start + 2}
                    , end{input.begin() + tok.end - 2} {}

            constexpr std::string_view body() const noexcept {
                return {begin, end};
            }
            const char *begin;
            const char *end;
        };
        struct text {};
    }// namespace blocks

  template <ct::string input, size_t index = 0>
  constexpr auto generate_blocks_impl(tt::is_tuple auto const&toks,
                                      tt::is_tuple auto &&output) {
      if constexpr (index < tt::size<std::decay_t<decltype(toks)>> - 1) {
          auto const& tok = tt::at<index>(toks);
          using TokenT = decltype(tok);
          if constexpr (decays_to<TokenT, tokens::render>) {
              return generate_blocks_impl<input, index + 1>(toks,
                      tt::push_back(xfwd(output), blocks::render{input, tok}));
          } else if constexpr (decays_to<TokenT, tokens::control>) {
              auto load_for = [&] { return blocks::for_control::try_load<input>(tok); };
              auto load_endfor = [&] { return blocks::endfor_control::try_load<input>(tok); };
              if constexpr (not std::is_void_v<decltype(load_for())>) {

              } else {
                static_assert(always_false<TokenT>::value, "Unhandled block !");
              }
          } else { // comment
            return generate_blocks_impl<input, index + 1>(toks, xfwd(output));
          }
      } else {
          return output;
      }
  }

  template <ct::string input, tt::is_tuple TokenTupleT>
  constexpr auto generate_blocks(TokenTupleT const&toks) {
      return generate_blocks_impl<input>(toks, std::tuple<>{});
  }

  template <ct::string input>
  constexpr auto parse() {
    return generate_blocks<input>(parse_tokens<input>());
  }

  template <ct::string input_>
  struct renderer {
    static constexpr ct::string input = input_;
    static constexpr auto blocks = parse<input>();
  };

  inline namespace literals {

    template <typename CharT, CharT... chars>
    requires std::same_as<CharT, char> constexpr auto operator"" _cts() {
      constexpr char elems[sizeof...(chars)] = {chars...};
      return xdev::ct::string<sizeof...(chars)>{elems};
    }

    template <typename CharT, CharT... chars>
    requires std::same_as<CharT, char> constexpr auto operator"" _ctt() {
      constexpr char input[sizeof...(chars)] = {chars...};
      return renderer<input>{};
    }

  }// namespace literals

}// namespace xdev

using namespace xdev;
using namespace xdev::literals;

int main() {
  std::vector<int> test;
  constexpr auto template_ = R"(
  {% for value in input %}
  - {{ value }}
  {% endfor %}
  )"_ctt;
  fmt::print("{}", template_.input.body());
  auto tmp = tt::at<0>(template_.blocks);
  fmt::print("{}", tmp.body());


  //  STATIC_REQUIRE(tt::size<decltype(result)> == 3);
  //  auto result = xdev::parse(R"(
  //{% for value in input %}
  //- {{ value }}
  //{% endfor %}
  //)");
  //  REQUIRE(xdev::tt::at<0>(result) == 1);
}

//
//SCENARIO("wft ! twig at compile time !!!", "[xdev-ctt][basic]") {
//
//}
