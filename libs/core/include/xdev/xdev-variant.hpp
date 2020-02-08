/**
 * @file xdev-variant.hpp
 * @brief The Ultimate variant class
 **/
#pragma once

#include <variant>
#include <memory>
#include <vector>
#include <list>
#include <string>
#include <map>
#include <stdint.h>

#include <iostream>

#include <xdev/xdev-core-export.hpp>

#include <xdev/xdev-typetraits.hpp>
#include <xdev/xdev-concepts.hpp>

#include <any>

#include <xdev/xdev-variant-value.hpp>
#include <xdev/xdev-variant-array.hpp>
#include <xdev/xdev-variant-dict.hpp>
#include <xdev/xdev-variant-function.hpp>

namespace xdev {

class XObjectBase;

namespace variant {

class Dict;
class Array;
class Variant;
using ObjectPtr = std::shared_ptr<XObjectBase>;

class Variant {
public:
    inline Variant();
    inline Variant(Value&&value);

    inline Variant(Variant&&other);
    inline Variant& operator=(Variant&&other);
    inline Variant(const Variant&other);
    inline Variant& operator=(const Variant&other);

    template<typename T,
             typename = std::enable_if_t<!std::is_same_v<std::decay_t<T>, Variant> /*&&
                                         !std::is_convertible_v<std::decay_t<T>, ObjectPtr>*/
                                         >
             >
    inline Variant(const T&value);

    template<typename T,
             typename = std::enable_if_t<!std::is_same_v<std::decay_t<T>, Variant> /*&&
                                         !std::is_convertible_v<std::decay_t<T>, ObjectPtr>*/
                                         >
             >
    inline Variant& operator=(const T&value);

    template<typename T,
             typename = std::enable_if_t<!std::is_same_v<std::decay_t<T>, Variant> /*&&
                                         !std::is_convertible_v<std::decay_t<T>, ObjectPtr>*/
                                         >
             >
    inline Variant(T&&value);

    template<typename T,
             typename = std::enable_if_t<!std::is_same_v<std::decay_t<T>, Variant> /*&&
                                         !std::is_convertible_v<std::decay_t<T>, ObjectPtr>*/
                                         >
             >
    inline Variant& operator=(T&&value);

    inline bool operator==(const Variant& other) const;
    inline bool operator!=(const Variant& other) const;
    inline bool operator<(const Variant& other) const;
    inline bool operator>(const Variant& other) const;

    /**
     * @brief Get value as @c T.
     */
    template<typename T>
    T& get();

    /**
     * @brief Get value as @c T (const).
     */
    template<typename T>
    const T& get() const;

    /**
     * @brief Get value as @c XObjectBase subclass.
     */
    template<XObjectDerived ObjectT>
    typename ObjectT::ptr get();

    /**
     * @brief Get value as @c XObjectBase subclass (const).
     */
    template<XObjectDerived ObjectT>
    typename ObjectT::ptr get() const;

    struct ConvertError : public std::runtime_error { using std::runtime_error::runtime_error; };

    template<typename T>
    bool is() const;

    template<typename T>
    T convert() const;

    inline size_t hash() const;

    template <bool inner = true, typename Visitor>
    decltype(auto) visit(Visitor&&visitor);

    template <bool inner = true, typename Visitor>
    decltype(auto) visit(Visitor&&visitor) const;

    template <bool inner = true>
    inline std::string typeName() const;

    inline std::string toString() const;

    inline bool empty() const;

    static XDEV_CORE_EXPORT Variant FromJSON(const std::string&);

    static constexpr const char* ctti_nameof()
    {
        return "XVariant";
    }

private:
    using value_t = std::variant<Value, Array, Dict, Function, ObjectPtr>;
    value_t _value;
};

} // variant

using XNone     = variant::None;
using XValue    = variant::Value;
using XArray    = variant::Array;
using XDict     = variant::Dict;
using XFunction = variant::Function;
using XVariant  = variant::Variant;

} // xdev

#include <xdev/xdev-variant-value.inl>
#include <xdev/xdev-variant-array.inl>
#include <xdev/xdev-variant-dict.inl>
#include <xdev/xdev-variant-function.inl>
#include <xdev/xdev-variant.inl>
