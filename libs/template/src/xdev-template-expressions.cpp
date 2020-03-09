#include <xdev/xdev-template-expressions.hpp>
#include <xdev/xdev-template.hpp>
#include <xdev/xdev-tools.hpp>

#include <iomanip>
#include <sstream>
#include <chrono>

using namespace xdev;
using namespace xdev::temp;

Expression::~Expression() {}

Expression::ptr Expression::Load(const string& content)
{
    if (regex_match(content, TestExpression::OpertatorsRe))
    {
        return TestExpression::Make(content);
    }
    return TemplateExpression::Make(content);
}

// -----------------------------------------
// TemplateExpression
//

TemplateExpression::TemplateExpression(const string& content) :
    Expression(content)
{
    static const regex pipe_expr(R"(([\w\.]+)\s{0,}((\|.+){1,}))");
    cmatch what;
    string function_name;
    vector<string> function_args;
    if (ExtractFunction(m_content, function_name, function_args))
    {
        m_functionCalls.push_back({RenderFunctions.at(function_name), function_args});
    }
    else if (regex_match(m_content.c_str(), what, pipe_expr))
    {
        // get base variable
        m_baseVariable = what[1];
        // process pipes
        vector<string> raw_pipes = tools::split(what[2], '|', true);
        tools::fast_foreach(raw_pipes, [this](string& raw_pipe)
        {
            tools::trim(raw_pipe);
            vector<string> fargs;
            string fname = raw_pipe;
            if (ExtractFunction(raw_pipe, fname, fargs) ||
                !fname.empty())
            {
                m_functionCalls.push_back({RenderFunctions.at(fname), fargs});
            }
        });
    }
    else
    {
        m_baseVariable = m_content;
    }
}

TemplateExpression::~TemplateExpression() {}

xvar TemplateExpression::eval(const xdict& context, const XResources::ptr& res)
{
    xvar result;
    if (m_baseVariable.size())
    {
        try {
            result = context.at(m_baseVariable);
        }
        catch (const std::out_of_range&) {
            try {
                result = xvar::FromJSON(m_baseVariable);
            } catch (const exception&) {
                throw Error("Undefined variable: " + m_baseVariable);
            }
        }
    }
    for(auto&[func, arg_list]: m_functionCalls)
    {
        xlist args;
        if (!result.empty())
        {
            args.push(result);
        }
        for(auto& arg: arg_list) {
            xvar var;
            cmatch what;
            static const regex string_re(R"(['"](.+)?['"])");
            static const regex num_re(R"(\d+(\.\d+)?)");
            // is this a string ?
            if (regex_match(arg.c_str(), what, string_re))
            {
                var = string(what[1]);
            }
            // is this a number ?
            else if (regex_match(arg.c_str(), what, num_re))
            {
                var = stod(what[1]);
            }
            // so... it might be a variable
            else
            {
                try {
                    var = context.at(arg);
                }
                catch (const std::out_of_range&) {
                    throw Error("Undefined variable: " + arg);
                }
            }
            args.push(var);
        }
        result = func(args, context, res);
    }
    return result;
}

TemplateExpression::RenderFunctionMap TemplateExpression::InitRunderFunctions()
{
    TemplateExpression::RenderFunctionMap map;
    map["date"] = [](const xlist& args, const xdict& /*context*/, const XResources::ptr& /*res*/) -> xvar {
        string fmt = "%c %Z";
        if (args.size() > 0)
            fmt = args[0].get<string>();
        auto now = std::chrono::system_clock::now();
        const auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), fmt.c_str());
        return ss.str();

    };

    map["upper"] = [](const xlist& args, const xdict& /*context*/, const XResources::ptr& /*res*/) -> xvar {
        string input = args[0].get<string>();
        tools::to_upper(input);
        return input;
    };

    map["lower"] = [](const xlist& args, const xdict& /*context*/, const XResources::ptr& /*res*/) -> xvar {
        string input = args[0].get<string>();
        tools::to_lower(input);
        return input;
    };

    map["replace"] = [](const xlist& args, const xdict& /*context*/, const XResources::ptr& /*res*/) -> xvar {
        string str = args[0].get<string>();
        string from = args[1].get<string>();
        string to = args[2].get<string>();
        tools::replace(str, from, to);
        return str;
    };

    map["length"] = [](const xlist& args, const xdict& /*context*/, const XResources::ptr& /*res*/) -> xvar {
        if (args[0].is<xdict>())
        {
            return int(args[0].get<xdict>().size());
        }
        else if (args[0].is<xlist>())
        {
            return int(args[0].get<xlist>().size());
        }
        else if (args[0].is<string>())
        {
            return int(args[0].get<string>().size());
        }
        throw Expression::Error("Cannot apply 'length' to "s/* + args[0].typeName()*/);
    };

    map["render"] = [](const xlist& args, const xdict& context, const XResources::ptr& res) -> xvar {
        string key = args[0].get<string>();
        if (res->has(key))
        {
            return xtemplate::CompileResource(key, res)->process(context);
        }
        throw Error("TODO");
    };

    map["super"] = [](const xlist& args, const xdict& context, const XResources::ptr& res) -> xvar {
        try
        {
            return TemplateExpression::CallFunction(context.at("$super_key").get<string>(),
                args, context, res);
        }
        catch(out_of_range& err)
        {
            throw Error("super failed", err);
        }
    };
    return map;
}

TemplateExpression::RenderFunctionMap TemplateExpression::RenderFunctions = TemplateExpression::InitRunderFunctions();

void TemplateExpression::AddFunction(const string & key, const TemplateExpression::RenderFunction & function)
{
    RenderFunctions[key] = function;
}

xvar TemplateExpression::CallFunction(const string & key, const xlist& nodes, const xdict& context, const XResources::ptr& res)
{
    try {
        return RenderFunctions.at(key)(nodes, context, res);
    }
    catch (std::out_of_range& err) {
        throw Error("No such function: " +  key, err);
    }
}

bool TemplateExpression::ExtractFunction(const string& raw_content, string& function_name, vector<string>& args)
{
    static const regex function_expr(R"(\s*([a-zA-Z_]+)\((.*)\)\s*$)");
    cmatch what;
    if (!regex_match(raw_content.c_str(), what, function_expr))
    {
        return false;
    }
    function_name = what[1];
    tools::trim(function_name);
    vector<string> raw_args = tools::split(what[2], ',', true);
    tools::fast_foreach(raw_args, [&args](string& arg)
    {
        tools::trim(arg);
        args.push_back(arg);
    });
    return true;
}

// --------------------------
// TestExpression
//

TestExpression::TestExpression(const string& content):
    Expression(content)
{
    smatch match;
    regex_match(content, match, OpertatorsRe);
    m_left = Expression::Load(match[1]);
    m_right = Expression::Load(match[3]);
    m_operator = Operators[match[2]];
}

xvar TestExpression::eval(const xdict& context, const XResources::ptr& res)
{
    return m_operator(m_left->eval(context, res), m_right->eval(context, res));
}

template <template<typename> typename Functor>
struct OperatorVisitor
{
    const xvar& m_left;

    OperatorVisitor(const xvar& left): m_left(left) {}

    template <typename T>
    bool operator()(const T& right)
    {
        return Functor<T>()(m_left.convert<T>(), right);
    }
    bool operator()(const xnone&)
    {
        return false;
    }

//    bool operator()(const XObject::ptr& right)
//    {
//        return false; //m_left.get<XObject::ptr>() == right;
//    }
    bool operator()(const xlist& /*right*/)
    {
        return false; //m_left.get<XList>() == right;
    }
    bool operator()(const xdict& /*right*/)
    {
        return false; //m_left.get<XDict>() == right;
    }
    bool operator()(const xval&)
    {
        throw std::runtime_error("might never append");
    }
    bool operator()(const xfn&)
    {
        throw std::runtime_error("functions not handled here");
    }
};

TestExpression::OperatorMap TestExpression::InitOperators()
{
    TestExpression::OperatorMap test_operators;
    test_operators["=="] = [](const xvar& left, const xvar& right) {
        return right.visit(OperatorVisitor<std::equal_to>(left));
    };
    test_operators["!="] = [](const xvar& left, const xvar& right) {
        return right.visit(OperatorVisitor<std::not_equal_to>(left));
    };
    test_operators["<="] = [](const xvar& left, const xvar& right) {
        return right.visit(OperatorVisitor<std::less_equal>(left));
    };
    test_operators[">="] = [](const xvar& left, const xvar& right) {
        return right.visit(OperatorVisitor<std::greater_equal>(left));
    };
    test_operators["<"] = [](const xvar& left, const xvar& right) {
        return right.visit(OperatorVisitor<std::less>(left));
    };
    test_operators[">"] = [](const xvar& left, const xvar& right) {
        return right.visit(OperatorVisitor<std::greater>(left));
    };
    return test_operators;
}

TestExpression::OperatorMap TestExpression::Operators = TestExpression::InitOperators();

regex TestExpression::OpertatorsRe = []() {
    // order is critical so autoload is not possible :(
    vector<string> test_operators = {"==", "!=", "\\<=", "\\>=", "\\<", "\\>"};
    return regex(R"((.*)()" + tools::join(test_operators, "|") + R"()(.*))");
} ();
