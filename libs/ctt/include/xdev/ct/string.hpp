/** @file xdev/ct/string.hpp
 * @brief Compile-time string
 */
#pragma once

#include <string_view>

namespace xdev::ct {

  /** @brief Compile-time string
   *
   * @note Inspired from ctll::fixed_string
   * @tparam N [Deduced] Number of elements in the string
   */
  template <size_t N>
  struct string {

    constexpr string(const char (&input)[N]) noexcept {
      std::copy(&input[0], &input[N], data_);
    }

    constexpr string(const string &other) noexcept {
      std::copy(&other.data_[0], &other.data_[N], data_);
    }

    constexpr explicit string() noexcept {}

    static constexpr auto from(const char *view) noexcept {
      string<N> str{};
      std::copy(view, view + N, str.data_);
      return str;
    }

    constexpr auto begin() const noexcept {
      return &data_[0];
    }

    constexpr auto end() const noexcept {
      return &data_[N - 1];
    }

    constexpr auto size() const noexcept {
      return N;
    }

    constexpr auto operator[](size_t index) const noexcept {
      return data_[index];
    }

    [[nodiscard]] constexpr std::string_view view() const noexcept {
      return {begin(), end()};
    }

    [[nodiscard]] constexpr std::string_view subview(size_t start, size_t stop) const noexcept {
      return {begin() + start, begin() + std::min(stop, size())};
    }

    char data_[N]{};
  };

//  template <typename CharT, size_t N>
//  string(const CharT (&)[N]) -> string<N>;
//
//  template <size_t N>
//  string(string<N>) -> string<N>;

  inline namespace literals {

    template <typename CharT, CharT... chars>
    requires std::same_as<CharT, char> constexpr auto operator"" _cts() {
      constexpr char elems[sizeof...(chars)] = {chars...};
      return xdev::ct::string<sizeof...(chars)>{elems};
    }
  } // namespace literals

}// namespace xdev::ct
