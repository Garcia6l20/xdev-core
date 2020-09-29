/** @file xdev/ctt/tokenizer.hpp
 * @brief Compile-time template tokenizer
 *
 */
#pragma once

#include <xdev/ct.hpp>

namespace xdev::ctt {

  namespace tokens {
    template <auto input, size_t start_pos, size_t end_pos, typename TokenT>
    struct impl {
      /** @brief Actual matched token type
       */
      using token_type              = TokenT;
      static constexpr auto start   = start_pos;
      static constexpr auto end     = end_pos;
      static constexpr auto content = std::string_view{input.begin() + start_pos + 2, input.begin() + end_pos - 1};
    };
    struct control {
      static constexpr std::string_view start_tok = "{%";
      static constexpr std::string_view end_tok   = "%}";
    };

    struct render {
      static constexpr std::string_view start_tok = "{{";
      static constexpr std::string_view end_tok   = "}}";
    };

    struct comment {
      static constexpr std::string_view start_tok = "{#";
      static constexpr std::string_view end_tok   = "#}";
    };

    using tokens_t = ct::tuple<control, render, comment>;
  }// namespace tokens

  template <ct::string input, size_t pos>
  constexpr auto match_start() {
    if constexpr (pos < input.size() - 1) {
      return ct::foreach<tokens::tokens_t>([]<typename ElemT>() {
        if constexpr (input[pos] == ElemT::start_tok[0] and input[pos + 1] == ElemT::start_tok[1]) { return pos; }
      });
    }
  }
  template <ct::string input, size_t pos, size_t start>
  constexpr auto match_end() {
    //    if constexpr (pos < input.size() - 1) {
    return ct::foreach<tokens::tokens_t>([]<typename ElemT>() {
      if constexpr (input[pos] == ElemT::end_tok[0] and input[pos + 1] == ElemT::end_tok[1]) {
        return tokens::impl<input, start, pos + 1, ElemT>{};
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
        constexpr auto start        = next_start<input, pos>();
        auto           get_next_end = [] { return next_end<input, start + 1, start>(); };
        static_assert(not std::is_void_v<decltype(get_next_end())>, "unterminated token");
        auto end = get_next_end();
        return end;
      }
    }
  }

  template <ct::string input, size_t pos = 0, ct::ct_tuple result>
  constexpr auto parse_tokens_impl() {
    if constexpr (pos < input.size() - 1) {
      auto next   = [] { return next_token<input, pos>(); };
      using NextT = decltype(next());
      if constexpr (std::is_void_v<NextT>) {
        return result{};
      } else {
        // token available here
        constexpr auto tok = next();
        return parse_tokens_impl<input, tok.end, ct::push_t<result, decltype(tok)>>();
      }
    } else {
      return result{};
    }
  }

  template <ct::string input>
  constexpr auto parse_tokens() {
    return parse_tokens_impl<input, 0, ct::tuple<>>();
  }

  template <ct::string input>
  constexpr auto parse() {
    return parse_tokens<input>();
  }
}// namespace xdev::ctt
