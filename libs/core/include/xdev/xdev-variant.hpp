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
#include <xdev/xdev-variant-list.hpp>
#include <xdev/xdev-variant-dict.hpp>
#include <xdev/xdev-variant-function.hpp>

namespace xdev {

class XObjectBase;

namespace variant {

using SharedObject = std::shared_ptr<XObjectBase>;

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

    inline Variant(List&&dct);
    inline Variant(const List&dct);

    /** @} */

    /**
     * @defgroup variant_dict_api XVariant dict API
     * @{
     */

    inline Variant(Dict&&dct);
    inline Variant(const Dict&dct);

    inline Variant& update(Dict&&dct);

    /** @} */

    /**
     * @defgroup variant_function_api XVariant function API
     * @{
     */

    inline Variant(Function&&fcn);
    inline Variant(const Function&fcn);

    inline auto operator()();

    template <typename FirstT, typename...RestT>
    auto operator()(FirstT&&, RestT&&...);

    inline auto apply(List&& args);
    inline auto apply(const List& args);

    /** @} */


    /**
     * @defgroup variant_object_api XVariant object API
     * @{
     */

    Variant(const XObjectPointer auto&);
    Variant(XObjectPointer auto&&);

    /** @} */

    /**
     * @ingroup variant_dict_api variant_list_api variant_object_api
     * @{
     **/

    inline Variant& operator[](const Value& index);
    inline const Variant& operator[](const Value& index) const;    

    /** @} */

    /**
     * @defgroup variant_value_api XVariant value API
     * @{
     */

    inline Variant(const char*);

    inline Variant(const Value&);
    inline Variant(Value&&);

    Variant(const XValueConvertible auto&);
    Variant(XValueConvertible auto&&);

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

    static constexpr const char* ctti_nameof() {
        return "xvar";
    }
private:
    using value_t = std::variant<Value, List, Dict, Function, SharedObject>;
    value_t _value;
};

} // variant

using xnone = variant::None;
using xval  = variant::Value;
using xlist = variant::List;
using xdict = variant::Dict;
using xfn   = variant::Function;
using xvar  = variant::Variant;

} // xdev

#include <xdev/xdev-variant.inl>
