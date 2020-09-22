/**
 * @file xdev-variant-array.hpp
 */
#pragma once

#include <list>
#include <string>

#include <xdev/xdev-typetraits.hpp>
#include <xdev/xdev-concepts.hpp>

namespace xdev::variant {


template <typename StringPolicy>
class Variant;

template <typename StringPolicy>
using ListInitList = std::initializer_list<Variant<StringPolicy>>;


template<template<class, class> typename what, typename T, typename... Args>
struct variant_disjunction {
  using type = typename std::disjunction<typename what<T, Args>::type...>::type;
  enum { value = std::disjunction<typename what<T, Args>::type...>::value };
};

template<typename T, typename... Args>
concept convertible_to_one_of = std::disjunction_v<typename std::is_convertible<T, Args>::type...>;

template<typename T>
concept is_container = requires(T v) {
  {v.begin()};
  {v.end()};
};
static_assert(is_container<std::list<int>>);

template<typename T, typename StringPolicy>
concept XListConvertible = is_container<T> and std::convertible_to<typename T::value_type, variant_type<StringPolicy>>;

template <typename StringPolicy>
class List {
public:
  using list_t = std::list<Variant<StringPolicy>>;

  inline List();

  inline List(List &&other);
  inline List &operator=(List &&other);
  inline List(const List &other);
  inline List &operator=(const List &other);
  inline List(const ListInitList<StringPolicy> &value);
  inline List(XListConvertible<StringPolicy> auto &&value);
  inline List(const XListConvertible<StringPolicy> auto &value);

  using iterator = typename list_t::iterator;
  using const_iterator = typename list_t::const_iterator;
  inline iterator begin();
  inline iterator end();
  inline auto &front();
  inline auto &back();
  inline const_iterator begin() const;
  inline const_iterator end() const;
  inline size_t size() const;
  inline auto &operator[](size_t index);
  inline const auto &operator[](size_t index) const;

  // TODO use StringPolicy::string_type
  inline std::string toString() const;

  template<typename... ArgsT>
  iterator emplace(const_iterator pos, ArgsT... args) {
    return _value.emplace(pos, xfwd(args)...);
  }
  template<typename... ArgsT>
  iterator emplace_front(ArgsT... args) {
    return _value.emplace_front(xfwd(args)...);
  }
  template<typename... ArgsT>
  iterator emplace_back(ArgsT... args) {
    return _value.emplace_back(xfwd(args)...);
  }

  enum Direction { Front, Back };
  template<Direction direction = Front>
  inline void pop(size_t count = 1);

  template<Direction direction = Back, typename First, typename... Rest>
  inline void push(First &&first, Rest &&... var);

  inline List::iterator find(Variant<StringPolicy> &&value);

  template<typename IteratorT>
  auto erase(IteratorT &&iter) {
    return _value.erase(xfwd(iter));
  }

  template <typename FriendStringPolicy>
  friend inline List &operator>>(Variant<StringPolicy> &&var, List &);

  template <typename FriendStringPolicy>
  friend inline List &operator-(size_t count, List &);

  template <typename FriendStringPolicy>
  friend inline List &operator<<(List &, Variant<StringPolicy> &&var);

  template <typename FriendStringPolicy>
  friend inline List &operator-(List &, size_t count);

  template <typename FriendStringPolicy>
  friend constexpr bool operator==(const List<FriendStringPolicy> &, const List<FriendStringPolicy> &);

  static constexpr const char *ctti_nameof() { return "xlist"; }

private:
  list_t _value;
};

} // namespace xdev::variant
