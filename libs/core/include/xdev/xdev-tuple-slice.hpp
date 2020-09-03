#pragma once

#include <tuple>
#include <string>

namespace xdev {

namespace detail {
  template<size_t __begin, size_t... __indices, typename Tuple>
  auto tuple_slice_impl(Tuple &&tuple, std::index_sequence<__indices...>) {
    return std::make_tuple(std::get<__begin + __indices>(std::forward<Tuple>(tuple))...);
  }
} // namespace detail

template <size_t __begin, size_t __count, typename Tuple>
auto tuple_slice(Tuple &&tuple) {
  static_assert(__count > 0, "splicing tuple to 0-length is weird...");
  return detail::tuple_slice_impl<__begin>(std::forward<Tuple>(tuple), std::make_index_sequence<__count>());
}

template <size_t __rbegin = 1, typename Tuple>
auto tuple_tail(Tuple &&tuple) {
  static_assert(__rbegin > 0, "invalid r-begin");
  static constexpr size_t __count = __rbegin;
  static constexpr size_t __begin = std::tuple_size_v<Tuple> - __count;
  static_assert(__count > 0, "splicing tuple to 0-length is weird...");
  return detail::tuple_slice_impl<__begin>(std::forward<Tuple>(tuple), std::make_index_sequence<__count>());
}

template <size_t __begin, size_t __count, typename Tuple>
using tuple_slice_t = decltype(tuple_slice<__begin, __count>(std::declval<Tuple>()));

template <size_t __rbegin, typename Tuple>
using tuple_tail_t = decltype(tuple_tail<__rbegin>(std::declval<Tuple>()));


using test_tuple = std::tuple<int, int, bool, nullptr_t, std::string>;
using sliced_test = tuple_tail_t<2, test_tuple>;

static_assert(std::same_as<std::tuple_element_t<0, sliced_test>, nullptr_t>);
static_assert(std::same_as<std::tuple_element_t<1, sliced_test>, std::string>);
}
