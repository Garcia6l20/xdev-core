#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <regex>

#include <xdev-moc-lib-resources.hpp>
#include <xdev/xdev-moc.hpp>
#include <xdev/xdev-template-expressions.hpp>
#include <xdev/xdev-tools.hpp>

#include <spdlog/spdlog.h>

using namespace xdev;
namespace fs = xdev::filesystem;

static bool g_check_functions_initialized = false;

auto register_renderer_functions = []() -> bool {
  if (g_check_functions_initialized)
    return true;
  temp::TemplateExpression::AddFunction("is_xobject", [](const xlist &args, const xdict &, const XResources::ptr &) {
    static const regex x_object_regex(R"(^\w+::ptr$)");
    string data_type_name = xvar(args[0]).get<string>();
    return regex_match(data_type_name, x_object_regex);
  });
  return g_check_functions_initialized = true;
};


XMetaObjectCompiler::CompileError::CompileError(const string &what) : XException(what), _file_path(), _line(-1) {
}

XMetaObjectCompiler::CompileError::CompileError(const string &what, const string &file_path, int line) :
  XMetaObjectCompiler::CompileError(what) {
  _file_path = file_path;
  _line = line;
}


XMetaObjectCompiler::XMetaObjectCompiler(const xdict &metadata) :
  _project_name {metadata.at("target").get<std::string>()},
  _src_path {metadata.at("source_dir").get<std::string>()},
  _bin_path {metadata.at("bin_dir").get<std::string>()},
  _metadata {metadata},
  _macroArgs(R"((\s{0,}([\w\d]+)\s{0,},?){0,})"),
  _header_template(xtemplate::CompileResource("templates/moc.h.xtf", XdevMocLibResources)),
  _source_template(xtemplate::CompileResource("templates/moc.cpp.xtf", XdevMocLibResources)),
  _pools_header_template(xtemplate::CompileResource("templates/pools.h.xtf", XdevMocLibResources)),
  _pools_source_template(xtemplate::CompileResource("templates/pools.cpp.xtf", XdevMocLibResources)),
  _pools(xdict()),
  _pools_includes(xlist()) {
  g_check_functions_initialized = register_renderer_functions();
}

string::const_iterator get_class_body_start(const string::const_iterator &begin, const string::const_iterator &end) {
  return find_if(begin, end, [](char c) { return c == '{'; });
}

pair<string::const_iterator, string::const_iterator> get_class_body(
  const string::const_iterator &begin, const string::const_iterator &end) {
  pair<string::const_iterator, string::const_iterator> result;
  int brace_counter = 1;
  result.first = find_if(begin, end, [](char c) { return c == '{'; });
  result.second = find_if(result.first + 1, end, [&brace_counter](char c) {
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

template<typename IterT, typename TokenT>
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

string get_class_body(const std::string &filecontent, size_t startpos) {
  auto p = get_class_body(filecontent.begin() + ssize_t(startpos), filecontent.end());
  return string(p.first, p.second);
}

xdict xmacro_extract_args(const string &raw_args) {
  xdict args;
  static const regex extract_expr(R"(\s*(\w+)\s*=\s*(\((.|\s)+\)|[\w\s\"\']+)\s*)");
  tools::regex_foreach(raw_args, extract_expr, [&](const smatch &match) {
    string name = match[1];
    string data = match[2];
    if (data[0] == '(') {
      args[name] = xmacro_extract_args(data);
    } else {
      if (data[0] == '"')
        data.erase(0, 1);
      if (data[data.size() - 1] == '"')
        data.erase(data.size() - 1, 1);
      args[name] = data;
    }
  });
  return args;
}

inline string read_file(const fs::path &path) {
  ifstream input(path.string());
  return string(istreambuf_iterator<char>(input), istreambuf_iterator<char>());
}

void XMetaObjectCompiler::registerPools(xdict &meta) {
  fs::path path = _src_path / meta.at("path").get<std::string>();
  string file_content = read_file(path);
  static const regex xregisterpool_expr(
    R"(XPOOLREGISTER\(\s{0,}(\w+)\s{0,},\s{0,}Access\s{0,}=\s{0,}(\w+)\s{0,},\s{0,}Macro\s{0,}=\s{0,}(\w+)\))");
  tools::regex_foreach(file_content, xregisterpool_expr, [&](auto &match) {
    xdict pool_def;
    pool_def["name"] = string(match[1]);
    pool_def["access"] = string(match[2]);
    pool_def["macro"] = string(match[3]);
    pool_def["items"] = xlist();
    _pools.get<xdict>()[pool_def["access"].get<string>()] = pool_def;
  });
}

void XMetaObjectCompiler::createPools() {
  xdict context {
    {"basename", _project_name + "-pools.xdev"s},
    {"includes", _pools_includes},
    {"pools", _pools.get<xdict>()},
  };
  cout << "-- pools: " << _pools.toString() << endl;
  ofstream(_project_name + "-pools.xdev.hpp") << _pools_header_template->process(context);
  ofstream(_project_name + "-pools.xdev.cpp") << _pools_source_template->process(context);
}

void XMetaObjectCompiler::processPools(xdict &meta) {
  string file_content = read_file(_src_path / meta.at("path").get<std::string>());

  list<string> matching_macros {"XCLASS"};

  for (const auto &item : _pools.get<xdict>()) {
    matching_macros.push_back(item.second.get<xdict>().at("macro").get<string>());
  }

  regex xclass_expr(
    "(" + tools::join(matching_macros, "|")
    + R"()\((.+)?\)[.\s]+class\s+(\w+)\s{0,}:((\s{0,}(public|protected|private)\s+([\w:<>]+)\s{0,},?)+))");

  tools::regex_foreach(file_content, xclass_expr, [&](auto &match) {
    string macro = match[1];
    string class_name = match[3];
    for (auto &item : _pools.get<xdict>()) {
      xdict &pool_dict = item.second.get<xdict>();
      if (pool_dict["macro"].get<string>() == macro) {
        pool_dict["items"].get<xlist>().push(xdict {{"class", class_name}});
        break;
      }
    }
  });
}

bool path_contains_file(fs::path &dir, fs::path file) {
  // If dir ends with "/" and isn't the root directory, then the final
  // component returned by iterators will include "." and will interfere
  // with the std::equal check below, so we strip it before proceeding.
  if (dir.filename() == ".")
    dir.remove_filename();
  // We're also not interested in the file's name.
  assert(file.has_filename());
  file.remove_filename();

  // If dir has more components than file, then file can't possibly
  // reside in dir.
  auto dir_len = std::distance(dir.begin(), dir.end());
  auto file_len = std::distance(file.begin(), file.end());
  if (dir_len > file_len)
    return false;

  // This stops checking when it reaches dir.end(), so it's OK if file
  // has more directory components afterward. They won't be checked.
  return std::equal(dir.begin(), dir.end(), file.begin());
}


void XMetaObjectCompiler::processFile(xdict &meta) try {
  fs::path filepath {meta.at("path").get<std::string>()};

  // skip generated files
  if (path_contains_file(_bin_path, filepath))
    return;

  fs::path path = _src_path / meta.at("path").get<std::string>();
  string basename = meta.at("name").get<std::string>();
  string header_file = basename + ".xdev.hpp";
  string source_file = basename + ".xdev.cpp";
  string origin_filename = filepath.filename();
  for (const auto &include_dir : _metadata.at("include_dirs").get<xlist>()) {
    fs::path dir = include_dir.get<std::string>();
    error_code ec;
    if (not path_contains_file(dir, path))
      continue;
    fs::path tmp = fs::relative(path, dir, ec);
    if (!ec) {
      origin_filename = tmp.string();
      break;
    }
  }
  _pools_includes.push(origin_filename);

  string file_content = read_file(path);
  spdlog::info("path: {}", path.string());
  // spdlog::info("content: {}", file_content);
  spdlog::info("origin_filename: {}", origin_filename);

  xdict context;
  context["basename"] = basename;
  if (_pools.get<xdict>().size()) {
    context["pools"] = _pools.get<xdict>();
  }

  list<string> matching_macros {"XCLASS", "XAPPLICATION"};

  for (const auto &item : _pools.get<xdict>()) {
    matching_macros.push_back(item.second.get<xdict>().at("macro").get<string>());
  }

  // X\(((?:[^()]++)*)\)\s*(\w+)\s*[{;]

  // regex x_regex(R"(X\(\s*([\w <>()(::)]+)\s*(,\s*((?:[^()]+)*))?\)\s*(\w+)(\s*=\s*(.*))?\s*;)");
  regex x_class_regex(
    R"(X\(\s*(class|struct)\s*(,\s*((?:[^()]+)*))?\)\s*(\w+)\s*:((\s*(public|protected|private)\s+((xdev::)?(\w+)<(\w+)>)\s*,?)+))");

  xlist class_defs;

  tools::regex_foreach(file_content, x_class_regex, [&](auto &match) {
    xdict class_def = {{"functions", xlist {}},
      {"events", xlist {}},
      {"properties", xlist {}},
      {"invokables", xlist {}},
      // this will probably be removed
      {"cpp_template", "templates/xclass.cpp.xtf"},
      {"hdr_template", "templates/xclass.h.xtf"}};
    cout << match[0] << endl;
    class_def["name"] = match[4];
    string class_name_check = match[11];
    if (class_def["name"] != class_name_check) {
      throw XException(class_def["name"].get<string>() + " must inherit from template xobj parent (eg.: xobj<"
                       + class_def["name"].get<string>() + ">)");
    }
    class_def["base"] = match[10];
    cout << "-- xdev class found: " << class_def["name"] << endl;
    auto body_start = match[0].second;
    auto body_end = find_scope_end(body_start, file_content.cend(), '{', '}');
    string body {body_start, body_end + 1}; // + 1 = keep ';'
    tools::regex_foreach(body, regex(R"(property\s*<([\w\s,<>:]*)>\s+(\w+).*(?=;))"), [&](auto &match) {
      class_def["properties"].get<xlist>().push(xdict {
        {"raw_args", match[1]},
        {"name", match[2]},
      });
    });
    tools::regex_foreach(body, regex(R"(function\s*<([\w\s,<>():]*)>\s+([\w:]+)(\s*=\s*[\w\d]+)?;)"), [&](auto &match) {
      class_def["functions"].get<xlist>().push(xdict {
        {"raw_args", match[1]},
        {"name", match[2]},
      });
    });
    tools::regex_foreach(body, regex(R"(event\s*<([\w\s,<>:]*)>\s+([\w:]+)(\s*=\s*[\w\d]+)?;)"), [&](auto &match) {
      class_def["events"].get<xlist>().push(xdict {
        {"raw_args", match[1]},
        {"name", match[2]},
      });
    });
    tools::regex_foreach(
      body, regex(R"(XINVOKABLE\s+([\w:]+)\s+(\w+)\s*\(\s*([\w <>():&,]+)?\)\s*[;{])"), [&](auto &match) {
        xlist args;
        tools::regex_foreach(match[3], regex(R"(,?\s*((const)?\s*([\w:<>,]+)&?)\s*(\w+)?)"), [&](auto &match) {
          string name = match[4].matched ? match[4] : "arg" + to_string(args.size());
          args.push(xdict {
            {"type", match[1]},
            {"clean_type", match[3]},
            {"name", name},
          });
        });
        string args_decl = tools::join(args, ", ", [](auto &v) {
          return v.template get<xdict>().at("type").toString() + " " + v.template get<xdict>().at("name").toString();
        });
        string args_types =
          tools::join(args, ", ", [](auto &v) { return v.template get<xdict>().at("clean_type").toString(); });
        string args_names =
          tools::join(args, ", ", [](auto &v) { return v.template get<xdict>().at("name").toString(); });
        class_def["invokables"].get<xlist>().push(xdict {
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
    tools::regex_foreach(body, metadata_expr, [&](auto &match) {
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
  ofstream(header_file) << _header_template->process(context);

  cout << "-- generating: " << source_file << endl;
  context["filename"] = source_file;
  ofstream(source_file) << _source_template->process(context);
} catch (const std::regex_error &err) {
  std::cerr << err.what() << std::endl;
}

void XMetaObjectCompiler::compile() {
  // find pool definitions
  for (auto &meta : _metadata["headers"].get<xlist>()) {
    registerPools(meta.get<xdict>());
  }

  // process pools
  for (auto &meta : _metadata["headers"].get<xlist>()) {
    processPools(meta.get<xdict>());
  }

  // process file
  for (auto &meta : _metadata["headers"].get<xlist>()) {
    processFile(meta.get<xdict>());
  }

  createPools();
}
