#include <xdev/xdev-variant.hpp>
#include <xdev/xdev-tools.hpp>
#include <xdev/xdev-object.hpp>
#include <xdev/xdev-typetraits.hpp>

#include <boost/type_index.hpp>

#include <fmt/format.h>

#include <algorithm>

namespace std {

template <>
struct hash<xdev::variant::Variant>
{
    std::size_t operator()(const xdev::variant::Variant& var) const
    {
        return var.hash();
    }
};

template <xdev::Stringable T>
static inline std::string to_string(const T& obj) {
    return obj.toString();
}

template <xdev::Stringable T>
static inline std::ostream& operator<<(std::ostream& stream, const T& obj) {
    stream << obj.toString();
    return stream;
}

}

namespace xdev::variant {

Variant::Variant(): _value(Value()) {}
Variant::Variant(Value&&value): _value(std::move(value)) {}

Variant::Variant(const Variant&other): _value(other._value) {}
Variant& Variant::operator=(const Variant&other) { _value = other._value; return *this; }
Variant::Variant(Variant&&other): _value(std::move(other._value)) { other._value = XNone{}; }
Variant& Variant::operator=(Variant&&other) { _value = std::move(other._value); other._value = XNone{}; return *this; }

template<typename T, typename>
Variant::Variant(T&&value) {
    if constexpr (std::is_convertible_v<T, ObjectPtr>)
        _value = std::dynamic_pointer_cast<XObjectBase>(std::forward<T>(value));
    else _value = std::forward<T>(value);
}

template<typename T, typename>
Variant& Variant::operator=(T&&value) {
    if constexpr (std::is_convertible_v<T, ObjectPtr>)
        _value = std::dynamic_pointer_cast<XObjectBase>(std::forward<T>(value));
    else _value = std::forward<T>(value);
    return *this;
}

template<typename T, typename>
Variant::Variant(const T&value) {
    if constexpr (std::is_convertible_v<T, ObjectPtr>)
        _value = std::dynamic_pointer_cast<XObjectBase>(value);
    else _value = value;
}

template<typename T, typename>
Variant& Variant::operator=(const T&value) {
    if constexpr (std::is_convertible_v<T, ObjectPtr>)
        _value = std::dynamic_pointer_cast<XObjectBase>(value);
    else _value = value;
    return *this;
}

//bool Variant::operator==(const Variant& other) const {
//    return hash() == other.hash();
//}

//bool Variant::operator!=(const Variant& other) const {
//    return hash() != other.hash();
//}

//bool Variant::operator<(const Variant& other) const {
//    if (is<Value>() && other.is<Value>())
//        return get<Value>() < other.get<Value>();
//    throw std::runtime_error("Comparaison only applyable on Value types");
//}

//template <typename T>
//bool operator<(const T& lhs, const Variant&rhs) {
//    return lhs < rhs.get<Value>();
//}

//bool Variant::operator>(const Variant& other) const {
//    if (is<Value>() && other.is<Value>())
//        return get<Value>() > other.get<Value>();
//    throw std::runtime_error("Comparaison only applyable on Value types");
//}

template<typename T>
T& Variant::get() {
    if constexpr (is_one_of_v<T, Value, Array, Dict, Function, ObjectPtr>)
        return std::get<T>(_value);
    else return std::get<Value>(_value).get<T>();
}

template<typename T>
const T& Variant::get() const {
    if constexpr (is_one_of_v<T, Value, Array, Dict, Function, ObjectPtr>)
        return std::get<T>(_value);
    else return std::get<Value>(_value).get<T>();
}

template<XObjectDerived ObjectT>
typename ObjectT::ptr Variant::get() {
    return std::get<XObjectBase::ptr>(_value)->cast<ObjectT>();
}

template<XObjectDerived ObjectT>
typename ObjectT::ptr Variant::get() const {
    return std::get<XObjectBase::ptr>(_value)->cast<ObjectT>();
}

template<typename T>
bool Variant::is() const {
    return std::visit([&]<typename TVal>(TVal&&){
        // check Value, Array, Dict types
        if constexpr (std::same_as<TVal, T>)
            return true;
        // check inner Value type
        else if constexpr (std::same_as<TVal, Value>)
            return get<Value>().is<T>();
        else return false;
    }, _value);
}

template<typename T>
T Variant::convert() const {
    return visit([&](auto&&value) -> T {
        using TVal = std::decay_t<decltype(value)>;
        if constexpr (std::is_convertible<TVal, T>::value) {
            return static_cast<T>(value);
        }
        throw ConvertError("Cannot apply requested convertion");
    });
}

template <bool inner>
std::string Variant::typeName() const {
    return visit<inner>([] (auto&&value) -> std::string {
        using T = std::decay_t<decltype(value)>;
        // return boost::typeindex::type_id<T>().pretty_name();
        return ctti::nameof<T>().str();
    });
}

template <bool inner, typename Visitor>
decltype(auto) Variant::visit(Visitor&&visitor) {
    if (is<Value>() && inner)
        return get<Value>().visit(std::forward<Visitor>(visitor));
    else return std::visit(std::forward<Visitor>(visitor), _value);
}

template <bool inner, typename Visitor>
decltype(auto) Variant::visit(Visitor&&visitor) const {
    if (is<Value>() && inner)
        return get<Value>().visit(std::forward<Visitor>(visitor));
    else return std::visit(std::forward<Visitor>(visitor), _value);
}

std::string Variant::toString() const {
    return visit<false>([](auto&&value) -> std::string {
        using T = std::decay_t<decltype(value)>;
        if constexpr (is_one_of<T, Value, Array, Dict>::value)
            return value.toString();
        else if constexpr (is_same_v<T, ObjectPtr>)
            return fmt::format("{}[0x{}]", value->objectName(), static_cast<void*>(value.get()));
        else throw std::runtime_error("Shall not append");
    });
}

bool Variant::empty() const {
    return is<None>();
}

size_t Variant::hash() const {
    return std::hash<value_t>{}(_value);
}

// in/decrement operators

Value& Variant::operator++() {
    return get<Value>().operator++();
}

Value Variant::operator++(int) {
    return get<Value>().operator++(0);
}

Value& Variant::operator--() {
    return get<Value>().operator--();
}

Value Variant::operator--(int) {
    return get<Value>().operator--(0);
}

} // xdev::variant
