/**
 * @file xdev-std-concepts.hpp
 * @brief Concepts library implementation.
 *
 * Since GCC enables concepts (with `-fconcepts` option) but does not provide the
 * concepts library yet... This file provides the missing std concepts wich will be included in
 * the future `concepts` include file.
 *
 * @see https://en.cppreference.com/w/cpp/concepts
 *
 **/
#pragma once

#include <string>
#include <memory>

#ifdef __cpp_lib_concepts
#include <concepts>
#else
#include <xdev/std-concepts.hpp>
#endif

#include <xdev/xdev-typetraits.hpp>

namespace xdev {

// forward declaration
class XObjectBase;

template<typename T>
struct is_shared_ptr : std::false_type {};
template<typename T>
struct is_shared_ptr<std::shared_ptr<T>> : std::true_type {};

template<typename T>
constexpr bool is_shared_ptr_v() {
  return is_shared_ptr<T>::value;
}

/**
 * @defgroup xobj-concepts XObject concepts
 * @{
 */

/**
 * @brief An XObject derived item.
 */
template<typename T>
concept XObjectDerived =
  std::same_as<std::decay_t<T>, xdev::XObjectBase> or std::derived_from<std::decay_t<T>, xdev::XObjectBase>;

/**
 * @brief An XObject container.
 */
template<typename T>
concept XObjectPointer = is_shared_ptr<T>::value and requires(T ptr) {
  { *ptr }
  ->XObjectDerived; // must have an arrow accessor
};

namespace variant {
  struct None;
  using variant_type = std::variant<None, bool, int, double, std::string>;
} // namespace variant


template<typename T>
concept XValueConvertible = std::convertible_to<std::decay_t<T>, variant::variant_type>;

/** @} */

/**
 * @brief A string convertible object
 */
template<typename T>
concept Stringable = requires(T obj) {
  { obj.toString() } ->std::same_as<std::string>;
  { obj.toString() } ->std::convertible_to<std::string>;
};

} // namespace xdev
