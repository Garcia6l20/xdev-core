#include <xdev/variant.hpp>
#include <xdev/tools.hpp>
#include <xdev/object.hpp>
#include <xdev/typetraits.hpp>

#include <boost/type_index.hpp>

#include <fmt/format.h>

#include <algorithm>

#include <xdev/variant-value.inl>
#include <xdev/variant-list.inl>
#include <xdev/variant-dict.inl>
#include <xdev/variant-function.inl>

#include <xdev/json.hpp>
#include <xdev/yaml.hpp>

namespace std {

// template <>
// struct hash<xdev::Variant<StringPolicy>::Variant>
//{
//    std::size_t operator()(const xdev::Variant<StringPolicy>::Variant& var) const
//    {
//        return var.hash();
//    }
//};

template<xdev::Stringable T>
static inline std::string to_string(const T &obj) {
  return obj.toString();
}

template<xdev::Stringable T>
static inline std::ostream &operator<<(std::ostream &stream, const T &obj) {
  stream << obj.toString();
  return stream;
}

} // namespace std

namespace xdev::variant {

template<typename StringPolicy>
Variant<StringPolicy>::Variant() : _value {Value {}} {
}

// template<typename...Ts>
// requires std::constructible_from<List, Ts...>
// template <typename StringPolicy> Variant<StringPolicy>::Variant(Ts&&...values):
// _value{List{std::forward<Ts>(values)...}} {}

template<typename StringPolicy>
Variant<StringPolicy>::Variant(const Variant &other) : _value(other._value) {
}

template<typename StringPolicy>
Variant<StringPolicy> &Variant<StringPolicy>::operator=(const Variant &other) {
  _value = other._value;
  return *this;
}

template<typename StringPolicy>
Variant<StringPolicy>::Variant(Variant &&other) : _value(std::move(other._value)) {
  other._value = Value {};
}

template<typename StringPolicy>
Variant<StringPolicy> &Variant<StringPolicy>::operator=(Variant &&other) {
  _value = std::move(other._value);
  other._value = Value {};
  return *this;
}

template<typename StringPolicy>
bool Variant<StringPolicy>::operator==(const Variant &b) const {
    return tools::visit_2way(tools::overloaded{
        []<typename T>(const T& lhs, const T& rhs){
    return lhs == rhs;
        },
        []<typename T>
        (const Value<StringPolicy>& lhs, const T& rhs) requires (std::convertible_to<T, Value<StringPolicy>> and not std::same_as<T, Value<StringPolicy>>) {
            return lhs == rhs;
}
, []<typename T, typename U>(const T &, const U &) requires(not std::same_as<T, U>) {
  return false;
}
} // namespace xdev::variant
, _value, b._value);
}

template<typename StringPolicy>
std::weak_ordering Variant<StringPolicy>::operator<=>(const Variant &b) const {
    return tools::visit_2way(tools::overloaded{
        // List stuff
        [](const List<StringPolicy>&lhs, const List<StringPolicy>&rhs){
    if (lhs == rhs) {
      return std::weak_ordering::equivalent;
    }
    return std::weak_ordering::less;
        },
        []<typename T>(const List<StringPolicy>&, const T&) requires (not one_of<T, Dict<StringPolicy>, Function<StringPolicy>, SharedObject>) {
            return std::weak_ordering::less;
}
, []<typename T>(const T &, const List<StringPolicy> &) requires(
    not one_of<T, Dict<StringPolicy>, Function<StringPolicy>, SharedObject>) {
  return std::weak_ordering::greater;
}
,
  // SharedObject stuff
  [](const SharedObject &lhs, const SharedObject &rhs) {
    if (lhs == rhs) {
      return std::weak_ordering::equivalent;
    }
    return std::weak_ordering::less;
  },
  []<typename T>(const SharedObject &, const T &) requires(not one_of<T, Function<StringPolicy>, Dict<StringPolicy>>) {
  return std::weak_ordering::less;
}
, []<typename T>(const T &, const SharedObject &) requires(not one_of<T, Function<StringPolicy>, Dict<StringPolicy>>) {
  return std::weak_ordering::greater;
}
,
  // Function stuff
  [](const Function<StringPolicy> &lhs, const Function<StringPolicy> &rhs) {
    if (lhs == rhs) {
      return std::weak_ordering::equivalent;
    }
    return std::weak_ordering::less;
  },
  []<typename T>(const Function<StringPolicy> &, const T &) requires(not one_of<T, Dict<StringPolicy>>) {
  return std::weak_ordering::less;
}
, []<typename T>(const T &, const Function<StringPolicy> &) requires(not one_of<T, Dict<StringPolicy>>) {
  return std::weak_ordering::greater;
}
,
  // Dict stuff
  [](const Dict<StringPolicy> &lhs, const Dict<StringPolicy> &rhs) {
    if (lhs == rhs) {
      return std::weak_ordering::equivalent;
    }
    return std::weak_ordering::less;
  },
  []<typename T>(const Dict<StringPolicy> &, const T &) { return std::weak_ordering::less; },
  []<typename T>(const T &, const Dict<StringPolicy> &) { return std::weak_ordering::greater; },
  // Value stuff
  []<typename T, typename U>(const T &lhs, const U &rhs) { return lhs <=> rhs; },
  []<typename T>(const std::string &, const T &) { return std::weak_ordering::less; },
  []<typename T>(const T &, const std::string &) { return std::weak_ordering::greater; },
  [](const std::string &lhs, const std::string &rhs) { return std::strcmp(lhs.c_str(), rhs.c_str()) <=> 0; },
}, _value, b._value);
}

template<typename StringPolicy>
bool Variant<StringPolicy>::operator==(char const *b) const {
  return std::visit(tools::overloaded {[]<typename T>(const T &) { return false; },
                      [b](const Value<StringPolicy> &lhs) { return lhs == b; }},
    _value);
}

template<typename StringPolicy>
std::weak_ordering Variant<StringPolicy>::operator<=>(const char *b) const {
  return std::visit(tools::overloaded {[]<typename T>(const T &) { return std::weak_ordering::less; },
                      [b](const Value<StringPolicy> &lhs) { return lhs <=> 0; }},
    _value);
}

template<typename StringPolicy>
template<typename T>
T &Variant<StringPolicy>::get() {
  if constexpr (
    is_one_of_v<T, Value<StringPolicy>, List<StringPolicy>, Dict<StringPolicy>, Function<StringPolicy>, SharedObject>)
    return std::get<T>(_value);
  else
    return std::get<Value<StringPolicy>>(_value).template get<T>();
}

template<typename StringPolicy>
template<typename T>
const T &Variant<StringPolicy>::get() const {
  if constexpr (
    is_one_of_v<T, Value<StringPolicy>, List<StringPolicy>, Dict<StringPolicy>, Function<StringPolicy>, SharedObject>)
    return std::get<T>(_value);
  else
    return std::get<Value<StringPolicy>>(_value).template get<T>();
}

template<typename StringPolicy>
template<XObjectDerived ObjectT>
typename ObjectT::ptr Variant<StringPolicy>::get() {
  return std::get<XObjectBase::ptr>(_value)->template cast<ObjectT>();
}

template<typename StringPolicy>
template<XObjectDerived ObjectT>
typename ObjectT::ptr Variant<StringPolicy>::get() const {
  return std::get<XObjectBase::ptr>(_value)->template cast<ObjectT>();
}

template<typename StringPolicy>
template<typename T>
bool Variant<StringPolicy>::is() const {
  return std::visit(
    [&]<typename TInput>(TInput &&) {
      using TVal = std::decay_t<TInput>;
      // check Value, List, Dict types
      if constexpr (std::same_as<TVal, T>)
        return true;
      // check inner Value type
      else if constexpr (std::same_as<TVal, Value<StringPolicy>>)
        return get<Value<StringPolicy>>().template is<T>();
      else
        return false;
    },
    _value);
}

template<typename StringPolicy>
template<typename T>
T Variant<StringPolicy>::convert() const {
  return visit([&](auto &&value) -> T {
    using TVal = std::decay_t<decltype(value)>;
    if constexpr (std::is_convertible<TVal, T>::value) {
      return static_cast<T>(value);
    }
    throw ConvertError("Cannot apply requested convertion");
  });
}

template<typename StringPolicy>
template<bool inner>
std::string Variant<StringPolicy>::typeName() const {
  return visit<inner>([](auto &&value) -> std::string {
    using T = std::decay_t<decltype(value)>;
    // return boost::typeindex::type_id<T>().pretty_name();
    return ctti::nameof<T>().str();
  });
}

template<typename StringPolicy>
template<bool inner, typename Visitor>
decltype(auto) Variant<StringPolicy>::visit(Visitor &&visitor) {
  if (is<Value<StringPolicy>>() && inner)
    return get<Value<StringPolicy>>().visit(std::forward<Visitor>(visitor));
  else
    return std::visit(std::forward<Visitor>(visitor), _value);
}

template<typename StringPolicy>
template<bool inner, typename Visitor>
decltype(auto) Variant<StringPolicy>::visit(Visitor &&visitor) const {
  if (inner && is<Value<StringPolicy>>())
    return get<Value<StringPolicy>>().visit(std::forward<Visitor>(visitor));
  else
    return std::visit(std::forward<Visitor>(visitor), _value);
}

template<typename StringPolicy>
std::string Variant<StringPolicy>::toString() const {
  return visit<false>([](auto &&value) -> std::string {
    using T = std::decay_t<decltype(value)>;
    if constexpr (one_of<T, Value<StringPolicy>, List<StringPolicy>, Dict<StringPolicy>>)
      return value.toString();
    else if constexpr (XObjectPointer<T>)
      return fmt::format("{}[0x{}]", value->objectName(), static_cast<void *>(value.get()));
    else
      throw std::runtime_error("Shall not append");
  });
}

template<typename StringPolicy>
bool Variant<StringPolicy>::empty() const {
  return is<None>();
}

// ---------------------------------
// Object API
//

template<typename StringPolicy>
Variant<StringPolicy>::Variant(const XObjectPointer auto &obj) : _value {std::dynamic_pointer_cast<XObjectBase>(obj)} {
}

template<typename StringPolicy>
Variant<StringPolicy>::Variant(XObjectPointer auto &&obj) : _value {std::dynamic_pointer_cast<XObjectBase>(xfwd(obj))} {
}

// ---------------------------------
// Value API
//

template<typename StringPolicy>
Variant<StringPolicy>::Variant(const char *data) : _value {Value {data}} {
}

template<typename StringPolicy>
Variant<StringPolicy>::Variant(const Value<StringPolicy> &value) : _value(value) {
}

template<typename StringPolicy>
Variant<StringPolicy>::Variant(Value<StringPolicy> &&value) : _value(xfwd(value)) {
}

template<typename StringPolicy>
Variant<StringPolicy>::Variant(const XValueConvertible<StringPolicy> auto &value) : _value {Value {value}} {
}

template<typename StringPolicy>
Variant<StringPolicy>::Variant(XValueConvertible<StringPolicy> auto &&value) : _value {Value {xfwd(value)}} {
}

template<typename StringPolicy>
Variant<StringPolicy> &Variant<StringPolicy>::operator!() {
  get<Value<StringPolicy>>() = !get<Value<StringPolicy>>();
  return *this;
}

// in/decrement operators
template<typename StringPolicy>
auto &Variant<StringPolicy>::operator++() {
  return get<Value<StringPolicy>>().operator++();
}

template<typename StringPolicy>
auto Variant<StringPolicy>::operator++(int) {
  return get<Value<StringPolicy>>().operator++(0);
}

template<typename StringPolicy>
auto &Variant<StringPolicy>::operator--() {
  return get<Value<StringPolicy>>().operator--();
}

template<typename StringPolicy>
auto Variant<StringPolicy>::operator--(int) {
  return get<Value<StringPolicy>>().operator--(0);
}

// -----------------------------------------
// Function API
//

template<typename StringPolicy>
Variant<StringPolicy>::Variant(Function<StringPolicy> &&value) : _value {std::move(value)} {
}

template<typename StringPolicy>
Variant<StringPolicy>::Variant(const Function<StringPolicy> &value) : _value {value} {
}

template<typename StringPolicy>
auto Variant<StringPolicy>::operator()() {
  return get<Function>()();
}

template<typename StringPolicy>
template<typename FirstT, typename... RestT>
auto Variant<StringPolicy>::operator()(FirstT &&first, RestT &&... rest) {
  return get<Function<StringPolicy>>()(xfwd(first), xfwd(rest)...);
}

template<typename StringPolicy>
auto Variant<StringPolicy>::apply(List<StringPolicy> &&args) {
  return get<Function<StringPolicy>>()(xfwd(args));
}

template<typename StringPolicy>
auto Variant<StringPolicy>::apply(const List<StringPolicy> &args) {
  return get<Function<StringPolicy>>()(args);
}

// -----------------------------------------
// Dict API
//

template<typename StringPolicy>
Variant<StringPolicy>::Variant(Dict<StringPolicy> &&value) : _value {std::move(value)} {
}

template<typename StringPolicy>
Variant<StringPolicy>::Variant(const Dict<StringPolicy> &value) : _value {value} {
}

template<typename StringPolicy>
Variant<StringPolicy> &Variant<StringPolicy>::update(Dict<StringPolicy> &&dct) {
  get<Dict<StringPolicy>>().update(xfwd(dct));
  return *this;
}

// -----------------------------------------
// List API
//

template<typename StringPolicy>
Variant<StringPolicy>::Variant(List<StringPolicy> &&value) : _value {std::move(value)} {
}

template<typename StringPolicy>
Variant<StringPolicy>::Variant(const List<StringPolicy> &value) : _value {value} {
}

// -----------------------------------------
// Dict/List/Object API
//

template<typename StringPolicy>
Variant<StringPolicy> &Variant<StringPolicy>::operator[](const Value<StringPolicy> &index) {
  return visit(tools::overloaded {
    [&index](List<StringPolicy> &lst) -> Variant & { return lst[static_cast<size_t>(index.template get<int>())]; },
    [&index](Dict<StringPolicy> &dct) -> Variant & { return dct[index]; },
    //        [&index](SharedObject& obj) -> Variant& {
    //            auto name = index.get<std::string>();
    //            return obj->prop(name);
    //        },
    [&index, this](None &) -> Variant & {
      // none promoted to dict
      _value = Dict<StringPolicy> {};
      return get<Dict<StringPolicy>>()[index];
    },
    [this](auto &) -> Variant & {
      spdlog::info("{:f}", *this);
      throw std::bad_cast();
    }});
}

template<typename StringPolicy>
const Variant<StringPolicy> &Variant<StringPolicy>::operator[](const Value<StringPolicy> &index) const {
  return std::visit(tools::overloaded {[&index](const List<StringPolicy> &lst) -> const Variant & {
                                         return lst[static_cast<size_t>(index.template get<int>())];
                                       },
                      [&index](const Dict<StringPolicy> &dct) -> const Variant & { return dct.at(index); },
                      [](const auto &) -> const Variant & { throw std::bad_cast(); }},
    _value);
}


template <typename StringPolicy>
Variant<StringPolicy> Variant<StringPolicy>::FromJSON(const std::string& input) {
  return json::parse(input);
}

template <typename StringPolicy>
Variant<StringPolicy> Variant<StringPolicy>::FromYAML(const std::string& input) {
  return yaml::parse(input);
}

} // xdev::variant
