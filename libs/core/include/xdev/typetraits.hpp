/**
 * @file xdev-typetraits.hpp
 **/
#pragma once

#include <type_traits>
#include <string>
#include <functional>

#ifdef __cpp_lib_concepts
#include <concepts>
#else
#include <xdev/std-concepts.hpp>
#endif

#ifdef _WIN32
namespace std {
namespace detail {
    template <class Default, class AlwaysVoid,
              template<class...> class Op, class... Args>
    struct detector {
      using value_t = std::false_type;
      using type = Default;
    };

    template <class Default, template<class...> class Op, class... Args>
    struct detector<Default, std::void_t<Op<Args...>>, Op, Args...> {
      using value_t = std::true_type;
      using type = Op<Args...>;
    };

} // detail

    struct nonesuch
    {
        nonesuch() = delete;
        ~nonesuch() = delete;
        nonesuch(nonesuch const&) = delete;
        void operator=(nonesuch const&) = delete;
    };

    template <template<class...> class Op, class... Args>
    using is_detected = typename detail::detector<nonesuch, void, Op, Args...>::value_t;

    template <template<class...> class Op, class... Args>
    using detected_t = typename detail::detector<nonesuch, void, Op, Args...>::type;

    template <class Default, template<class...> class Op, class... Args>
    using detected_or = detail::detector<Default, void, Op, Args...>;
} // std
#define __PRETTY_FUNCTION__ __FUNCSIG__
#else
#include <experimental/type_traits>
namespace std {
template <template<class...> class Op, class... Args>
using is_detected = experimental::is_detected<Op, Args...>;
}
#endif

#define xfwd(...) std::forward<decltype(__VA_ARGS__)>(__VA_ARGS__)

namespace xdev {

    template<typename T>
    struct always_false : std::false_type {};

    template <typename T, typename...TypesT>
    struct is_one_of
    {
        enum { value = (std::same_as<std::decay_t<T>, TypesT> || ...) };
    };
    template <typename T, typename...TypesT>
    constexpr bool is_one_of_v = is_one_of<T, TypesT...>::value;

    template <typename T, typename...TypesT>
    concept one_of = is_one_of_v<T, TypesT...>;

    template<typename T>
    using std_to_string_expression = decltype(std::to_string(std::declval<T>()));

    template<typename T>
    using has_std_to_string = std::is_detected<std_to_string_expression, T>;

    template<typename T>
    using to_string_expression = decltype(to_string(std::declval<T>()));

    template<typename T>
    using has_to_string = std::is_detected<to_string_expression, T>;


    template<typename T, typename = void>
    struct is_callable : std::is_function<T> { };

    template<typename T>
    struct is_callable<T, typename std::enable_if<
        std::is_same<decltype(void(&T::operator())), void>::value
        >::type> : std::true_type { };

    template <typename T>
    constexpr bool is_callable_v = is_callable<T>::value;


    namespace function_detail
    {
        template<typename Ret, typename Cls, typename IsMutable, typename IsLambda, typename... Args>
        struct types
        {
            using is_mutable = IsMutable;
            using is_lambda = IsLambda;
            static constexpr auto is_function() { return !is_lambda(); }

            enum { arity = sizeof...(Args) };

            using return_type = Ret;

            template<size_t i>
            struct arg
            {
                using type = typename std::tuple_element<i, std::tuple<Args...>>::type;
                using clean_type = std::remove_cvref_t<type>;
            };

            using function_type = std::function<Ret(Args...)>;

            struct parameters_tuple_all_enabled {
                template <typename T>
                static constexpr bool enabled = false;
            };

            template <typename...ArgsT>
            struct parameters_tuple_disable {
                template <typename T>
                static constexpr bool enabled = !std::disjunction_v<std::is_same<std::remove_cvref_t<T>, std::remove_cvref_t<ArgsT>>...>;
            };

            template <typename Predicate = parameters_tuple_all_enabled>
            struct parameters_tuple {

                template <typename FirstT, typename...RestT>
                static constexpr auto __make_tuple() {
                    if constexpr (!sizeof...(RestT)) {
                        if constexpr (Predicate::template enabled<FirstT>)
                            return std::make_tuple<FirstT>({});
                        else return std::tuple();
                    } else {
                        if constexpr (Predicate::template enabled<FirstT>)
                            return std::tuple_cat(std::make_tuple<FirstT>({}), __make_tuple<RestT...>());
                        else return __make_tuple<RestT...>();
                    }
                }

                struct _make {
                    constexpr auto operator()() {
                        if constexpr (sizeof...(Args) == 0)
                            return std::make_tuple();
                        else return __make_tuple<Args...>();
                    }
                };

                static constexpr auto make() {
                    return _make{}();
                }

                using tuple_type = std::invoke_result_t<_make>;
            };

            using args_tuple_type = std::tuple<Args...>;
        };
    }

    template<class T>
    struct function_traits
        : function_traits<decltype(&std::remove_cvref_t<T>::operator())>
    {};

    // mutable lambda
    template<class Ret, class Cls, class... Args>
    struct function_traits<Ret(Cls::*)(Args...)>
        : function_detail::types<Ret, Cls, std::true_type, std::true_type, Args...>
    {};

    // immutable lambda
    template<class Ret, class Cls, class... Args>
    struct function_traits<Ret(Cls::*)(Args...) const>
        : function_detail::types<Ret, Cls, std::false_type, std::true_type, Args...>
    {};

    // function
    template<class Ret, class... Args>
    struct function_traits<std::function<Ret(Args...)>>
        : function_detail::types<Ret, std::nullptr_t, std::true_type, std::false_type, Args...>
    {};

    // c-style function
    template<class Ret, class... Args>
    struct function_traits<Ret(*)(Args...)>
        : function_detail::types<Ret, std::nullptr_t, std::true_type, std::false_type, Args...>
    {};

    // template specialization detection

    template <class T, template <class...> class Template>
    struct is_specialization : std::false_type {};

    template <template <class...> class Template, class... Args>
    struct is_specialization<Template<Args...>, Template> : std::true_type {};

    template <class T, template <class...> class Template>
    concept specialization_of = is_specialization<std::decay_t<T>, Template>::value;

    template <typename DecaysT, typename ToT>
    concept decays_to = std::same_as<std::decay_t<DecaysT>, ToT>;

    } // xdev
