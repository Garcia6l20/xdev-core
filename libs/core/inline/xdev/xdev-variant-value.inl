#include <xdev/xdev-variant.hpp>

#include <type_traits>

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

Value::Value(): _value(None{}) {}

Value::Value(const Value&other): _value(other._value) {}
Value& Value::operator=(const Value&other) { _value = other._value; return *this; }
Value::Value(Value&&other): _value(std::move(other._value)) { other._value = XNone{}; }
Value& Value::operator=(Value&&other) { _value = std::move(other._value); other._value = XNone{}; return *this; }

template<typename T>
    requires !std::same_as<Value, T>
Value::Value(const T&value): _value(value) {}

template<typename T>
    requires !std::same_as<Value, T>
Value& Value::operator=(const T&value) { _value = value; return *this; }

template<typename T>
    requires !std::same_as<Value, T>
Value::Value(T&&value): _value(std::forward<T>(value)) { }

template<typename T>
    requires !std::same_as<Value, T>
Value& Value::operator=(T&&value) { _value = std::forward<T>(value); return *this; }

Value::Value(const char* value): _value(std::string(value)) {}

Value& Value::operator=(const char* value) { _value = std::string(value); return *this; }

template <typename T>
bool Value::operator==(const T& value) {
    return is<T>() ? get<T>() == value : false;
}

bool Value::operator==(const Value& value) {
    return hash() == value.hash();
}

bool Value::operator==(const char* value) {
    return is<std::string>() ? get<std::string>().compare(value) == 0 : false;
}

template <typename T>
bool Value::operator!=(const T& value) {
    return is<T>() ? get<T>() != value : true;
}

bool Value::operator!=(const Value& value) {
    return hash() != value.hash();
}

bool Value::operator!=(const char* value) {
    return is<std::string>() ? get<std::string>().compare(value) != 0 : true;
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
    return std::visit([](auto&&value){
        return std::is_same<T, std::decay_t<decltype(value)>>::value;
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

bool Value::operator<(const Value& other) const {
    return visit([&other](auto&&value) -> bool {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same<T, None>::value) {
            return true;
        } else return value < other.get<T>();
    });
}

bool Value::operator>(const Value& other) const {
    return visit([&other](auto&&value) -> bool {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same<T, None>::value) {
            return false;
        } else return value > other.get<T>();
    });
}

bool Value::operator<=(const Value& other) const {
    return visit([&other](auto&&value) -> bool {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same<T, None>::value) {
            return true;
        } else return value <= other.get<T>();
    });
}

bool Value::operator>=(const Value& other) const {
    return visit([&other](auto&&value) -> bool {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same<T, None>::value) {
            return false;
        } else return value >= other.get<T>();
    });
}

std::string Value::toString() const {
    return visit([](auto&&value) -> std::string {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same<T, bool>::value)
            return value ? "true" : "false";
        else if constexpr (std::is_same<T, None>::value)
            return "none";
        else if constexpr (std::is_same<T, std::string>::value)
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

} // variant
} // xdev
