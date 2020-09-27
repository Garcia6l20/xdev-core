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
    template<size_t N>
    struct ct_string {

        constexpr ct_string(const char (&input)[N]) noexcept {
            std::copy(&input[0], &input[N], data_);
        }

        constexpr ct_string(const ct_string &other) noexcept {
            std::copy(&other.data_[0], &other.data_[N], data_);
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

        constexpr std::string_view body() const noexcept {
            return {begin(), end()};
        }

        char data_[N]{};
    };

    template<typename CharT, size_t N> ct_string(const CharT (&)[N]) -> ct_string<N>;
    template<size_t N> ct_string(ct_string<N>) -> ct_string<N>;

} // namespace xdev::ct
