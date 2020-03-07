/**
 * @file xdev-template-expressions.hpp
 * @author Sylvain Garcia <garcia.sylvain@gmail.com>
 * @date 20/12/2015
 **/
#pragma once

#include <xdev/xdev-template-export.hpp>

#include <xdev/xdev-variant.hpp>
#include <xdev/xdev-resources.hpp>
#include <xdev/xdev-tools.hpp>
#include <xdev/xdev-exception.hpp>

#include <boost/math/constants/constants.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/qi.hpp>

#include <cmath>
#include <limits>
#include <iterator>

namespace xdev::temp {

class XDEV_TEMPLATE_EXPORT Expression
{
public:
    class Error : public XException {
    public:
        using XException::XException;
    };
    using ptr = shared_ptr<Expression>;
    virtual ~Expression();
    virtual XVariant eval(const XDict& context, const XResources::ptr& res = XResources::Make()) = 0;
    const std::string& content() const { return m_content; }
    static Expression::ptr Load(const string& content);
protected:
    Expression(const std::string& content): m_content(content) {
        tools::trim(m_content);
    }
    string m_content;
private:
    static regex TestOperatorsRe;
};

class XDEV_TEMPLATE_EXPORT TemplateExpression : public Expression, protected tools::SharedMaker<TemplateExpression>
{
public:
    virtual XVariant eval(const XDict& context, const XResources::ptr& res = XResources::Make()) override;
    using RenderFunction = function<XVariant(const XList&, const XDict& context, const XResources::ptr& res)>;
    static void AddFunction(const string& name, const RenderFunction& function);
    static XVariant CallFunction(const string & key, const XList& nodes, const XDict& context, const XResources::ptr& res);
private:
    TemplateExpression(const string& content);
    virtual ~TemplateExpression() override;
    using RenderFunctionMap = map<string, RenderFunction>;
    static RenderFunctionMap InitRunderFunctions();
    static RenderFunctionMap RenderFunctions;
    string m_baseVariable;
    list<pair<RenderFunction, vector<string>>> m_functionCalls;
    static bool ExtractFunction(const string& raw_content, string& function_name, vector<string>& args);
    friend class Expression;
    friend class BlockBlock;
    friend struct tools::make_shared_enabler<TemplateExpression>;
};

class TestExpression : public Expression, protected tools::SharedMaker<TestExpression>
{
public:
    virtual XVariant eval(const XDict& context, const XResources::ptr& res = XResources::Make()) override;
private:
    TestExpression(const string& content);
    Expression::ptr m_left, m_right;
    using Operator = std::function<bool(const XVariant&, const XVariant&)>;
    Operator m_operator;
    using OperatorMap = map<string, Operator>;
    static OperatorMap InitOperators();
    static OperatorMap Operators;
    static regex OpertatorsRe;
    friend class Expression;
    friend struct tools::make_shared_enabler<TestExpression>;
};

namespace expressions {

struct VariantLoaderVisitor {
    template <typename T>
    inline double operator()(const T& val) const
    {
        return static_cast<double>(val);
    }
    inline double operator()(bool&& val) const
    {
        return val ? 1. : 0.;
    }
    inline double operator()(const XNone&) const
    {
        return 0.;
    }
    inline double operator()(const string& val) const
    {
        return stod(val);
    }
    // folowing operators just checks existance
    inline double operator()(const XObjectBase::ptr& obj) const
    {
        return static_cast<double>(obj.get() != nullptr);
    }
    inline double operator()(const XList&) const
    {
        return 1.;
    }
    inline double operator()(const XDict&) const
    {
        return 1.;
    }
    inline double operator()(const XValue&) const
    {
        return 0.; // might not append
    }
    inline double operator()(const XFunction&) const
    {
        return 0.; // might not append
    }
};

struct lazy_pow_
{
    template <typename X, typename Y>
    struct result { typedef X type; };

    template <typename X, typename Y>
    X operator()(X x, Y y) const
    {
        return std::pow(x, y);
    }
};

struct lazy_ufunc_
{
    template <typename F, typename A1>
    struct result { typedef A1 type; };

    template <typename F, typename A1>
    A1 operator()(F f, A1 a1) const
    {
        return f(a1);
    }
};

struct lazy_bfunc_
{
    template <typename F, typename A1, typename A2>
    struct result { typedef A1 type; };

    template <typename F, typename A1, typename A2>
    A1 operator()(F f, A1 a1, A2 a2) const
    {
        return f(a1, a2);
    }
};

template <class T>
T max_by_value ( const T a, const T b ) {
    return std::max(a, b);
}

template <class T>
T min_by_value ( const T a, const T b ) {
    return std::min(a, b);
}

template <typename FPT, typename Iterator>
struct grammar
    : boost::spirit::qi::grammar<
            Iterator, FPT(), boost::spirit::ascii::space_type
        >
{

    // symbol table for constants like "pi"
    struct constant_
        : boost::spirit::qi::symbols<
                typename std::iterator_traits<Iterator>::value_type,
                FPT
            >
    {
        constant_()
        {
            this->add
                ("digits",   std::numeric_limits<FPT>::digits    )
                ("digits10", std::numeric_limits<FPT>::digits10  )
                ("e" ,       boost::math::constants::e<FPT>()    )
                ("epsilon",  std::numeric_limits<FPT>::epsilon() )
                ("pi",       boost::math::constants::pi<FPT>()   )
            ;
        }
    } constant;

    struct locals_
        : boost::spirit::qi::symbols<
                typename std::iterator_traits<Iterator>::value_type,
                FPT
            >
    {

    } locals;

    // symbol table for unary functions like "abs"
    struct ufunc_
        : boost::spirit::qi::symbols<
                typename std::iterator_traits<Iterator>::value_type,
                FPT (*)(FPT)
            >
    {
        ufunc_()
        {
            this->add
                ("abs"  , static_cast<FPT (*)(FPT)>(&std::abs  ))
                ("acos" , static_cast<FPT (*)(FPT)>(&std::acos ))
                ("asin" , static_cast<FPT (*)(FPT)>(&std::asin ))
                ("atan" , static_cast<FPT (*)(FPT)>(&std::atan ))
                ("ceil" , static_cast<FPT (*)(FPT)>(&std::ceil ))
                ("cos"  , static_cast<FPT (*)(FPT)>(&std::cos  ))
                ("cosh" , static_cast<FPT (*)(FPT)>(&std::cosh ))
                ("exp"  , static_cast<FPT (*)(FPT)>(&std::exp  ))
                ("floor", static_cast<FPT (*)(FPT)>(&std::floor))
                ("log10", static_cast<FPT (*)(FPT)>(&std::log10))
                ("log"  , static_cast<FPT (*)(FPT)>(&std::log  ))
                ("sin"  , static_cast<FPT (*)(FPT)>(&std::sin  ))
                ("sinh" , static_cast<FPT (*)(FPT)>(&std::sinh ))
                ("sqrt" , static_cast<FPT (*)(FPT)>(&std::sqrt ))
                ("tan"  , static_cast<FPT (*)(FPT)>(&std::tan  ))
                ("tanh" , static_cast<FPT (*)(FPT)>(&std::tanh ))
            ;
        }
    } ufunc;

    // symbol table for binary functions like "pow"
    struct bfunc_
        : boost::spirit::qi::symbols<
                typename std::iterator_traits<Iterator>::value_type,
                FPT (*)(FPT, FPT)
            >
    {
        bfunc_()
        {
            this->add
                ("atan2", static_cast<FPT (*)(FPT, FPT)>(&std::atan2  ))
                ("max"  , static_cast<FPT (*)(FPT, FPT)>(&max_by_value))
                ("min"  , static_cast<FPT (*)(FPT, FPT)>(&min_by_value))
                ("pow"  , static_cast<FPT (*)(FPT, FPT)>(&std::pow    ))
            ;
        }
    } bfunc;

    boost::spirit::qi::rule<
            Iterator, FPT(), boost::spirit::ascii::space_type
        > expression, term, factor, primary;

    grammar() : grammar::base_type(expression)
    {
        using boost::spirit::qi::real_parser;
        using boost::spirit::qi::real_policies;
        real_parser<FPT,real_policies<FPT> > real;

        using boost::spirit::qi::_1;
        using boost::spirit::qi::_2;
        using boost::spirit::qi::_3;
        using boost::spirit::qi::no_case;
        using boost::spirit::qi::_val;

        boost::phoenix::function<lazy_pow_>   lazy_pow;
        boost::phoenix::function<lazy_ufunc_> lazy_ufunc;
        boost::phoenix::function<lazy_bfunc_> lazy_bfunc;

        expression =
            term                   [_val =  _1]
            >> *(  ('+' >> term    [_val += _1])
                |  ('-' >> term    [_val -= _1])
                )
            ;

        term =
            factor                 [_val =  _1]
            >> *(  ('*' >> factor  [_val *= _1])
                |  ('/' >> factor  [_val /= _1])
                )
            ;

        factor =
            primary                [_val =  _1]
            >> *(  ("**" >> factor [_val = lazy_pow(_val, _1)])
                )
            ;

        primary =
            real                   [_val =  _1]
            |   '(' >> expression  [_val =  _1] >> ')'
            |   ('-' >> primary    [_val = -_1])
            |   ('+' >> primary    [_val =  _1])
            |   (no_case[ufunc] >> '(' >> expression >> ')')
                                   [_val = lazy_ufunc(_1, _2)]
            |   (no_case[bfunc] >> '(' >> expression >> ','
                                       >> expression >> ')')
                                   [_val = lazy_bfunc(_1, _2, _3)]
            |   no_case[constant]  [_val =  _1]
            |   no_case[locals]    [_val =  _1]
            ;

    }
};

template <typename FPT, typename Iterator>
bool parse(const string& input,
           const grammar<FPT,Iterator> &g,
           FPT &result)
{
    return boost::spirit::qi::phrase_parse(
        input.begin(), input.end(), g, boost::spirit::ascii::space, result);
}

template <typename T = double>
T eval(const std::string & input, const XDict& context)
{
    T result;
    grammar<T,std::string::const_iterator> eg;
    for(auto&& item: context){
        eg.locals.add(item.first.get<string>(), item.second.visit(VariantLoaderVisitor()));
    }
    parse(input, eg, result);
    return result;
}

} // namespace expressions
} // namespace xdev::template
