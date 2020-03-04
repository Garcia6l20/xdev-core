#include <xdev/xdev-variant.hpp>

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
Value::Value(Value&&other) noexcept: _value(std::move(other._value)) { other._value = XNone{}; }
Value& Value::operator=(Value&&other) noexcept { _value = std::move(other._value); other._value = XNone{}; return *this; }

template<typename T>
    requires (!std::same_as<Value, std::decay_t<T>>)
Value::Value(const T&value): _value(value) {}

template<typename T>
    requires (!std::same_as<Value, std::decay_t<T>>)
Value& Value::operator=(const T&value) { _value = value; return *this; }

template<typename T>
    requires (!std::same_as<Value, std::decay_t<T>>)
Value::Value(T&&value): _value(std::forward<T>(value)) { }

template<typename T>
    requires (!std::same_as<Value, std::decay_t<T>>)
Value& Value::operator=(T&&value) { _value = std::forward<T>(value); return *this; }

Value::Value(const char* value): _value(std::string(value)) {}

Value& Value::operator=(const char* value) { _value = std::string(value); return *this; }

template <typename T>
bool Value::operator==(const T& val) const {
    if constexpr (std::same_as<T, Variant>)
        return val.template get<Value>() == *this;
    else return is<T>() ? get<T>() == val : false;
}

bool Value::operator==(const Value& value) const {
    return hash() == value.hash();
}

bool Value::operator==(const char* value) const {
    return is<std::string>() ? get<std::string>().compare(value) == 0 : false;
}

template <typename T>
bool Value::operator!=(const T& value) const {
    return is<T>() ? get<T>() != value : true;
}

bool Value::operator!=(const Value& value) const {
    return hash() != value.hash();
}

bool Value::operator!=(const char* value) const {
    return is<std::string>() ? get<std::string>().compare(value) != 0 : true;
}


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

bool Value::operator<(const Value& other) const {
    return visit([&other]<typename InputT>(InputT&&value) -> bool {
         using T = std::decay_t<InputT>;
        if constexpr (std::same_as<T, None>) {
            return true;
        } else return value < other.get<std::decay_t<T>>();
    });
}

bool Value::operator>(const Value& other) const {
    return visit([&other]<typename InputT>(InputT&&value) -> bool {
         using T = std::decay_t<InputT>;
        if constexpr (std::same_as<T, None>) {
            return false;
        } else return value > other.get<std::decay_t<T>>();
    });
}

bool Value::operator<=(const Value& other) const {
    return visit([&other]<typename InputT>(InputT&&value) -> bool {
         using T = std::decay_t<InputT>;
        if constexpr (std::same_as<T, None>) {
            return true;
        } else return value <= other.get<std::decay_t<T>>();
    });
}

bool Value::operator>=(const Value& other) const {
    return visit([&other]<typename InputT>(InputT&&value) -> bool {
        using T = std::decay_t<InputT>;
        if constexpr (std::same_as<T, None>) {
            return false;
        } else return value >= other.get<std::decay_t<T>>();
    });
}

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
