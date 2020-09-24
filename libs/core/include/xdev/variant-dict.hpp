/**
 * @file xdev-variant-dict.hpp
 */
#pragma once

#include <map>

namespace xdev::variant {

template <typename StringPolicy>
class Variant;

template <typename StringPolicy>
class Value;

template <typename StringPolicy>
using DictNode = std::pair<const Value<StringPolicy>, Variant<StringPolicy>>;

template <typename StringPolicy>
using DictInitList = std::initializer_list<DictNode<StringPolicy>>;

template <typename StringPolicy>
class Dict {
public:
  using map_t = std::map<Value<StringPolicy>, Variant<StringPolicy>>;

  inline Dict();
  inline Dict(const Dict &other);
  inline Dict &operator=(const Dict &other);
  inline Dict(Dict &&other);
  inline Dict &operator=(Dict &&other);
  inline Dict(const DictInitList<StringPolicy> &value);

  inline decltype(auto) begin();
  inline decltype(auto) end();
  inline decltype(auto) begin() const;
  inline decltype(auto) end() const;
  inline size_t size() const;
  inline Variant<StringPolicy> &operator[](const Value<StringPolicy> &index);

  template<typename... RestT>
  inline Variant<StringPolicy> &at(Value<StringPolicy> &&key, RestT &&... rest);

  template<typename... RestT>
  inline Variant<StringPolicy> &at(const Value<StringPolicy> &key, const RestT &... rest);

  template<typename... RestT>
  inline const Variant<StringPolicy> &at(const Value<StringPolicy> &key, const RestT &... rest) const;

  inline Variant<StringPolicy> &dotAt(const std::string &key);
  inline const Variant<StringPolicy> &dotAt(const std::string &key) const;
  inline std::string toString() const;

  inline Dict &update(Dict &&other);

  inline bool operator==(const Dict &) const;
  inline auto operator<=>(const Dict &) const;

  template<typename T>
  inline bool contains(const T &key) const {
    return _value.contains(key);
  }

  static constexpr const char *ctti_nameof() { return "xdict"; }

private:
  map_t _value;
};

} // namespace xdev::variant
