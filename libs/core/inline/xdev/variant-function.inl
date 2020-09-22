#include <xdev/variant.hpp>
#include <xdev/exception.hpp>
#include <xdev/object.hpp>
#include <xdev/typetraits.hpp>

namespace std {

template<>
struct hash<xdev::xfn> {
  std::size_t operator()(const xdev::xfn & /*var*/) const {
    throw xdev::XException("Dont use xdev::XFunction as keys");
  }
};

} // namespace std

namespace xdev { namespace variant {

  namespace priv {
    template<typename Ret>
    Ret callfunc(std::function<Ret()> func, xlist &&lst) {
      if (lst.size() > 0)
        throw std::runtime_error("oops, argument list too long");
      return func();
    }

    template<typename Ret, typename Arg0, typename... Args>
    std::function<Ret(Args...)> make_recursive_lambda(std::function<Ret(Arg0, Args...)> func, xlist &&lst) {
      auto arg0 = lst[0].get<Arg0>();
      lst.erase(lst.begin());
      return [=](Args... args) -> Ret { return func(arg0, args...); };
    }

    template<typename Ret, typename ObjectT, typename... Args>
    enable_if_t<is_base_of_v<XObjectBase, ObjectT>, std::function<Ret(Args...)>> make_recursive_lambda(
      std::function<Ret(shared_ptr<ObjectT>, Args...)> func, xlist &&lst) {
      auto arg0 = lst[0].get<XObjectBase::ptr>()->cast<ObjectT>();
      lst.erase(lst.begin());
      return [=](Args... args) -> Ret { return func(arg0, args...); };
    }

    template<typename Ret, typename Arg0, typename... Args>
    Ret callfunc(std::function<Ret(Arg0, Args...)> func, xlist &&lst) {
      if (lst.size() != sizeof...(Args) + 1)
        throw std::runtime_error("oops, argument list too short");
      return callfunc(make_recursive_lambda(func, xfwd(lst)), xfwd(lst));
    }
  } // namespace priv

  template<typename StringPolicy>
  template<typename Functor>
  requires(is_callable_v<Functor> and (not is_xfunction<Functor, StringPolicy>)
           and (not std::same_as<Function<StringPolicy>, Variant<StringPolicy>>))
    Function<StringPolicy>::Function(Functor ftor) :
    base([f = std::function(ftor)](List<StringPolicy> &&lst) -> Variant<StringPolicy> {
      using traits = function_traits<Functor>;
      if constexpr (is_same_v<typename traits::return_type, void>) {
        priv::callfunc(f, xfwd(lst));
        return {}; // none
      } else
        return priv::callfunc(f, xfwd(lst));
    }) {}

  template<typename StringPolicy>
  template<typename ResultT>
  ResultT Function<StringPolicy>::apply(const List<StringPolicy> &args) {
    return base::operator()(args);
  }

  template<typename StringPolicy>
  template<typename ResultT>
  ResultT Function<StringPolicy>::apply(List<StringPolicy> &&args) {
    return base::operator()(std::forward<xlist>(args));
  }

  template<typename StringPolicy>
  template<typename ResultT, typename FirstT, typename... RestT>
  ResultT Function<StringPolicy>::operator()(FirstT &&first, RestT &&... rest) {
    if constexpr (is_same_v<ResultT, xvar>)
      return apply<ResultT>(xlist {xfwd(first), xfwd(rest)...});
    else if constexpr (is_base_of_v<XObjectBase, ResultT>)
      return apply<ResultT>(xlist {xfwd(first), xfwd(rest)...})
        .template get<XObjectBase::ptr>()
        ->template cast<ResultT>();
    else
      return apply<ResultT>(xlist {xfwd(first), xfwd(rest)...}).template get<ResultT>();
  }

  template<typename StringPolicy>
  template<typename ResultT>
  ResultT Function<StringPolicy>::operator()() {
    if constexpr (is_same_v<ResultT, xvar>)
      return apply<ResultT>(xlist {});
    else if constexpr (is_base_of_v<XObjectBase, ResultT>)
      return apply<ResultT>(xlist {}).template get<XObjectBase::ptr>()->template cast<ResultT>();
    else
      return apply<ResultT>(xlist {}).template get<ResultT>();
  }

  template<typename StringPolicy>
  auto Function<StringPolicy>::operator<=>(const Function<StringPolicy> &other) const {
    return this->template target<target_t>() <=> other.template target<target_t>();
  }

  template<typename StringPolicy>
  bool Function<StringPolicy>::operator==(const Function<StringPolicy> &other) const {
    return this->template target<target_t>() == other.template target<target_t>();
  }

}} // namespace xdev::variant
