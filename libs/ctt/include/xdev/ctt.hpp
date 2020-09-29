/** @file xdev/ctt.hpp
 * @brief Compile-time template
 */
#pragma once

#include <xdev/ctt/blocks.hpp>

namespace xdev::ctt {

  template <ct::string input_>
  struct renderer {
    static constexpr ct::string input = input_;
    using toks_t                      = std::decay_t<decltype(parse_tokens<input>())>;
    using blocks_t                    = std::decay_t<decltype(generate_blocks<input, toks_t>().output)>;
    std::string operator()(const xdict &context) {
      std::string output{};
      ct::foreach<blocks_t>([&]<typename BlockT>() { BlockT::process(context, output); });
      return output;
    }
  };

  inline namespace literals {

    template <typename CharT, CharT... chars>
    requires std::same_as<CharT, char> constexpr auto operator"" _ctt() {
      constexpr char input[sizeof...(chars)] = {chars...};
      return renderer<ct::string<sizeof...(chars)>{input}>{};
    }

  }// namespace literals
}//namespace xdev::ctt
