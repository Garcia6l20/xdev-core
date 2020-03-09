/**
 * @file xdev-variant-value.hpp
 */
#pragma once

#include <xdev/xdev-concepts.hpp>

#include <variant>
#include <string>

namespace xdev {

class XObjectBase;

namespace variant {

struct None: std::monostate {
    auto operator==(const None&) const {return false;};
};

class Variant;
class List;
class Dict;
class Function;

using SharedObject = std::shared_ptr<XObjectBase>;

class Value {
public:

    inline Value() noexcept;


    inline Value(const char* value) noexcept;
    inline Value& operator=(const char* value) noexcept;

    inline Value(const Value&other) noexcept;
    inline Value& operator=(const Value&other) noexcept;

    inline Value(Value&&other) noexcept;
    inline Value& operator=(Value&&other) noexcept;

    inline Value(const XValueConvertible auto&value);
    inline Value(XValueConvertible auto&&value);

    inline Value& operator!();

    inline bool operator==(const Value& rhs) const;
    inline std::weak_ordering operator<=>(const Value& rhs) const;
    inline bool operator==(char const* rhs) const;
    inline std::weak_ordering operator<=>(const char* rhs) const;

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

    inline std::string typeName() const;

    static constexpr const char* ctti_nameof() {
        return "xval";
    }

private:
    using value_t = std::variant<None, bool, int, double, std::string>;
    value_t _value;
};

} // namspace variant
} // namspace xdev
