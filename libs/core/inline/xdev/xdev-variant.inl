#include <xdev/xdev-variant.hpp>
#include <xdev/xdev-tools.hpp>
//#include <xdev/xdev-object.hpp>
#include <xdev/xdev-typetraits.hpp>

#include <boost/type_index.hpp>

#include <fmt/format.h>
#include <gsl/gsl-lite.hpp>

#include <algorithm>

namespace std {

//template <>
//struct hash<xdev::variant::Variant>
//{
//    std::size_t operator()(const xdev::variant::Variant& var) const
//    {
//        return var.hash();
//    }
//};

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

Variant::Variant(): _value{Value{}} {}

//template<typename...Ts>
//requires std::constructible_from<List, Ts...>
//Variant::Variant(Ts&&...values): _value{List{std::forward<Ts>(values)...}} {}

Variant::Variant(const Variant&other): _value(other._value) {}
Variant& Variant::operator=(const Variant&other) { _value = other._value; return *this; }
Variant::Variant(Variant&&other): _value(std::move(other._value)) { other._value = Value{}; }
Variant& Variant::operator=(Variant&&other) { _value = std::move(other._value); other._value = Value{}; return *this; }

bool Variant::operator==(const Variant& b) const {
    return tools::visit_2way(tools::overloaded{
        []<typename T>(const T& lhs, const T& rhs){
            return lhs == rhs;
        },
        []<typename T>
        requires (std::convertible_to<T, Value>)
        (const Value& lhs, const T& rhs){
            return lhs == rhs;
        },
        []<typename T, typename U>(const T&lhs, const U&rhs){
            return false;
        }
    }, _value, b._value);
}

std::weak_ordering Variant::operator<=>(const Variant& b) const {
    return tools::visit_2way(tools::overloaded{
        []<typename T, typename U>(const T&lhs, const U&rhs){
            return lhs <=> rhs;
        },
        []<typename T>(const T&, const std::string&){
            return std::weak_ordering::less;
        },
        []<typename T>(const std::string&, const T&){
            return std::weak_ordering::less;
        },
        [](const std::string& lhs, const std::string& rhs){
            return std::strcmp(lhs.c_str(), rhs.c_str()) <=> 0;
        },
        [](const List&lhs, const List&rhs){
            if (lhs == rhs) {
                return std::weak_ordering::equivalent;
            }
            return std::weak_ordering::less;
        },
        []<typename T>(const List&, const T&){
            return std::weak_ordering::less;
        },
        []<typename T>(const T&, const List&){
            return std::weak_ordering::less;
        },
    }, _value, b._value);
}

bool Variant::operator==(char const* b) const {
    return std::visit(tools::overloaded{
        []<typename T>(const T&){
            return false;
        },
        [b](const Value& lhs){
            return lhs == b;
        }
    }, _value);
}

std::weak_ordering Variant::operator<=>(const char* b) const {
    return std::visit(tools::overloaded{
       []<typename T>(const T&){
          return std::weak_ordering::less;
       },
       [b](const Value& lhs){
          return lhs <=> 0;
       }
    }, _value);
}

template<typename T>
T& Variant::get() {
    if constexpr (is_one_of_v<T, Value, List>)//, Dict, Function, ObjectPtr>)
        return std::get<T>(_value);
    else return std::get<Value>(_value).get<T>();
}

template<typename T>
const T& Variant::get() const {
    if constexpr (is_one_of_v<T, Value, List>)//, Dict, Function, ObjectPtr>)
        return std::get<T>(_value);
    else return std::get<Value>(_value).get<T>();
}

//template<XObjectDerived ObjectT>
//typename ObjectT::ptr Variant::get() {
//    return std::get<XObjectBase::ptr>(_value)->cast<ObjectT>();
//}

//template<XObjectDerived ObjectT>
//typename ObjectT::ptr Variant::get() const {
//    return std::get<XObjectBase::ptr>(_value)->cast<ObjectT>();
//}

template<typename T>
bool Variant::is() const {
    return std::visit([&]<typename TInput>(TInput&&){
        using TVal = std::decay_t<TInput>;
        // check Value, List, Dict types
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
        if constexpr (is_one_of<T, Value, List>/*, Dict>*/::value)
            return value.toString();
        //else if constexpr (is_same_v<T, ObjectPtr>)
        //    return fmt::format("{}[0x{}]", value->objectName(), static_cast<void*>(value.get()));
        else throw std::runtime_error("Shall not append");
    });
}

bool Variant::empty() const {
    return is<None>();
}

//size_t Variant::hash() const {
//    return std::hash<value_t>{}(_value);
//}

// ---------------------------------
// Value API
//

Variant::Variant(const char* data): _value{Value{data}} {}

Variant::Variant(Value&&value): _value(std::move(value)) {}

template <typename T>
requires std::convertible_to<T, Value>
Variant::Variant(T&&value): _value{Value{std::forward<T>(value)}} {
}

//Variant& Variant::operator=(Value&& value) {
//    _value = std::forward<Value>(value);
//    return *this;
//}


Variant& Variant::operator!() {
    gsl_Expects(is<Value>());
    get<Value>() = !get<Value>();
    return *this;
}

// in/decrement operators

Value& Variant::operator++() {
    gsl_Expects(is<Value>());
    return get<Value>().operator++();
}

Value Variant::operator++(int) {
    gsl_Expects(is<Value>());
    return get<Value>().operator++(0);
}

Value& Variant::operator--() {
    gsl_Expects(is<Value>());
    return get<Value>().operator--();
}

Value Variant::operator--(int) {
    gsl_Expects(is<Value>());
    return get<Value>().operator--(0);
}

// -----------------------------------------
// List API
//

Variant::Variant(List&&value): _value(std::move(value)) {}

Variant& Variant::operator[](size_t index) {
    gsl_Expects(is<List>());
    return get<List>()[index];
}

const Variant& Variant::operator[](size_t index) const {
    gsl_Expects(is<List>());
    return get<List>()[index];
}

} // xdev::variant
