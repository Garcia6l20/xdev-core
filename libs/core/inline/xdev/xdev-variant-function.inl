#include <xdev/xdev-variant.hpp>
#include <xdev/xdev-exception.hpp>
#include <xdev/xdev-object.hpp>
#include <xdev/xdev-typetraits.hpp>

namespace std {

template <>
struct hash<xdev::XFunction>
{
    std::size_t operator()(const xdev::XFunction& var) const
    {
        throw xdev::XException("Dont use xdev::XFunction as keys");
    }
};

}

namespace xdev {
namespace variant {

namespace priv {
    template <typename Ret>
    Ret callfunc (std::function<Ret()> func, XArray&& array)
    {
        if (array.size() > 0)
            throw std::runtime_error("oops, argument list too long");
        return func();
    }

    template <typename Ret, typename Arg0, typename... Args>
    std::function<Ret(Args...)> make_recursive_lambda(std::function<Ret(Arg0, Args...)> func, XArray&& array) {
        auto arg0 = array[0].get<Arg0>();
        array.erase(array.begin());
        return [=](Args... args) -> Ret {
             return func(arg0, args...);
        };
    }

    template <typename Ret, typename ObjectT, typename... Args>
    enable_if_t<is_base_of_v<XObjectBase, ObjectT>,
    std::function<Ret(Args...)>> make_recursive_lambda(std::function<Ret(shared_ptr<ObjectT>, Args...)> func, XArray&& array) {
        auto arg0 = array[0].get<XObjectBase::ptr>()->cast<ObjectT>();
        array.erase(array.begin());
        return [=](Args... args) -> Ret {
             return func(arg0, args...);
        };
    }

    template <typename Ret, typename Arg0, typename... Args>
    Ret callfunc (std::function<Ret(Arg0, Args...)> func, XArray&& array)
    {
        if (array.size() != sizeof...(Args) + 1)
            throw std::runtime_error("oops, argument list too short");
        return callfunc(make_recursive_lambda(func, xfwd(array)), xfwd(array));
    }
}

template <typename Functor>
    requires(!is_xfunction<Functor> && is_callable_v<Functor>)
Function::Function(Functor ftor): base([f = std::function(ftor)](XArray&&array) -> XVariant {
    using traits = function_traits<Functor>;
    if constexpr (is_same_v<typename traits::return_type, void>) {
        priv::callfunc(f, xfwd(array));
        return {}; // none
    } else return priv::callfunc(f, xfwd(array));
}) {}

template <typename ResultT>
ResultT Function::apply(const XArray& args) {
    return base::operator()(args);
}

template <typename ResultT>
ResultT Function::apply(XArray&& args) {
    return base::operator()(std::forward<XArray>(args));
}

template <typename ResultT = XVariant, typename FirstT, typename...RestT>
ResultT Function::operator()(FirstT&&first, RestT&&...rest) {
    if constexpr (is_same_v<ResultT, XVariant>)
        return apply<ResultT>(XArray{xfwd(first), xfwd(rest)...});
    else if constexpr (is_base_of_v<XObjectBase, ResultT>)
        return apply<ResultT>(XArray{xfwd(first), xfwd(rest)...}).template get<XObjectBase::ptr>()->template cast<ResultT>();
    else return apply<ResultT>(XArray{xfwd(first), xfwd(rest)...}).template get<ResultT>();
}

template <typename ResultT = XVariant>
ResultT Function::operator()() {
    if constexpr (is_same_v<ResultT, XVariant>)
        return apply<ResultT>(XArray{});
    else if constexpr (is_base_of_v<XObjectBase, ResultT>)
        return apply<ResultT>(XArray{}).template get<XObjectBase::ptr>()->template cast<ResultT>();
    else return apply<ResultT>(XArray{}).template get<ResultT>();
}

bool Function::operator==(const Function& other) const {
    return target<target_t>() == other.target<target_t>();
}

}
}
