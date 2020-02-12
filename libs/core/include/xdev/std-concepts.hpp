// <concepts> -*- C++ -*-

/**
 * @file concepts
 **/

#ifndef _GLIBCXX_CONCEPTS
#define _GLIBCXX_CONCEPTS 1

#pragma GCC system_header

#ifdef __cpp_lib_concepts
#error "Concepts library implemented, you should not use this header"
#endif


#include <type_traits>
#include <functional>
#include <utility>

namespace std {

/** The `Same` concept requires types `T` and `U` to be equivalent types.
 * Examples:
 *  - Same<int, int>()       -> satsified
 *  - Same<int, int const>() -> not satisfied
 * @see https://en.cppreference.com/w/cpp/concepts/Same
 **/
template <typename T, typename U>
concept same_as = is_same_v<T, U> && is_same_v<U, T>;

/** A type `T` is convertible to another type `U` an expression
 * of type `T` is implicitly convertible to `U`.
 * Examples:
 *  - ConvertibleTo<int, int>()       -> satisfied
 *  - ConvertibleTo<int, int const>() -> satisfied
 *  - ConvertibleTo<int const, int>() -> not satisfied
 * @see https://en.cppreference.com/w/cpp/concepts/ConvertibleTo
 **/
template <typename From, typename To>
concept convertible_to = is_convertible_v<From, To> &&
    requires(From (&f)()) {
        static_cast<To>(f());
    };

/** The concept DerivedFrom<Derived, Base> is satisfied if and only if
 * Base is a class type that is either Derived or
 * a public and unambiguous base of Derived, ignoring cv-qualifiers.
 * @see https://en.cppreference.com/w/cpp/concepts/DerivedFrom
 **/
template <class Derived, class Base>
concept derived_from =
  is_base_of_v<Base, Derived> &&
  is_convertible_v<const volatile Derived*, const volatile Base*>;

template <class F, class...Args>
concept invocable = requires(F&& f, Args&&...args) {
    std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
};
template< class F, class... Args >
concept regular_invocable = std::invocable<F, Args...>;

#if 0
/** The concept CommonReference<T, U> specifies that two types T and U share
 * a common reference type (as computed by std::common_reference_t) to which both can be converted.
 * @see https://en.cppreference.com/w/cpp/concepts/CommonReference
 */
template < class T, class U >
concept common_reference =
  same_as<common_reference_t<T, U>, common_reference_t<U, T>> &&
  convertible_to<T, common_reference_t<T, U>> &&
  convertible_to<U, common_reference_t<T, U>>;

/** The concept Common<T, U> specifies that two types T and U share
 * a common type (as computed by std::common_type_t) to which both can be converted.
 * @see https://en.cppreference.com/w/cpp/concepts/Common
 */
template <class T, class U>
concept common =
  same_as<common_type_t<T, U>, common_type_t<U, T>> &&
  requires {
    static_cast<common_type_t<T, U>>(declval<T>());
    static_cast<common_type_t<T, U>>(declval<U>());
  } &&
  common_reference<
    add_lvalue_reference_t<const T>,
    add_lvalue_reference_t<const U>> &&
  common_reference<
    add_lvalue_reference_t<common_type_t<T, U>>,
    common_reference_t<
      add_lvalue_reference_t<const T>,
      add_lvalue_reference_t<const U>>>;

#endif

} // namespace std

#endif // _GLIBCXX_CONCEPTS
