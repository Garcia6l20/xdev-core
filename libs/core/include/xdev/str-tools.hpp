/** @file xdev/str-tools.hpp
 * @brief String utilities
 */
#pragma once

#include <string_view>
#include <ranges>

namespace xdev {

  namespace rng = std::ranges;
  namespace views = std::views;

  /** @brief Left trim
   *
   * @param input Input view
   * @return Left-trimmed input
   */
  constexpr std::string_view ltrim(std::string_view input) {
    input.remove_prefix(std::min(input.find_first_not_of(" \t\r\v\n"), input.size()));
    return input;
  }

  /** @brief Right trim
   *
   * @param input Input view
   * @return Right-trimmed input
   */
  constexpr std::string_view rtrim(std::string_view input) {
    input.remove_suffix(std::min(input.size() - input.find_last_not_of(" \t\r\v\n") - 1, input.size()));
    return input;
  }

  /** @brief Trim
   *
   * @param input Input view
   * @return Trimmed input
   */
  constexpr std::string_view trim(std::string_view input) {
    return rtrim(ltrim(input));
  }
} // namespace xdev
