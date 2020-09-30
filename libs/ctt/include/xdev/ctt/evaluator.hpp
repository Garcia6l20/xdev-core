/** @file xdev/ctt/evaluator.hpp
 * @brief Compile-time template blocks
 *
 */
#pragma once

#include <xdev/ct.hpp>
#include <xdev/str-tools.hpp>
#include <xdev/ctt/concepts.hpp>

#include <ctre.hpp>

namespace xdev::ctt {

  namespace evaluators {
    /** @brief Value evaluator
     * @details
     * Evaluates simple values:
     * Examples:
     * - simple variable: `{{ value }}`
     * - dictionary: `{{ dict.key }}`
     */
    struct value {
      template <ct::string input, size_t begin, size_t end>
      struct impl {
        static constexpr auto view = trim(input.subview(begin, end));
        static constexpr bool match() { return ctre::match<R"([\w\.]+)">(view); }
        constexpr auto        operator()(dictionary auto const&context) const noexcept {
          // TODO dict keys should be accessible from string view
          return context.at(view);
        }
      };
    };
    using evaluators_t = ct::tuple<value>;
  }// namespace evaluators

  /** @brief Get corresponding evaluator for the given @a input[begin:end]
   *
   * @tparam input  Input ct::string
   * @tparam begin  Begining of the expression
   * @tparam end    End of the expression
   * @return The matching evaluator or nothing (void)
   */
  template <ct::string input, size_t begin, size_t end>
  constexpr auto evaluator_of() {
    return ct::foreach<evaluators::evaluators_t>([]<typename EvaluatorT>() {
      if constexpr (EvaluatorT::template impl<input, begin, end>::match()) {
        return typename EvaluatorT::template impl<input, begin, end>{};
      }
    });
  }
}// namespace xdev::ctt
