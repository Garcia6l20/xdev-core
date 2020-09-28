/** @file xdev/ctt/evaluator.hpp
 * @brief Compile-time template blocks
 *
 */
#pragma once

#include <xdev/ct.hpp>
#include <xdev/variant.hpp>
#include <xdev/str-tools.hpp>

#include <ctre.hpp>

namespace xdev::ctt {

  namespace evaluators {
    struct basic {
      template <ct::string input, size_t begin, size_t end>
      struct impl {
        static constexpr auto view = trim(input.subview(begin, end));
        static constexpr bool match() {
          return ctre::match<R"(\w+)">(view);
        }
        constexpr auto operator()(const xdict &context) const noexcept {
          // TODO dict keys should be accessible from string view
          return context.at(std::string{view});
        }
      };
    };
    using evaluators_t = ct::tuple<basic>;
  }

  template <ct::string input, size_t begin, size_t end>
  constexpr auto evaluator_of() {
    return ct::foreach<evaluators::evaluators_t>([]<typename EvaluatorT>() {
      if constexpr (EvaluatorT::template impl<input, begin, end>::match()) {
        return typename EvaluatorT::template impl<input, begin, end>{};
      }
    });
  }
}
