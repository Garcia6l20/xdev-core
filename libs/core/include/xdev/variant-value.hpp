/**
 * @file xdev-variant-value.hpp
 */
#pragma once

#include <xdev/concepts.hpp>

#include <string>
#include <variant>

namespace xdev {

  class XObjectBase;

  namespace variant {

    struct None : std::monostate {
      auto operator==(const None &) const { return false; };
    };

    template <typename StringPolicy>
    class Variant;

    template <typename StringPolicy>
    class List;

    template <typename StringPolicy>
    class Dict;

    template <typename StringPolicy>
    class Function;

    using SharedObject = std::shared_ptr<XObjectBase>;

    template <typename StringPolicy = struct StdStringPolicy>
    class Value {
    public:
      using string_type = typename StringPolicy::string_type;

      constexpr Value() noexcept;

      constexpr Value(const XValueConvertible<StringPolicy> auto &value) requires(
        not decays_to<decltype(value), std::string_view>);
      constexpr Value(XValueConvertible<StringPolicy> auto &&value) requires(
        not decays_to<decltype(value), std::string_view>);

      constexpr Value(std::string_view value) noexcept;

      constexpr Value(const Value &other) noexcept;
      constexpr Value &operator=(const Value &other) noexcept;

      constexpr Value(Value &&other) noexcept;
      constexpr Value &operator=(Value &&other) noexcept;

      constexpr bool               operator==(const Value &rhs) const;
      constexpr std::weak_ordering operator<=>(const Value &rhs) const;
      constexpr bool               operator==(char const *rhs) const;
      constexpr std::weak_ordering operator<=>(const char *rhs) const;

      constexpr Value &operator!();

      constexpr auto &operator++();
      constexpr auto  operator++(int);

      constexpr auto &operator--();
      constexpr auto  operator--(int);

      template <typename T>
      constexpr T &get();

      template <typename T>
      constexpr const T &get() const;

      template <typename T>
      [[nodiscard]] constexpr bool is() const;

      template <typename Visitor>
      constexpr decltype(auto) visit(Visitor &&visitor);

      template <typename Visitor>
      constexpr decltype(auto) visit(Visitor &&visitor) const;

      [[nodiscard]] inline std::string toString() const;

      [[nodiscard]] constexpr std::string_view typeName() const;

      static constexpr const char *ctti_nameof() { return "xval"; }

    private:
      using value_t = variant_type<StringPolicy>;
      value_t _value;
    };

  }// namespace variant
}// namespace xdev
