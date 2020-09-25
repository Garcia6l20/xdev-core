#pragma once

#include <string>
#include <tuple>

#include <xdev/typetraits.hpp>

#include <ctti/nameof.hpp>
#include <spdlog/spdlog.h>

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
    template <size_t begin_, size_t... indices_, typename Tuple>
    auto tuple_slice_impl(Tuple &&tuple, std::index_sequence<indices_...>) {
      return std::make_tuple(std::get<begin_ + indices_>(xfwd(tuple))...);
    }

  }// namespace detail

  template <size_t begin_, size_t count_, typename Tuple>
  auto slice(Tuple &&tuple) {
    static_assert(count_ > 0, "slicing tuple to 0-length is weird...");
    return detail::tuple_slice_impl<begin_>(xfwd(tuple), std::make_index_sequence<count_>());
  }

  template <size_t rbegin_ = 1, typename Tuple>
  auto tail(Tuple &&tuple) {
    static_assert(rbegin_ > 0, "invalid r-begin");
    static constexpr size_t count_ = rbegin_;
    static constexpr size_t begin_ = std::tuple_size_v<Tuple> - count_;
    static_assert(count_ > 0, "splicing tuple to 0-length is weird...");
    return detail::tuple_slice_impl<begin_>(xfwd(tuple), std::make_index_sequence<count_>());
  }

  template <size_t begin_, size_t count_, typename Tuple>
  using slice_t = decltype(slice<begin_, count_>(std::declval<Tuple>()));

  template <size_t rbegin_, typename Tuple>
  using tail_t = decltype(tail<rbegin_>(std::declval<Tuple>()));

  namespace detail {
    /** @brief Index-invocable requirement
     * @details Satisfied when the object can be called with a templated operator with std::size_t as first parameter.
     *
     * Example:
     * {@code
     * constexpr auto foo = []<size_t ii> {
     *   return ii;
     * };
     * static_assert(is_index_invocable<decltype(foo)>);
     * }
     * @tparam T
     * @tparam Args
     */
    template <typename T, typename... Args>
    concept is_index_invocable = requires(T v, Args... args) {
      {v.template operator()<std::size_t(42)>(args...)};
    };

    template <size_t index = 0, typename TupleT, typename LambdaT>
    constexpr auto foreach_impl(TupleT &tuple, LambdaT &&lambda) {
      using ValueT = std::decay_t<decltype(std::get<index>(tuple))>;
      auto invoker = [&]() mutable {
        if constexpr (is_index_invocable<decltype(lambda), ValueT>) {
          return lambda.template operator()<index>(std::get<index>(tuple));
        } else {
          return lambda(std::get<index>(tuple));
        }
      };
      using ReturnT = decltype(invoker());
      if constexpr (std::is_void_v<ReturnT>) {
        invoker();
        if constexpr (index + 1 < std::tuple_size_v<TupleT>) {
          return foreach_impl<index + 1>(tuple, xfwd(lambda));
        }
      } else {
        return invoker();
      }
    }
  }// namespace detail

  constexpr decltype(auto) foreach (is_tuple auto &tuple, auto &&functor) {
    return detail::foreach_impl(tuple, functor);
  }

  constexpr decltype(auto) push_back(is_tuple auto &&tuple, auto &&elem) {
    return std::tuple_cat(xfwd(tuple), std::make_tuple(xfwd(elem)));
  }

  namespace detail {

    template <size_t index = 0, typename TupleT, typename LambdaT, typename CurrentT = std::tuple<>>
    constexpr decltype(auto) transform_impl(TupleT &tuple, LambdaT &&lambda, CurrentT current = {}) {
      using ValueT = at_t<index, TupleT>;
      auto invoker = [&]() mutable {
        if constexpr (is_index_invocable<decltype(lambda), ValueT>) {
          return lambda.template operator()<index>(std::get<index>(tuple));
        } else {
          return lambda(std::get<index>(tuple));
        }
      };
      using ReturnT = decltype(invoker());
      if constexpr (std::is_void_v<ReturnT>) {
        invoker();
        if constexpr (index + 1 < size<TupleT>) {
          return transform_impl<index + 1>(tuple, xfwd(lambda), std::move(current));
        } else {
          return current;
        }
      } else {
        auto then = [&](auto &&next) mutable {
          if constexpr (index + 1 < size<TupleT>) {
            return transform_impl<index + 1>(tuple, xfwd(lambda), xfwd(next));
          } else {
            return next;
          }
        };
        return then(push_back(std::move(current),
                              invoker()));
      }
    }

    template <typename TupleT, size_t index = 0, typename LambdaT, typename CurrentT = std::tuple<>>
    constexpr decltype(auto) transform_impl2(LambdaT &&lambda, CurrentT current = {}) {
      using ValueT = std::decay_t<at_t<index, TupleT>>;
      auto invoker = [&] {
        if constexpr (is_index_invocable<decltype(lambda), ValueT>) {
          return lambda.template operator()<index>(ValueT{});
        } else {
          return lambda(ValueT{});
        }
      };
      using ReturnT = decltype(invoker());
      auto then = [&](auto &&next) {
        return transform_impl2<TupleT, index + 1>(xfwd(lambda), xfwd(next));
      };
      if constexpr (std::is_void_v<ReturnT>) {
        invoker();
        if constexpr (index + 1 < size<TupleT>) {
          return then(std::move(current));
        } else {
          return current;
        }
      } else {
        auto next = push_back(std::move(current), invoker());
        if constexpr (index + 1 < size<TupleT>) {
          return then(std::move(next));
        } else {
          return next;
        }
      }
    }
  }// namespace detail

  /** @brief Transform a tuple
   *
   * Makes easy to transform tuples.
   *
   * @param tuple Input tuple
   * @param generator Generator function
   * @return a tuple of types returned by @a generator
   */
  constexpr decltype(auto) transform(is_tuple auto const & tuple,
                                     auto &&generator) {
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


  namespace detail {
    template <size_t index = 0, typename LambdaT, typename CurrentT = std::tuple<>>
    constexpr decltype(auto) generate_impl(LambdaT &&lambda, CurrentT current = {}) {
      auto invoker = [&]() mutable {
        return lambda.template operator()<index>();
      };
      using ReturnT = decltype(invoker());
      if constexpr (std::is_void_v<ReturnT>) {
        return current;
      } else {
        auto next = push_back(std::move(current), invoker());
        return generate_impl<index + 1>(xfwd(lambda), std::move(next));
      }
    }
  }// namespace detail

  /** @brief Generate a tuple
   * @details
   * generate will call your lambda until it returns void
   * @req @a lambda shall satisfy detail::is_index_invocable
   */
  template <detail::is_index_invocable LambdaT>
  constexpr auto generate(LambdaT &&lambda) {
    return detail::generate_impl(xfwd(lambda));
  }

  /** @brief Flatten tuples in a tuple
   * @details
   * Example:
   * {@code
   * constexpr auto input = std::make_tuple(1, std::make_tuple(2, std::make_tuple(3)));
   * static_assert(flatten<true>(input) == std::make_tuple(1, 2, 3));
   * }
   * @tparam recursive Whatever should apply recursively
   * @param tuple Input tuple
   * @return Flattened @a input tuple
   */
  template <bool recursive = true>
  constexpr auto flatten(is_tuple auto &&tuple);

  namespace detail {
    template <bool recursive, size_t index = 0, typename LambdaT, typename CurrentT = std::tuple<>>
    constexpr decltype(auto) flatten_impl(LambdaT &&lambda, CurrentT current = {}) {
      auto invoker = [&]() mutable {
        return lambda.template operator()<index>();
      };
      using ReturnT = decltype(invoker());
      auto then = [&](auto &&next) {
        return flatten_impl<recursive, index + 1>(xfwd(lambda), xfwd(next));
      };
      if constexpr (std::is_void_v<ReturnT>) {
        return current;
      } else if constexpr (is_tuple<ReturnT>) {
        if constexpr (recursive) {
          return then(std::tuple_cat(std::move(current), flatten(invoker())));
        } else {
          return then(std::tuple_cat(std::move(current), invoker()));
        }
      } else {
        return then(push_back(std::move(current), invoker()));
      }
    }
  }// namespace detail

  template <bool recursive>
  constexpr auto flatten(is_tuple auto &&tuple) {
    constexpr auto sz = tt::size<std::decay_t<decltype(tuple)>>;
    return detail::flatten_impl<recursive>([&tuple, sz]<size_t index>() {
      if constexpr (index >= sz) {
        return;
      } else {
        return tt::at<index>(tuple);
      }
    });
  }

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

}// namespace xdev::tt
