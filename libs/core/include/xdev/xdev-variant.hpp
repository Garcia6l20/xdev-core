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
//#include <xdev/xdev-variant-dict.hpp>
//#include <xdev/xdev-variant-function.hpp>

namespace xdev {

class XObjectBase;

namespace variant {

//class Dict;
class List;
class Variant;
//using ObjectPtr = std::shared_ptr<XObjectBase>;

class Variant {
public:
    inline Variant();

    inline Variant(Variant&&other);
    inline Variant& operator=(Variant&&other);

    inline Variant(const Variant&other);
    inline Variant& operator=(const Variant&other);

    inline bool operator==(const Variant& b) const;
    inline std::weak_ordering operator<=>(const Variant& b) const;
    inline std::weak_ordering operator<=>(const char* b) const;

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

//    /**
//     * @brief Get value as @c XObjectBase subclass.
//     */
//    template<XObjectDerived ObjectT>
//    typename ObjectT::ptr get();

//    /**
//     * @brief Get value as @c XObjectBase subclass (const).
//     */
//    template<XObjectDerived ObjectT>
//    typename ObjectT::ptr get() const;

    struct ConvertError : public std::runtime_error { using std::runtime_error::runtime_error; };

    template<typename T>
    bool is() const;

    template<typename T>
    T convert() const;

//    inline size_t hash() const;

    template <bool inner = true, typename Visitor>
    decltype(auto) visit(Visitor&&visitor);

    template <bool inner = true, typename Visitor>
    decltype(auto) visit(Visitor&&visitor) const;

    template <bool inner = true>
    inline std::string typeName() const;

    inline std::string toString() const;

    inline bool empty() const;

    /**
     * @defgroup variant_list_api XVariant list API
     * @{
     */

    inline Variant(List&&value);
    inline Variant& operator[](size_t index);
    inline const Variant& operator[](size_t index) const;

    /** @} */



    /**
     * @defgroup variant_value_api XVariant value API
     * @{
     */

    inline Variant(const char*);
    inline Variant(Value&&);
    //inline Variant& operator=(Value&& value);

    template <typename T>
    requires std::convertible_to<T, Value>
    Variant(T&&);

    inline Variant& operator!();

    inline bool operator==(char const* b) const;

    // in/decrement operators

    inline Value& operator++();
    inline Value operator++(int);

    inline Value& operator--();
    inline Value operator--(int);

    /** @} */

    static XDEV_CORE_EXPORT Variant FromJSON(const std::string&);
    static XDEV_CORE_EXPORT Variant FromYAML(const std::string&);

    static constexpr const char* ctti_nameof()
    {
        return "XVariant";
    }
private:
    using value_t = std::variant<Value, List>;//, Dict, Function, ObjectPtr>;
    value_t _value;
};

} // variant

using XNone     = variant::None;
using XValue    = variant::Value;
using XList    = variant::List;
//using XDict     = variant::Dict;
//using XFunction = variant::Function;
using XVariant  = variant::Variant;

} // xdev

#include <xdev/xdev-variant-value.inl>
#include <xdev/xdev-variant-array.inl>
//#include <xdev/xdev-variant-dict.inl>
//#include <xdev/xdev-variant-function.inl>
#include <xdev/xdev-variant.inl>
