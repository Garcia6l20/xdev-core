#pragma once

#include <tuple>
#include <string>

#include <xdev/xdev-typetraits.hpp>

#include <spdlog/spdlog.h>
#include <ctti/nameof.hpp>

/** @brief Tuple tools
 *
 */
namespace xdev::tt {

template <typename T>
concept is_tuple = specialization_of<T, std::tuple>;

template <size_t index>
constexpr decltype(auto) at(is_tuple auto &tuple) noexcept {
  return std::get<index>(tuple);
}

template <size_t index, is_tuple TupleT>
using at_t = std::tuple_element_t<index, TupleT>;

template <is_tuple TupleT>
constexpr auto size = std::tuple_size_v<TupleT>;

namespace detail {
template<size_t __begin, size_t... __indices, typename Tuple>
auto tuple_slice_impl(Tuple &&tuple, std::index_sequence<__indices...>) {
  return std::make_tuple(std::get<__begin + __indices>(xfwd(tuple))...);
}

} // namespace detail

template<size_t __begin, size_t __count, typename Tuple>
auto slice(Tuple &&tuple) {
  static_assert(__count > 0, "slicing tuple to 0-length is weird...");
  return detail::tuple_slice_impl<__begin>(xfwd(tuple), std::make_index_sequence<__count>());
}

template<size_t __rbegin = 1, typename Tuple>
auto tail(Tuple &&tuple) {
  static_assert(__rbegin > 0, "invalid r-begin");
  static constexpr size_t __count = __rbegin;
  static constexpr size_t __begin = std::tuple_size_v<Tuple> - __count;
  static_assert(__count > 0, "splicing tuple to 0-length is weird...");
  return detail::tuple_slice_impl<__begin>(xfwd(tuple), std::make_index_sequence<__count>());
}

template<size_t __begin, size_t __count, typename Tuple>
using slice_t = decltype(slice<__begin, __count>(std::declval<Tuple>()));

template<size_t __rbegin, typename Tuple>
using tail_t = decltype(tail<__rbegin>(std::declval<Tuple>()));

namespace detail {
  template<size_t index = 0, typename TupleT, typename LambdaT>
  constexpr auto foreach_impl (TupleT &tuple, LambdaT && lambda) {
    using ValueT = std::decay_t<decltype(std::get<index>(tuple))>;
    using ReturnT = decltype(lambda(std::declval<ValueT>()));
    if constexpr (std::is_void_v<ReturnT>) {
      lambda(std::get<index>(tuple));
      if constexpr (index + 1 < std::tuple_size_v<TupleT>) {
        return foreach_impl<index + 1>(tuple, xfwd(lambda));
      }
    } else {
      return lambda(std::get<index>(tuple));
    }
  }
}

constexpr decltype(auto) foreach(is_tuple auto &tuple, auto &&functor) {
  return detail::foreach_impl(tuple, functor);
}

constexpr decltype(auto) push_back(is_tuple auto &&tuple, auto &&elem) {
  return std::tuple_cat(xfwd(tuple), std::make_tuple(xfwd(elem)));
}

namespace detail {

  template<size_t index = 0, typename TupleT, typename LambdaT, typename CurrentT = std::tuple<>>
  constexpr decltype(auto) transform_impl(TupleT &tuple, LambdaT &&lambda, CurrentT current = {}) {
    using ValueT = at_t<index, TupleT>;
    using ReturnT = decltype(lambda(std::declval<ValueT&>()));
    if constexpr (std::is_void_v<ReturnT>) {
      lambda(std::get<index>(tuple));
      if constexpr (index + 1 < size<TupleT>) {
        return transform_impl<index + 1>(tuple, xfwd(lambda), std::move(current));
      } else {
        return current;
      }
    } else {
      auto next = push_back(std::move(current), lambda(at<index>(tuple)));
      if constexpr (index + 1 < size<TupleT>) {
        return transform_impl<index + 1>(tuple, xfwd(lambda), std::move(next));
      } else {
        return next;
      }
    }
  }

  template<typename TupleT, size_t index = 0, typename LambdaT, typename CurrentT = std::tuple<>>
  constexpr decltype(auto) transform_impl2(LambdaT &&lambda, CurrentT current = {}) {
    using ValueT = std::decay_t<at_t<index, TupleT>>;
    using ReturnT = decltype(lambda(std::declval<ValueT&>()));
    if constexpr (std::is_void_v<ReturnT>) {
      lambda(ValueT{});
      if constexpr (index + 1 < size<TupleT>) {
        return transform_impl2<TupleT, index + 1>(xfwd(lambda), std::move(current));
      } else {
        return current;
      }
    } else {
      auto next = push_back(std::move(current), lambda(ValueT{}));
      if constexpr (index + 1 < size<TupleT>) {
        return transform_impl2<TupleT, index + 1>(xfwd(lambda), std::move(next));
      } else {
        return next;
      }
    }
  }
} // namespace detail

/** @brief Transform a tuple
 *
 * Makes easy to transform tuples.
 *
 * @param tuple Input tuple
 * @param generator Generator function
 * @return a tuple of types returned by @a generator
 */
constexpr decltype(auto) transform(is_tuple auto const &tuple, auto &&generator) {
  return detail::transform_impl(tuple, xfwd(generator));
}
constexpr decltype(auto) transform(is_tuple auto &tuple, auto &&generator) {
  return detail::transform_impl(tuple, xfwd(generator));
}

template <is_tuple TupleT>
constexpr decltype(auto) transform(auto &&generator) {
  return detail::transform_impl2<TupleT>(xfwd(generator));
}

/** @brief Transform a tuple type
 *
 * Same as tt::transform but works on types.
 *
 * Example:
 * {@code
 * using tuple_type = std::tuple<int, bool, std::string>;
 * using trivial_tuple_type = tt::transform_t<tuple_type, []<typename T>(T) {
 *  if constexpr (std::is_trivially_constructible_v<T>)
 *      return T{};
 * }>;
 * }
 */
template <is_tuple TupleT, auto lambda>
using transform_t = std::decay_t<decltype(transform<TupleT>(std::declval<std::decay_t<decltype(lambda)>>()))>;

/** @brief Count types in a tuple
 *
 * @tparam TupleT   Type of the tuple
 * @tparam T        Types to count
 */
template <is_tuple TupleT, typename T>
constexpr auto type_count = [] {
  size_t count = 0;
  transform<TupleT>([&count]<typename ItemT>(ItemT) mutable {
    if constexpr (decays_to<ItemT, T>) {
      ++count;
    }
  });
  return count;
}();

template <is_tuple TupleT, typename T>
constexpr auto index_of = [] {
  constexpr TupleT tuple;
  return foreach(tuple, [index = size_t{0}]<typename ItemT>(ItemT) mutable {
    if constexpr (decays_to<ItemT, T>) {
      return index;
    }
    ++index;
  });
}();

} // namespace xdev::tt
