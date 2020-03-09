#include <xdev/xdev-moc.hpp>
#include <regex>
#include <functional>
#include <iostream>
#include <fstream>
#include <iterator>

#include <xdev/xdev-template-expressions.hpp>
#include <xdev/xdev-tools.hpp>

#include <xdev-moc-lib-resources.hpp>

#include <spdlog/spdlog.h>

using namespace xdev;
namespace fs = xdev::filesystem;

static bool g_check_functions_initialized = false;

auto register_renderer_functions = []() -> bool {
    if (g_check_functions_initialized)
        return true;
    temp::TemplateExpression::AddFunction("is_xobject", [](const xlist& args, const xdict&, const XResources::ptr&) {
        static const regex x_object_regex(R"(^\w+::ptr$)");
        string data_type_name = xvar(args[0]).get<string>();
        return regex_match(data_type_name, x_object_regex);
    });
    return g_check_functions_initialized = true;
};


XMetaObjectCompiler::CompileError::CompileError(const string& what) :
	XException(what),
	m_filePath(),
	m_line(-1)
{
}

XMetaObjectCompiler::CompileError::CompileError(const string& what, const string& file_path, int line) :
	XMetaObjectCompiler::CompileError(what)
{
	m_filePath = file_path;
	m_line = line;
}


XMetaObjectCompiler::XMetaObjectCompiler(const std::string& project_name, const std::vector<xdev::filesystem::path>& files) :
    m_projectName(project_name),
	m_files(files),
	m_macroArgs(R"((\s{0,}([\w\d]+)\s{0,},?){0,})"),
    m_headerTemplate(xtemplate::CompileResource("templates/moc.h.xtf",
                                                XdevMocLibResources)),
    m_sourceTemplate(xtemplate::CompileResource("templates/moc.cpp.xtf",
                                                XdevMocLibResources)),
    m_poolsHeaderTemplate(xtemplate::CompileResource("templates/pools.h.xtf",
                                                     XdevMocLibResources)),
    m_poolsSourceTemplate(xtemplate::CompileResource("templates/pools.cpp.xtf",
                                                     XdevMocLibResources)),
    m_pools(xdict()),
    m_poolsIncludes(xlist())
{
    g_check_functions_initialized = register_renderer_functions();
}

string::const_iterator get_class_body_start(const string::const_iterator& begin, const string::const_iterator& end)
{
	return find_if(begin, end, [](char c) { return c == '{'; });
}

pair<string::const_iterator, string::const_iterator> get_class_body(
	const string::const_iterator& begin,
	const string::const_iterator& end)
{
	pair<string::const_iterator, string::const_iterator> result;
	int brace_counter = 1;
	result.first = find_if(begin, end, [](char c) { return c == '{'; });
	result.second = find_if(result.first + 1, end, [&brace_counter](char c)
	{
		if (c == '{')
			++brace_counter;
		else if (c == '}')
			--brace_counter;
		if (brace_counter <= 0)
			return true;
		return false;
	}) + 1;
	return result;
}

template <typename IterT, typename TokenT>
auto find_scope_end(IterT start, IterT end, TokenT open_tok, TokenT close_tok) {
    TokenT toks[] = {open_tok, close_tok};
    int counter = 0;
    auto it = start;
    do {
        it = find_first_of(it, end, toks, toks + sizeof(toks) / sizeof(toks[0]));
        if (*it == open_tok) {
            ++counter;
        } else if (*it == close_tok) {
            --counter;
        } else {
            return end;
        }
        ++it;
    } while (counter > 0);
    return it;
}

string get_class_body(const std::string& filecontent, size_t startpos)
{
	auto p = get_class_body(filecontent.begin() + ssize_t(startpos), filecontent.end());
	return string(p.first, p.second);
}

xdict xmacro_extract_args(const string& raw_args)
{
    xdict args;
	static const regex extract_expr(R"(\s*(\w+)\s*=\s*(\((.|\s)+\)|[\w\s\"\']+)\s*)");
	tools::regex_foreach(raw_args, extract_expr, [&](const smatch& match)
	{
        string name = match[1];
        string data = match[2];
		if (data[0] == '(')
		{
			args[name] = xmacro_extract_args(data);
		}
		else
		{
			if (data[0] == '"')
                data.erase(0, 1);
			if (data[data.size() - 1] == '"')
                data.erase(data.size() - 1, 1);
			args[name] = data;
		}
	});
	return args;
}

inline string read_file(const fs::path& path)
{
    ifstream input(path.string());
    return string(istreambuf_iterator<char>(input), istreambuf_iterator<char>());
}

void XMetaObjectCompiler::registerPools(const fs::path& path)
{
    string input_file = path.string();
    string file_content = read_file(path);
    static const regex xregisterpool_expr(R"(XPOOLREGISTER\(\s{0,}(\w+)\s{0,},\s{0,}Access\s{0,}=\s{0,}(\w+)\s{0,},\s{0,}Macro\s{0,}=\s{0,}(\w+)\))");
    tools::regex_foreach(file_content, xregisterpool_expr, [&](auto& match)
    {
        xdict pool_def;
        pool_def["name"] = string(match[1]);
        pool_def["access"] = string(match[2]);
        pool_def["macro"] = string(match[3]);
        pool_def["items"] = xlist();
        m_pools.get<xdict>()[pool_def["access"].get<string>()] = pool_def;
    });
    m_poolsIncludes.get<xlist>().push(path.filename().string());
}

void XMetaObjectCompiler::createPools()
{
    xdict context {
        { "basename", m_projectName + "-pools.xdev"s },
        { "includes", m_poolsIncludes.get<xlist>() },
        { "pools", m_pools.get<xdict>() },
    };
    cout << "-- pools: " << m_pools.toString() << endl;
    ofstream(m_projectName + "-pools.xdev.hpp") << m_poolsHeaderTemplate->process(context);
    ofstream(m_projectName + "-pools.xdev.cpp") << m_poolsSourceTemplate->process(context);
}

void XMetaObjectCompiler::processPools(const fs::path& path)
{
    string file_content = read_file(path);

    list<string> matching_macros{ "XCLASS" };

    for (const auto& item : m_pools.get<xdict>())
    {
        matching_macros.push_back(item.second.get<xdict>().at("macro").get<string>());
    }

    regex xclass_expr("(" + tools::join(matching_macros, "|") + R"()\((.+)?\)[.\s]+class\s+(\w+)\s{0,}:((\s{0,}(public|protected|private)\s+([\w:<>]+)\s{0,},?)+))");

    tools::regex_foreach(file_content, xclass_expr, [&](auto& match)
    {
        string macro = match[1];
        string class_name = match[3];
        for (auto& item : m_pools.get<xdict>())
        {
            xdict& pool_dict = item.second.get<xdict>();
            if (pool_dict["macro"].get<string>() == macro)
            {
                pool_dict["items"].get<xlist>().push(xdict{
                    { "class", class_name }
                    });
                break;
            }
        }
    });
}

void XMetaObjectCompiler::processFile(const fs::path& path) try
{
	string input_file = path.string();
    string basename = regex_replace(path.filename().string(), regex(R"(\.h([p\+]+)?)"), "");
	string header_file = basename + ".xdev.hpp";
	string source_file = basename + ".xdev.cpp";
    string origin_filename = basename + path.extension().string();

	string file_content = read_file(path);

    xdict context;
	context["basename"] = basename;
    if (m_pools.get<xdict>().size())
    {
        context["pools"] = m_pools.get<xdict>();
    }
    xlist xclasses;

    list<string> matching_macros { "XCLASS", "XAPPLICATION" };

    for (const auto& item : m_pools.get<xdict>())
    {
        matching_macros.push_back(item.second.get<xdict>().at("macro").get<string>());
    }

    // X\(((?:[^()]++)*)\)\s*(\w+)\s*[{;]

    regex x_regex(R"(X\(\s*([\w <>()(::)]+)\s*(,\s*((?:[^()]+)*))?\)\s*(\w+)(\s*=\s*(.*))?\s*;)");
    regex x_class_regex(R"(X\(\s*(class|struct)\s*(,\s*((?:[^()]+)*))?\)\s*(\w+)\s*:((\s*(public|protected|private)\s+((xdev::)?(\w+)<(\w+)>)\s*,?)+))");

    xlist class_defs;

    tools::regex_foreach(file_content, x_class_regex, [&](auto& match)
    {
        xdict class_def = {
            {"functions", xlist{}},
            {"events", xlist{}},
            {"properties", xlist{}},
            {"invokables", xlist{}},
            // this will probably be removed
            {"cpp_template", "templates/xclass.cpp.xtf"},
            {"hdr_template", "templates/xclass.h.xtf"}
        };
        cout << match[0] << endl;
        class_def["name"] = match[4];
        string class_name_check = match[11];
        if (class_def["name"] != class_name_check) {
            throw XException(class_def["name"].get<string>() +
                    " must inherit from template xobj parent (eg.: xobj<" + class_def["name"].get<string>() + ">)" );
        }
        class_def["base"] = match[10];
        cout << "-- xdev class found: " << class_def["name"] << endl;
        auto body_start = match[0].second;
        auto body_end = find_scope_end(body_start, file_content.cend(), '{', '}');
        string body{body_start, body_end + 1}; // + 1 = keep ';'
        tools::regex_foreach(body, regex(R"(property\s*<([\w\s,<>:]*)>\s+(\w+).*(?=;))"), [&](auto& match) {
            class_def["properties"].get<xlist>().push(xdict{
                {"raw_args", match[1]},
                {"name", match[2]},
            });
        });
        tools::regex_foreach(body, regex(R"(function\s*<([\w\s,<>():]*)>\s+([\w:]+)(\s*=\s*[\w\d]+)?;)"), [&](auto& match) {
            class_def["functions"].get<xlist>().push(xdict{
                {"raw_args", match[1]},
                {"name", match[2]},
            });
        });
        tools::regex_foreach(body, regex(R"(event\s*<([\w\s,<>:]*)>\s+([\w:]+)(\s*=\s*[\w\d]+)?;)"), [&](auto& match) {
            class_def["events"].get<xlist>().push(xdict{
                {"raw_args", match[1]},
                {"name", match[2]},
            });
        });
        tools::regex_foreach(body, regex(R"(XINVOKABLE\s+([\w:]+)\s+(\w+)\s*\(\s*([\w <>():&,]+)?\)\s*[;{])"), [&](auto& match) {
            xlist args;
            tools::regex_foreach(match[3], regex(R"(,?\s*((const)?\s*([\w:<>,]+)&?)\s*(\w+)?)"), [&](auto& match) {
                string name = match[4].matched ? match[4] : "arg" + to_string(args.size());
                args.push(xdict{
                              {"type", match[1]},
                              {"clean_type", match[3]},
                              {"name", name},
                          });
            });
            string args_decl = tools::join(args, ", ", [](auto& v) {
                    return v.template get<xdict>().at("type").toString() + " " + v.template get<xdict>().at("name").toString();
            });
            string args_types = tools::join(args, ", ", [](auto& v) {
                    return v.template get<xdict>().at("clean_type").toString();
            });
            string args_names = tools::join(args, ", ", [](auto& v) {
                    return v.template get<xdict>().at("name").toString();
            });
            class_def["invokables"].get<xlist>().push(xdict{
                {"name", match[2]},
                {"return_type", match[1]},
                {"args", args},
                {"args_decl", args_decl},
                {"args_types", args_types},
                {"args_names", args_names},
            });
        });

        static const regex metadata_expr(R"(XMETADATA\((\w+)\s{0,},\s{0,}([^)]+))");
        xdict metadata;
        tools::regex_foreach(body, metadata_expr, [&](auto& match) {
            metadata[match[1]] = xvar::FromJSON(match[2]);
            cout << "-- metadata found: " << match[1] << " = " << metadata[match[1]] << endl;
        });

        class_def["metadata"] = metadata;
        cout << "-- metadata: " << metadata << endl;

        class_defs.push(class_def);
    });

    context["xclasses"] = class_defs;

	// cout << XVariant(context).toString() << endl;

	cout << "-- generating: " << header_file << endl;
	context["filename"] = header_file;
    context["origin_filename"] = origin_filename;
    ofstream(header_file) << m_headerTemplate->process(context);

	cout << "-- generating: " << source_file << endl;
	context["filename"] = source_file;
    ofstream(source_file) << m_sourceTemplate->process(context);
} catch (const std::regex_error & err) {
    std::cerr << err.what() << std::endl;
}

void XMetaObjectCompiler::compile()
{
    // find pool definitions
    for (const auto& path: m_files)
    {
        registerPools(path);
    }

    // process pools
    for (const auto& path: m_files)
    {
        processPools(path);
    }

    // process file
    for (const auto& path: m_files)
    {
        processFile(path);
    }

    createPools();
}
