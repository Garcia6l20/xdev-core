/**
 * @file xdev-variant.hpp
 * @brief The Ultimate variant class
 **/
#pragma once

#include <xdev/variant-policies.hpp>

#include <variant>
#include <memory>
#include <vector>
#include <list>
#include <string>
#include <map>
#include <stdint.h>

#include <iostream>

#include <xdev/xdev-core-export.hpp>

#include <xdev/typetraits.hpp>
#include <xdev/concepts.hpp>

#include <any>

#include <xdev/variant-value.hpp>
#include <xdev/variant-list.hpp>
#include <xdev/variant-dict.hpp>
#include <xdev/variant-function.hpp>

namespace xdev {

class XObjectBase;

namespace variant {

using SharedObject = std::shared_ptr<XObjectBase>;

template <typename StringPolicyT = StdStringPolicy>
class Variant {

  using value_type = Value<StringPolicyT>;
  using dict_type = Dict<StringPolicyT>;
  using list_type = List<StringPolicyT>;
  using fn_type = Function<StringPolicyT>;

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

    inline Variant(List<StringPolicyT>&&dct);
    inline Variant(const List<StringPolicyT>&dct);

    /** @} */

    /**
     * @defgroup variant_dict_api XVariant dict API
     * @{
     */

    inline Variant(Dict<StringPolicyT> &&dct);
    inline Variant(const Dict<StringPolicyT> &dct);

    inline Variant& update(Dict<StringPolicyT> &&dct);

    /** @} */

    /**
     * @defgroup variant_function_api XVariant function API
     * @{
     */

    inline Variant(Function<StringPolicyT> &&fcn);
    inline Variant(const Function<StringPolicyT> &fcn);

    inline auto operator()();

    template <typename FirstT, typename...RestT>
    auto operator()(FirstT&&, RestT&&...);

    inline auto apply(List<StringPolicyT> && args);
    inline auto apply(const List<StringPolicyT> & args);

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

    inline Variant& operator[](const Value<StringPolicyT>& index);
    inline const Variant& operator[](const Value<StringPolicyT>& index) const;

    /** @} */

    /**
     * @defgroup variant_value_api XVariant value API
     * @{
     */

    inline Variant(const char*);

    inline Variant(const Value<StringPolicyT>&);
    inline Variant(Value<StringPolicyT>&&);

    Variant(const XValueConvertible<StringPolicyT> auto&);
    Variant(XValueConvertible<StringPolicyT> auto&&);

    inline Variant& operator!();

    inline bool operator==(char const* b) const;

    // in/decrement operators

    inline auto& operator++();
    inline auto operator++(int);

    inline auto & operator--();
    inline auto operator--(int);

    /** @} */

    static inline Variant FromJSON(const std::string&);
    static inline Variant FromYAML(const std::string&);

    static constexpr const char* ctti_nameof() {
        return "xvar";
    }
private:
    using value_t = std::variant<Value<StringPolicyT>, List<StringPolicyT>, Dict<StringPolicyT>, Function<StringPolicyT>, SharedObject>;
    value_t _value;
};

} // variant

using xnone = variant::None;

using xval  = variant::Value<struct StdStringPolicy>;
using xlist = variant::List<struct StdStringPolicy>;
using xdict = variant::Dict<struct StdStringPolicy>;
using xfn   = variant::Function<struct StdStringPolicy>;
using xvar  = variant::Variant<struct StdStringPolicy>;

using xcval  = variant::Value<struct StdStringViewPolicy>;
using xclist = variant::List<struct StdStringViewPolicy>;
using xcdict = variant::Dict<struct StdStringViewPolicy>;
using xcfn   = variant::Function<struct StdStringViewPolicy>;
using xcvar  = variant::Variant<struct StdStringViewPolicy>;

} // xdev

#include <xdev/variant.inl>
