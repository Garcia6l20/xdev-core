/**
 * @file xdev-variant-function.hpp
 */
#pragma once

#include <functional>

#ifdef __cpp_lib_concepts
#include <concepts>
#else
#include <xdev/std-concepts.hpp>
#endif

namespace xdev::variant {

class Variant;
class Array;

template <typename T>
concept is_xfunction = requires(T f) {
    {f(Array())} -> std::convertible_to<Variant>;
};

template <typename T, typename...Args>
concept is_xfunction_wrappable = requires(T f) {
    {f(Args{}...)};
};

struct Function: std::function<Variant(Array)> {
    typedef Variant(*target_t)(Array);
    using base = std::function<Variant(Array)>;
    using base::function;
    using base::operator();
    using base::operator=;
    using base::operator bool;

    template <typename Functor>
        requires(!is_xfunction<Functor> && is_callable_v<Functor>)
    Function(Functor ftor);

    template <typename ResultT = Variant>
    ResultT operator()();

    template <typename ResultT = Variant, typename FirstT, typename...RestT>
    ResultT operator()(FirstT&&, RestT&&...);

    template <typename ResultT = Variant>
    ResultT apply(Array&& args);

    template <typename ResultT = Variant>
    ResultT apply(const Array& args);

    inline bool operator ==(const Function&) const;
};

}
