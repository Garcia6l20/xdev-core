#include <xdev/xdev-variant.hpp>
#include <xdev/xdev-exception.hpp>
#include <xdev/xdev-object.hpp>
#include <xdev/xdev-typetraits.hpp>

namespace std {

template <>
struct hash<xdev::xfn>
{
    std::size_t operator()(const xdev::xfn& /*var*/) const
    {
        throw xdev::XException("Dont use xdev::XFunction as keys");
    }
};

}

namespace xdev {
namespace variant {

namespace priv {
    template <typename Ret>
    Ret callfunc (std::function<Ret()> func, xlist&& lst)
    {
        if (lst.size() > 0)
            throw std::runtime_error("oops, argument list too long");
        return func();
    }

    template <typename Ret, typename Arg0, typename... Args>
    std::function<Ret(Args...)> make_recursive_lambda(std::function<Ret(Arg0, Args...)> func, xlist&& lst) {
        auto arg0 = lst[0].get<Arg0>();
        lst.erase(lst.begin());
        return [=](Args... args) -> Ret {
             return func(arg0, args...);
        };
    }

    template <typename Ret, typename ObjectT, typename... Args>
    enable_if_t<is_base_of_v<XObjectBase, ObjectT>,
    std::function<Ret(Args...)>> make_recursive_lambda(std::function<Ret(shared_ptr<ObjectT>, Args...)> func, xlist&& lst) {
        auto arg0 = lst[0].get<XObjectBase::ptr>()->cast<ObjectT>();
        lst.erase(lst.begin());
        return [=](Args... args) -> Ret {
             return func(arg0, args...);
        };
    }

    template <typename Ret, typename Arg0, typename... Args>
    Ret callfunc (std::function<Ret(Arg0, Args...)> func, xlist&& lst)
    {
        if (lst.size() != sizeof...(Args) + 1)
            throw std::runtime_error("oops, argument list too short");
        return callfunc(make_recursive_lambda(func, xfwd(lst)), xfwd(lst));
    }
}

template <typename Functor>
    requires(is_callable_v<Functor> and (not is_xfunction<Functor>) and (not std::same_as<Function, Variant>))
Function::Function(Functor ftor): base([f = std::function(ftor)](xlist&&lst) -> xvar {
    using traits = function_traits<Functor>;
    if constexpr (is_same_v<typename traits::return_type, void>) {
        priv::callfunc(f, xfwd(lst));
        return {}; // none
    } else return priv::callfunc(f, xfwd(lst));
}) {}

template <typename ResultT>
ResultT Function::apply(const xlist& args) {
    return base::operator()(args);
}

template <typename ResultT>
ResultT Function::apply(xlist&& args) {
    return base::operator()(std::forward<xlist>(args));
}

template <typename ResultT, typename FirstT, typename...RestT>
ResultT Function::operator()(FirstT&&first, RestT&&...rest) {
    if constexpr (is_same_v<ResultT, xvar>)
        return apply<ResultT>(xlist{xfwd(first), xfwd(rest)...});
    else if constexpr (is_base_of_v<XObjectBase, ResultT>)
        return apply<ResultT>(xlist{xfwd(first), xfwd(rest)...}).template get<XObjectBase::ptr>()->template cast<ResultT>();
    else return apply<ResultT>(xlist{xfwd(first), xfwd(rest)...}).template get<ResultT>();
}

template <typename ResultT>
ResultT Function::operator()() {
    if constexpr (is_same_v<ResultT, xvar>)
        return apply<ResultT>(xlist{});
    else if constexpr (is_base_of_v<XObjectBase, ResultT>)
        return apply<ResultT>(xlist{}).template get<XObjectBase::ptr>()->template cast<ResultT>();
    else return apply<ResultT>(xlist{}).template get<ResultT>();
}

auto Function::operator<=>(const Function& other) const {
    return target<target_t>() <=> other.target<target_t>();
}

bool Function::operator==(const Function& other) const {
    return target<target_t>() == other.target<target_t>();
}

}
}
