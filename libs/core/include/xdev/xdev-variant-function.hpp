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
class List;

template <typename T>
concept is_xfunction = requires(T f) {
    {f(List())} -> std::convertible_to<Variant>;
};

template <typename T, typename...Args>
concept is_xfunction_wrappable = requires(T f) {
    {f(Args{}...)};
};

struct Function: std::function<Variant(List)> {
    typedef Variant(*target_t)(List);
    using base = std::function<Variant(List)>;
    using base::function;
    using base::operator();
    using base::operator=;
    using base::operator bool;

    template <typename Functor>
        requires(is_callable_v<Functor> and (not is_xfunction<Functor>) and (not std::same_as<Function, Variant>))
    Function(Functor ftor);

    template <typename ResultT = Variant>
    ResultT operator()();

    template <typename ResultT = Variant, typename FirstT, typename...RestT>
    ResultT operator()(FirstT&&, RestT&&...);

    template <typename ResultT = Variant>
    ResultT apply(List&& args);

    template <typename ResultT = Variant>
    ResultT apply(const List& args);

    auto operator<=>(const Function&) const = default;
    inline bool operator ==(const Function&) const;
};

}
