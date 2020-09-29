#include <fmt/format.h>
#include <xdev/tools.hpp>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#include <ctti/nameof.hpp>
#pragma GCC diagnostic pop

#include <concepts>
#include <type_traits>

namespace xdev::variant {

  template <typename StringPolicy>
  constexpr Value<StringPolicy>::Value() noexcept
      : _value(None{}) {}

  template <typename StringPolicy>
  constexpr Value<StringPolicy>::Value(const Value &other) noexcept
      : _value(other._value) {}

  template <typename StringPolicy>
  constexpr Value<StringPolicy> &Value<StringPolicy>::operator=(const Value &other) noexcept {
    _value = other._value;
    return *this;
  }

  template <typename StringPolicy>
  constexpr Value<StringPolicy>::Value(Value &&other) noexcept
      : _value(std::move(other._value)) {
    other._value = None{};
  }

  template <typename StringPolicy>
  constexpr Value<StringPolicy> &Value<StringPolicy>::operator=(Value &&other) noexcept {
    _value       = std::move(other._value);
    other._value = None{};
    return *this;
  }

  template <typename StringPolicy>
  constexpr Value<StringPolicy>::Value(const XValueConvertible<StringPolicy> auto &value) requires(
    not decays_to<decltype(value), std::string_view>)
      : _value(value) {}

  template <typename StringPolicy>
  constexpr Value<StringPolicy>::Value(XValueConvertible<StringPolicy> auto &&value) requires(
    not decays_to<decltype(value), std::string_view>)
      : _value(xfwd(value)) {}

  template <typename StringPolicy>
  constexpr Value<StringPolicy>::Value(std::string_view value) noexcept
      : _value{string_type{value}} {}

  template <typename StringPolicy>
  constexpr std::string_view Value<StringPolicy>::typeName() const {
    return std::visit(
      []<typename T>(T &&) {
        using TClean = std::decay_t<T>;
        auto name    = ctti::nameof<TClean>();
        return std::string_view{std::begin(name), std::end(name)};
      },
      _value);
  }

  template <typename StringPolicy>
  constexpr Value<StringPolicy> &Value<StringPolicy>::operator!() {
    std::visit(tools::overloaded{[this]<typename TInput>(TInput &) {
                                   throw std::runtime_error(
                                     fmt::format("{} values cannot be negated", ctti::nameof<TInput>().cppstring()));
                                 },
                                 [](bool &value) { value = !value; }, [](int &value) { value = !value; },
                                 [](double &value) { value = !value; }},
               _value);
    return *this;
  }

  template <typename StringPolicy>
  constexpr bool Value<StringPolicy>::operator==(const Value &b) const {
    return tools::visit_2way(tools::overloaded{[]<typename T>(const T &lhs, const T &rhs) { return lhs == rhs; },
                                               []<typename T, typename U>(const T &, const U &) { return false; },
                                               [](const string_type &lhs, const string_type &rhs) {
                                                 return lhs.size() == rhs.size() &&
                                                        std::strcmp(lhs.data(), rhs.data()) == 0;
                                               }},
                             _value, b._value);
  }

  template <typename StringPolicy>
  constexpr std::weak_ordering Value<StringPolicy>::operator<=>(const Value &b) const {
    return tools::visit_2way(
      tools::overloaded{
        []<typename T>(const T &lhs, const T &rhs) {
          auto cmp = lhs <=> rhs;
          if (cmp == std::partial_ordering::unordered) return std::strong_ordering::less;
          else if (cmp == std::partial_ordering::greater)
            return std::strong_ordering::greater;
          else if (cmp == std::partial_ordering::less)
            return std::strong_ordering::less;
          else
            return std::strong_ordering::equal;
        },
        []<typename T, typename U>(const T &, const U &) { return std::strong_ordering::less; },
        []<typename T>(const T &, const string_type &) { return std::strong_ordering::less; },
        []<typename T>(const string_type &, const T &) { return std::strong_ordering::greater; },
        [](const string_type &lhs, const string_type &rhs) { return std::strcmp(lhs.data(), rhs.data()) <=> 0; }},
      _value, b._value);
  }

  template <typename StringPolicy>
  constexpr bool Value<StringPolicy>::operator==(char const *b) const {
    return std::visit(tools::overloaded{[]<typename T>(const T &) { return false; },
                                        [b](const string_type &lhs) { return std::strcmp(lhs.data(), b) == 0; }},
                      _value);
  }

  template <typename StringPolicy>
  constexpr std::weak_ordering Value<StringPolicy>::operator<=>(const char *b) const {
    return std::visit(
      tools::overloaded{[]<typename T>(const T &) { return std::strong_ordering::less; },
                        // gcc bug ? 0-compared values interpreted as char*
                        [b](const bool &val) { return static_cast<size_t>(val) <=> reinterpret_cast<size_t>(b); },
                        [b](const int &val) { return static_cast<size_t>(val) <=> reinterpret_cast<size_t>(b); },
                        [b](const string_type &lhs) { return std::strcmp(lhs.data(), b) <=> 0; }},
      _value);
  }

  // in/decrement operators

  template <typename StringPolicy>
  constexpr auto &Value<StringPolicy>::operator++() {
    std::visit(
      []<typename InputT>(InputT &&item) {
        using T = std::decay_t<InputT>;
        if constexpr (std::integral<T> && !std::same_as<T, bool>) {
          ++item;
        } else {
          throw std::bad_cast{};
        }
      },
      _value);
    return *this;
  }

  template <typename StringPolicy>
  constexpr auto Value<StringPolicy>::operator++(int) {
    Value prev = static_cast<const Value &>(*this);
    std::visit(
      []<typename InputT>(InputT &&item) {
        using T = std::decay_t<InputT>;
        if constexpr (std::integral<T> && !std::same_as<T, bool>) {
          ++item;
        } else {
          throw std::bad_cast{};
        }
      },
      _value);
    return prev;
  }

  template <typename StringPolicy>
  constexpr auto &Value<StringPolicy>::operator--() {
    std::visit(
      []<typename InputT>(InputT &&item) {
        using T = std::decay_t<InputT>;
        if constexpr (std::integral<T> && !std::same_as<T, bool>) {
          --item;
        } else {
          throw std::bad_cast{};
        }
      },
      _value);
    return *this;
  }

  template <typename StringPolicy>
  constexpr auto Value<StringPolicy>::operator--(int) {
    Value prev = static_cast<const Value &>(*this);
    std::visit(
      []<typename InputT>(InputT &&item) {
        using T = std::decay_t<InputT>;
        if constexpr (std::integral<T> && !std::same_as<T, bool>) {
          --item;
        } else {
          throw std::bad_cast{};
        }
      },
      _value);
    return prev;
  }

  template <typename StringPolicy>
  template <typename T>
  constexpr T &Value<StringPolicy>::get() {
    return std::get<T>(_value);
  }

  template <typename StringPolicy>
  template <typename T>
  constexpr const T &Value<StringPolicy>::get() const {
    return std::get<T>(_value);
  }

  template <typename StringPolicy>
  template <typename T>
  constexpr bool Value<StringPolicy>::is() const {
    return std::visit([]<typename ValueT>(ValueT &&) { return std::same_as<T, std::decay_t<ValueT>>; }, _value);
  }

  template <typename StringPolicy>
  template <typename Visitor>
  constexpr decltype(auto) Value<StringPolicy>::visit(Visitor &&visitor) {
    return std::visit(std::forward<Visitor>(visitor), _value);
  }

  template <typename StringPolicy>
  template <typename Visitor>
  constexpr decltype(auto) Value<StringPolicy>::visit(Visitor &&visitor) const {
    return std::visit(std::forward<Visitor>(visitor), _value);
  }

  template <typename StringPolicy>
  std::string Value<StringPolicy>::toString() const {
    return visit([]<typename InputT>(InputT &&value) -> std::string {
      using T = std::decay_t<InputT>;
      if constexpr (std::same_as<T, bool>) return value ? "true" : "false";
      else if constexpr (std::same_as<T, None>)
        return "none";
      else if constexpr (std::same_as<T, string_type>)
        return value;
      else if constexpr (has_to_string<T>::value)
        return to_string(value);
      else if constexpr (has_std_to_string<T>::value)
        return std::to_string(value);
      else
        static_assert(always_false<T>::value, "Missing toString converter");
    });
  }

}// namespace xdev::variant
