#include <fmt/color.h>
#include <fmt/format.h>
#include <xdev/xdev-base64.hpp>
#include <xdev/xdev-basic-rc.hpp>
#include <xdev/xdev-core.hpp>
#include <xdev/xdev-tools.hpp>

#include <fstream>
#include <iostream>

using namespace xdev;
using namespace xdev::rc;

namespace fs = xdev::filesystem;

class scoped_file
{
public:
  static scoped_file open(std::filesystem::path path, std::string_view mode = default_mode)
  {
    auto guard = scoped_file(std::move(path));
    guard.open(mode);
    return guard;
  }

  scoped_file() = delete;

  scoped_file(const scoped_file &) = delete;
  scoped_file &operator=(const scoped_file &) = delete;

  scoped_file(scoped_file &&other) : _path{ std::move(other._path) }, _file{ other._file } { other._file = nullptr; }
  scoped_file &operator=(scoped_file &&other)
  {
    _path = std::move(other._path);
    _file = other._file;
    other._file = nullptr;
    return *this;
  }

  ~scoped_file()
  {
    if (_file) fclose(_file);
  }

  std::FILE *operator*() const { return _file; }

private:
  scoped_file(std::filesystem::path path) : _path{ path } {}

  void open(std::string_view mode)
  {
    _file = fopen(_path.c_str(), mode.data());
    if (not _file) { throw std::runtime_error("unable to open " + _path.string()); }
  }

  static const std::string_view default_mode;
  std::filesystem::path _path;
  std::FILE *_file = nullptr;
};

const std::string_view scoped_file::default_mode = "w+";

XBasicResourceCompiler::XBasicResourceCompiler(const string &name, const fs::path &path, bool verbose) :
  m_name(name), m_path(path), _verbose(verbose)
{}

void XBasicResourceCompiler::processFile(const fs::path &path, const string &root)
{
  string input_file = path.string();
  string basename = path.filename().string();

  if (!root.empty()) {
    fs::create_directories(root);
    basename = root + "/" + basename;
  }

  fs::path resource_file = basename + ".xrc";
  string variable_name = regex_replace(basename, regex(R"([-\./(\\)])"), "_");
  ifstream input(path.string());

  fmt::print(fg(fmt::color::peru), "-- generating: {}\n", resource_file.c_str());
  if (_verbose) fmt::print(fg(fmt::color::peru), "-- working directory: {}\n", fs::current_path().c_str());

  {
    auto file = scoped_file::open(resource_file);
    fmt::print(*file,
      "// key: {}\n"
      "const uint8_t {}_data[] = {{",
      basename,
      variable_name);

    char tmp_dump[2];
    for_each(istreambuf_iterator<char>(input), istreambuf_iterator<char>(), [&file, &tmp_dump](char c) {
      tools::hex_dump(static_cast<uint8_t>(c), tmp_dump);
      fmt::print(*file, "0x{},", std::string_view{ tmp_dump, sizeof(tmp_dump) });
    });
    fmt::print(*file, "}};\n");

    fmt::print(*file,
      "const size_t {v}_len = sizeof({v}_data);\n"
      "const xdev::XResources::resource_data_t {v} = {{"
      "{v}_data, {v}_len"
      "}};\n",
      fmt::arg("v", variable_name));
  }

  rc_map mapping;
  mapping.key = basename;
  mapping.resource_path = resource_file;
  mapping.resource_name = variable_name;
  m_rcMaps.push_back(mapping);
}

#include <algorithm>
void XBasicResourceCompiler::createResourceFiles()
{
  string cpp_filename = m_name + "-resources.cpp";
  string h_filename = m_name + "-resources.hpp";
  string base_name = regex_replace(m_name, regex(R"([-\.])"), "_");
  string header_lock_name = base_name;
  tools::to_upper(header_lock_name);
  tools::title(base_name);
  base_name.erase(std::remove(base_name.begin(), base_name.end(), '_'), base_name.end());

  {
    auto header = scoped_file::open(fs::path(h_filename.c_str()));
    fmt::print(*header,
      "#pragma once\n\n"
      "#include <xdev/xdev-resources.hpp>\n\n"
      "extern const xdev::XResources::ptr {}Resources;\n",
      base_name);
  }

  {
    auto cpp = scoped_file::open(fs::path(cpp_filename.c_str()));
    fmt::print(*cpp,
      "#include <{}>\n\n"
      "using namespace xdev;\n\n",
      h_filename);
    for (const rc_map &item : m_rcMaps) { fmt::print(*cpp, "#include <{}>\n", item.resource_path); }
    fmt::print(*cpp,
      "\n"
      "using namespace xdev;\n\n"
      "static XResources::ptr _load_resources() {{\n"
      "    XResources::ptr resources = XResources::Make();");
    for (const rc_map &item : m_rcMaps) {
      fmt::print(*cpp, "    resources->add(\"{}\", {});\n", item.key, item.resource_name);
    }
    fmt::print(*cpp,
      "    return resources;\n"
      "}}\n\n"
      "const XResources::ptr {}Resources = _load_resources();",
      base_name);
  }
}

void XBasicResourceCompiler::compile()
{
  if (!fs::exists(m_path)) { throw CompileError(std::string() + "Error: path does not exist: " + m_path.string()); }

  fmt::print(fg(fmt::color::peru), "Generating ressources for: {}\n", m_name);

  tools::walk_directory(m_path, [&](const fs::path &path, const fs::path &root) {
    auto relative = root.string();
    relative.erase(relative.find(m_path.string()), m_path.string().size());
    relative = regex_replace(relative, regex(R"(\\)"), "/");
    if (relative[0] == '/') relative.erase(0, 1);
    processFile(path, relative);
  });

  createResourceFiles();
}
