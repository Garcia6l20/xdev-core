/**
 * @file xdev-variant-value.hpp
 */
#pragma once

#ifdef __cpp_lib_concepts
#include <concepts>
#else
#include <xdev/std-concepts.hpp>
#endif

#include <variant>
#include <string>

namespace xdev::variant {

struct None: std::monostate {
    auto operator==(const None&) const {return false;};
};

class Variant;
class List;
class Dict;
class Function;

class Value {
public:

    inline Value() noexcept;


    inline Value(const char* value) noexcept;
    inline Value& operator=(const char* value) noexcept;

    inline Value(const Value&other) noexcept;
    inline Value& operator=(const Value&other) noexcept;

    inline Value(Value&&other) noexcept;
    inline Value& operator=(Value&&other) noexcept;

    template<typename T>
        requires (not one_of<std::decay_t<T>, Value, Variant, List, Dict, Function>)
    inline Value(const T&value);

    template<typename T>
        requires (not one_of<std::decay_t<T>, Value, Variant, List, Dict, Function>)
    inline Value& operator=(const T&value);

    template<typename T>
        requires (not one_of<std::decay_t<T>, Value, Variant, List, Dict, Function>)
    inline Value(T&&value);

    template<typename T>
        requires (not one_of<std::decay_t<T>, Value, Variant, List, Dict, Function>)
    inline Value& operator=(T&&value);



    inline Value& operator!();

//     std::weak_ordering operator<=>(const Value&) const;

    inline bool operator==(const Value& rhs) const;
    inline std::weak_ordering operator<=>(const Value& rhs) const;
    inline bool operator==(char const* rhs) const;
    inline std::weak_ordering operator<=>(const char* rhs) const;

//    template <typename T>
//    inline bool operator==(const T& value) const;

//    inline bool operator==(const Value& value) const;
//    inline bool operator==(const char* value) const;

//    template <typename T>
//    inline bool operator!=(const T& value) const;

//    inline bool operator!=(const Value& value) const;
//    inline bool operator!=(const char* value) const;

//    inline bool operator<(const Value& other) const;
//    inline bool operator>(const Value& other) const;
//    inline bool operator<=(const Value& other) const;
//    inline bool operator>=(const Value& other) const;

    inline Value& operator++();
    inline Value operator++(int);

    inline Value& operator--();
    inline Value operator--(int);

    template <typename T>
    inline T& get();

    template <typename T>
    inline const T& get() const;

    template<typename T>
    inline bool is() const;

    inline size_t hash() const;

    template <typename Visitor>
    inline decltype(auto) visit(Visitor&&visitor);

    template <typename Visitor>
    inline decltype(auto) visit(Visitor&&visitor) const;

    inline std::string toString() const;

    static constexpr const char* ctti_nameof()
    {
        return "XValue";
    }

private:
    using value_t = std::variant<None, bool, int, double, std::string>;
    value_t _value;
};

}
