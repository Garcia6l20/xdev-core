/** @file xdev/str-tools.hpp
 * @brief String utilities
 */
#pragma once

#include <string_view>

namespace xdev {

  namespace detail {
    constexpr std::string_view default_trim_chars = " \t\r\v\n";
  }

  /** @brief Left trim
   *
   * @param input Input view
   * @return Left-trimmed input
   */
  constexpr std::string_view ltrim(std::string_view input, std::string_view chars = detail::default_trim_chars) {
    input.remove_prefix(std::min(input.find_first_not_of(chars), input.size()));
    return input;
  }

  /** @brief Right trim
   *
   * @param input Input view
   * @return Right-trimmed input
   */
  constexpr std::string_view rtrim(std::string_view input, std::string_view chars = detail::default_trim_chars) {
    input.remove_suffix(std::min(input.size() - input.find_last_not_of(chars) - 1, input.size()));
    return input;
  }

  /** @brief Trim
   *
   * @param input Input view
   * @return Trimmed input
   */
  constexpr std::string_view trim(std::string_view input, std::string_view chars = detail::default_trim_chars) {
    return rtrim(ltrim(input, chars), chars);
  }
} // namespace xdev
