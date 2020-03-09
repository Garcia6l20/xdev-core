//#include <xdev/xdev-variant.hpp>
#include <xdev/xdev-tools.hpp>
#include <fmt/format.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#include <ctti/nameof.hpp>
#pragma GCC diagnostic pop

#include <type_traits>
#include <concepts>

namespace std {

template <>
struct hash<xdev::variant::None>
{
    inline std::size_t operator()(const xdev::variant::None& var) const
    {
        return hash<monostate>{}(var);
    }
};

template <>
struct hash<xdev::variant::Value>
{
    std::size_t operator()(const xdev::variant::Value& var) const
    {
        return var.hash();
    }
};

} // std

namespace xdev {
namespace variant {

Value::Value() noexcept: _value(None{}) {}

Value::Value(const Value&other) noexcept: _value(other._value) {}
Value& Value::operator=(const Value&other) noexcept { _value = other._value; return *this; }
Value::Value(Value&&other) noexcept: _value(std::move(other._value)) { other._value = None{}; }
Value& Value::operator=(Value&&other) noexcept { _value = std::move(other._value); other._value = None{}; return *this; }

Value::Value(const XValueConvertible auto&value): _value(value) {}
Value::Value(XValueConvertible auto&&value): _value(xfwd(value)) { }

Value::Value(const char* value) noexcept: _value(std::string(value)) {}

Value& Value::operator=(const char* value) noexcept { _value = std::string(value); return *this; }

std::string Value::typeName() const {
    return std::visit([]<typename T>(T&&) -> std::string {
        using TClean = std::decay_t<T>;
        return ctti::nameof<TClean>().str();
    }, _value);
}

Value& Value::operator!() {
    std::visit(tools::overloaded{
       [this]<typename TInput>(TInput&) {
           throw std::runtime_error(fmt::format("{} values cannot be negated", ctti::nameof<TInput>().cppstring()));
       },
       [](bool& value) {
           value = !value;
       },
       [](int& value) {
           value = !value;
       },
       [](double& value) {
           value = !value;
       }
    }, _value);
    return *this;
}

bool Value::operator==(const Value& b) const {
    return tools::visit_2way(tools::overloaded{
       []<typename T>(const T& lhs, const T& rhs){
           return lhs == rhs;
       },
       []<typename T, typename U>(const T&, const U&){
           return false;
       },
       [](const std::string& lhs, const std::string& rhs){
          return lhs.size() == rhs.size() &&
            std::strcmp(lhs.c_str(), rhs.c_str()) == 0;
       }
    }, _value, b._value);
}

std::weak_ordering Value::operator<=>(const Value& b) const {
    return tools::visit_2way(tools::overloaded{
         []<typename T>(const T&lhs, const T&rhs){
             auto cmp = lhs <=> rhs;
            if (cmp == std::partial_ordering::unordered)
                return std::strong_ordering::less;
            else if (cmp == std::partial_ordering::greater)
                return std::strong_ordering::greater;
            else if (cmp == std::partial_ordering::less)
                return std::strong_ordering::less;
            else return std::strong_ordering::equal;
         },
       []<typename T, typename U>(const T&, const U&){
           return std::strong_ordering::less;
       },
      []<typename T>(const T&, const std::string&){
          return std::strong_ordering::less;
      },
      []<typename T>(const std::string&, const T&){
          return std::strong_ordering::greater;
      },
       [](const std::string& lhs, const std::string& rhs){
          return std::strcmp(lhs.c_str(), rhs.c_str()) <=> 0;
       }
    }, _value, b._value);
}

bool Value::operator==(char const* b) const {
    return std::visit(tools::overloaded{
       []<typename T>(const T&){
           return false;
       },
       [b](const std::string& lhs){
          return std::strcmp(lhs.c_str(), b) == 0;
       }
    }, _value);
}
std::weak_ordering Value::operator<=>(const char* b) const {
    return std::visit(tools::overloaded{
       []<typename T>(const T&){
          return std::strong_ordering::less;
       },
        // gcc bug ? 0-compared values interpreted as char*
      [b](const bool& val){
         return static_cast<size_t>(val) <=> reinterpret_cast<size_t>(b);
      },
      [b](const int& val){
         return static_cast<size_t>(val) <=> reinterpret_cast<size_t>(b);
      },
       [b](const std::string& lhs){
          return std::strcmp(lhs.c_str(), b) <=> 0;
       }
    }, _value);
}

//template <typename T>
//bool Value::operator==(const T& val) const {
//    if constexpr (std::same_as<T, Variant>)
//        return val.template get<Value>() == *this;
//    else return is<T>() ? get<T>() == val : false;
//}

//bool Value::operator==(const Value& value) const {
//    return hash() == value.hash();
//}

//bool Value::operator==(const char* value) const {
//    return is<std::string>() ? get<std::string>().compare(value) == 0 : false;
//}

//template <typename T>
//bool Value::operator!=(const T& value) const {
//    return is<T>() ? get<T>() != value : true;
//}

//bool Value::operator!=(const Value& value) const {
//    return hash() != value.hash();
//}

//bool Value::operator!=(const char* value) const {
//    return is<std::string>() ? get<std::string>().compare(value) != 0 : true;
//}


// in/decrement operators

Value& Value::operator++() {
    std::visit([]<typename InputT>(InputT&& item){
        using T = std::decay_t<InputT>;
        if constexpr (std::integral<T> && !std::same_as<T, bool>) {
            ++item;
        } else {
            throw std::bad_cast{};
        }
    }, _value);
    return *this;
}

Value Value::operator++(int) {
    Value prev = static_cast<const Value&>(*this);
    std::visit([]<typename InputT>(InputT&& item){
        using T = std::decay_t<InputT>;
        if constexpr (std::integral<T> && !std::same_as<T, bool>) {
            ++item;
        } else {
            throw std::bad_cast{};
        }
    }, _value);
    return prev;
}

Value& Value::operator--() {
    std::visit([]<typename InputT>(InputT&& item){
       using T = std::decay_t<InputT>;
        if constexpr (std::integral<T> && !std::same_as<T, bool>) {
            --item;
        } else {
            throw std::bad_cast{};
        }
    }, _value);
    return *this;
}

Value Value::operator--(int) {
    Value prev = static_cast<const Value&>(*this);
    std::visit([]<typename InputT>(InputT&& item){
        using T = std::decay_t<InputT>;
        if constexpr (std::integral<T> && !std::same_as<T, bool>) {
            --item;
        } else {
            throw std::bad_cast{};
        }
    }, _value);
    return prev;
}

template <typename T>
T& Value::get() {
    return std::get<T>(_value);
}

template <typename T>
const T& Value::get() const {
    return std::get<T>(_value);
}

template<typename T>
bool Value::is() const {
    return std::visit([]<typename ValueT>(ValueT&&){
        return std::same_as<T, std::decay_t<ValueT>>;
    }, _value);
}

template <typename Visitor>
decltype(auto) Value::visit(Visitor&&visitor) {
    return std::visit(std::forward<Visitor>(visitor), _value);
}

template <typename Visitor>
decltype(auto) Value::visit(Visitor&&visitor) const {
    return std::visit(std::forward<Visitor>(visitor), _value);
}

std::string Value::toString() const {
    return visit([]<typename InputT>(InputT&&value) -> std::string {
        using T = std::decay_t<InputT>;
        if constexpr (std::same_as<T, bool>)
            return value ? "true" : "false";
        else if constexpr (std::same_as<T, None>)
            return "none";
        else if constexpr (std::same_as<T, std::string>)
            return value;
        else if constexpr (has_to_string<T>::value)
            return to_string(value);
        else if constexpr (has_std_to_string<T>::value)
            return std::to_string(value);
        else static_assert (always_false<T>::value,
                           "Missing toString converter");
    });
}


size_t Value::hash() const {
    return std::hash<value_t>{}(_value);
}

//bool Value::operator<(const Value& other) const {
//    return visit([&other]<typename InputT>(InputT&&value) -> bool {
//         using T = std::decay_t<InputT>;
//        if constexpr (std::same_as<T, None>) {
//            return true;
//        } else return value < other.get<std::decay_t<T>>();
//    });
//}

//bool Value::operator>(const Value& other) const {
//    return visit([&other]<typename InputT>(InputT&&value) -> bool {
//         using T = std::decay_t<InputT>;
//        if constexpr (std::same_as<T, None>) {
//            return false;
//        } else return value > other.get<std::decay_t<T>>();
//    });
//}

//bool Value::operator<=(const Value& other) const {
//    return visit([&other]<typename InputT>(InputT&&value) -> bool {
//         using T = std::decay_t<InputT>;
//        if constexpr (std::same_as<T, None>) {
//            return true;
//        } else return value <= other.get<std::decay_t<T>>();
//    });
//}

//bool Value::operator>=(const Value& other) const {
//    return visit([&other]<typename InputT>(InputT&&value) -> bool {
//        using T = std::decay_t<InputT>;
//        if constexpr (std::same_as<T, None>) {
//            return false;
//        } else return value >= other.get<std::decay_t<T>>();
//    });
//}

//std::weak_ordering Value::operator<=>(const Value& other) const {
//    auto cmp = visit([&other]<typename InputT>(InputT&&value) {
//        using T = std::decay_t<InputT>;
//        if constexpr (std::same_as<T, None>) {
//            return std::weak_ordering::less;
//        } else return other.visit([&]<typename OtherInputT>(OtherInputT&& other_value) {
//            using OtherT = std::decay_t<OtherInputT>;
//            if constexpr (!std::same_as<T, OtherT>) {
//                return std::weak_ordering::less;
//            } else if constexpr (std::same_as<T, std::string>
//                              && std::same_as<OtherT, std::string>) {
//                return std::weak_ordering::less;
//            }  else {
//                return value <=> other_value;
//            }
//        });
//    });
//    return cmp;
//}

} // variant
} // xdev
