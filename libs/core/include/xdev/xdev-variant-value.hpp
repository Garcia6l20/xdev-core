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

struct None: std::monostate {};

class Value {
public:

    inline Value();

    inline Value(const Value&other);
    inline Value& operator=(const Value&other);
    inline Value(Value&&other);
    inline Value& operator=(Value&&other);

    template<typename T>
        requires !std::same_as<Value, T>
    inline Value(const T&value);

    template<typename T>
        requires !std::same_as<Value, T>
    inline Value& operator=(const T&value);

    template<typename T>
        requires !std::same_as<Value, T>
    inline Value(T&&value);

    template<typename T>
        requires !std::same_as<Value, T>
    inline Value& operator=(T&&value);

    inline Value(const char* value);
    inline Value& operator=(const char* value);

    template <typename T>
    inline bool operator==(const T& value);

    inline bool operator==(const Value& value);
    inline bool operator==(const char* value);

    template <typename T>
    inline bool operator!=(const T& value);

    inline bool operator!=(const Value& value);
    inline bool operator!=(const char* value);

    inline bool operator<(const Value& other) const;
    inline bool operator>(const Value& other) const;
    inline bool operator<=(const Value& other) const;
    inline bool operator>=(const Value& other) const;

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
