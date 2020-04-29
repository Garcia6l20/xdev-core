/**
 * @file xdev-variant-array.hpp
 */
#pragma once

#include <list>
#include <string>

#include <xdev/xdev-typetraits.hpp>
#include <xdev/xdev-concepts.hpp>

namespace xdev::variant {

class Variant;
using ListInitList = std::initializer_list<Variant>;


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

template<typename T>
concept XListConvertible = is_container<T> and std::convertible_to<typename T::value_type, variant_type>;

class List {
public:
  using list_t = std::list<Variant>;

  inline List();

  inline List(List &&other);
  inline List &operator=(List &&other);
  inline List(const List &other);
  inline List &operator=(const List &other);
  inline List(const ListInitList &value);
  inline List(XListConvertible auto &&value);
  inline List(const XListConvertible auto &value);

  using iterator = list_t::iterator;
  using const_iterator = list_t::const_iterator;
  inline iterator begin();
  inline iterator end();
  inline Variant &front();
  inline Variant &back();
  inline const_iterator begin() const;
  inline const_iterator end() const;
  inline size_t size() const;
  inline Variant &operator[](size_t index);
  inline const Variant &operator[](size_t index) const;
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

  inline List::iterator find(Variant &&value);

  template<typename IteratorT>
  auto erase(IteratorT &&iter) {
    return _value.erase(xfwd(iter));
  }

  friend inline List &operator>>(Variant &&var, List &);
  friend inline List &operator-(size_t count, List &);

  friend inline List &operator<<(List &, Variant &&var);
  friend inline List &operator-(List &, size_t count);

  friend inline bool operator==(const List &, const List &);

  static constexpr const char *ctti_nameof() { return "xlist"; }

private:
  list_t _value;
};

} // namespace xdev::variant
