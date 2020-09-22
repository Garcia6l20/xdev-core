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

template <typename StringPolicy>
class Variant;

template <typename StringPolicy>
class List;

template <typename T, typename StringPolicy>
concept is_xfunction = requires(T f) {
    {f(List<StringPolicy>())} -> std::convertible_to<Variant<StringPolicy>>;
};

template <typename T, typename...Args>
concept is_xfunction_wrappable = requires(T f) {
    {f(Args{}...)};
};

template <typename StringPolicy>
class Function: std::function<Variant<StringPolicy>(List<StringPolicy>)> {
    typedef Variant<StringPolicy>(*target_t)(List<StringPolicy>);

public:
    using base = std::function<Variant<StringPolicy>(List<StringPolicy>)>;
    using base::function;
    using base::operator();
    using base::operator=;
    using base::operator bool;

    template <typename Functor>
        requires(is_callable_v<Functor> and (not is_xfunction<Functor, StringPolicy>) and (not std::same_as<Function<StringPolicy>, Variant<StringPolicy>>))
    Function(Functor ftor);

    template <typename ResultT = Variant<StringPolicy>>
    ResultT operator()();

    template <typename ResultT = Variant<StringPolicy>, typename FirstT, typename...RestT>
    ResultT operator()(FirstT&&, RestT&&...);

    template <typename ResultT = Variant<StringPolicy>>
    ResultT apply(List<StringPolicy>&& args);

    template <typename ResultT = Variant<StringPolicy>>
    ResultT apply(const List<StringPolicy>& args);

    inline auto operator<=>(const Function&) const;
    inline bool operator ==(const Function&) const;

    static constexpr const char* ctti_nameof() {
        return "xfn";
    }
};

}
