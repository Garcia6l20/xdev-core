/** @file xdev/ct/tuple.hpp
 * @brief Compile-time tuple
 *
 * @details
 * Tuple implementation that can be passed through template parameters.
 *
 */
#pragma once

#include <tuple>

#include <xdev/typetraits.hpp>
#include <xdev/tuple-tools.hpp>

namespace xdev::ct {
  template <typename...Ts>
  struct tuple {
    using type = std::tuple<Ts...>;
    static constexpr size_t size = sizeof...(Ts);
  };

  template <typename TupleT>
  concept ct_tuple = specialization_of<TupleT, tuple>;

  template <ct_tuple TupleT>
  constexpr auto size = TupleT::size;



  namespace detail {
    template<size_t pos, class Tuple>
    struct at_impl;

    template<size_t pos, class... Types>
    struct at_impl<pos, tuple<Types...>>
    {
      template <size_t index, typename FirsT, typename...RestT>
      static constexpr auto impl() {
        if constexpr (index == pos) {
          return FirsT{};
        } else {
          return impl<index + 1, RestT...>();
        }
      }
      using type = decltype(impl<0, Types...>());
    };
  }
  template <size_t pos, typename TupleT>
  using at_t = typename detail::at_impl<pos, TupleT>::type;

  template <typename TupleT>
  using front_t = at_t<0, TupleT>;

  template <typename TupleT>
  using back_t = at_t<size<TupleT> - 1, TupleT>;

  namespace detail {
    template <typename Left, typename Right>
    struct concat_impl;

    template <typename...Left, typename...Right>
    struct concat_impl<tuple<Left...>, tuple<Right...>> {
      using type = tuple<Left..., Right...>;
    };
  }

  template <ct_tuple Left, ct_tuple Right>
  using cat_t = typename detail::concat_impl<Left, Right>::type;

  template <ct_tuple TupleT, typename T>
  using push_t = typename detail::concat_impl<TupleT, tuple<T>>::type;

  template <ct_tuple TupleT>
  constexpr decltype(auto) foreach (auto &&functor) {
    return tt::foreach<typename TupleT::type>(xfwd(functor));
  }
}
