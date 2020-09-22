#include <xdev/moc.hpp>

#include <lyra/lyra.hpp>
#include <fmt/format.h>
#include <fmt/color.h>
#include <spdlog/spdlog.h>

#include <iostream>

using namespace xdev;

int main(int argc, char **argv) {
  bool help = false;
  bool verbose = false;
  std::string data;
  auto cli = lyra::help(help) | lyra::arg(data, "data")("Metadata about headers to be mocced");

  auto res = cli.parse({argc, argv});
  spdlog::info("arguments: {}", xvar {xlist {std::vector<std::string> {argv + 1, argv + argc}}});
  if (!res) {
    fmt::print(fg(fmt::color::red), "error: {}\n", res.errorMessage());
    return -1;
  }

  if (help) {
    std::cout << cli << '\n';
    return 0;
  }

  auto meta = xvar::FromJSON(data);
  spdlog::info("metadata: {}", meta);

  XMetaObjectCompiler compiler(meta.get<xdict>());
  try {
    compiler.compile();
  } catch (const XMetaObjectCompiler::CompileError &error) {
    fmt::print(stderr, fg(fmt::color::red), "compilation error: {}\n", error.what());
    return -1;
  }
  return 0;
}
