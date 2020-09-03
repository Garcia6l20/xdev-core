#include <catch2/catch.hpp>

#include <xdev/xdev.hpp>
#include <xdev/xdev-moc.hpp>

#include <fmt/format.h>
#include <spdlog/spdlog.h>

using namespace xdev;

inline filesystem::path source_dir() {
  return filesystem::path(__FILE__).parent_path();
}

TEST_CASE("MOCBasicObject.Compilation") {
  auto config = fmt::format(R"(
project_name: test
target: test
source_dir: {}
bin_dir: .
include_dirs: []
headers:
- full_path: {}
  path: {}
  name: test
  directory: .
)",
    source_dir().string(),
    (source_dir() / "test_object.h").string(),
    "test_object.h");
  auto data = yaml::parse(config);
  spdlog::set_level(spdlog::level::debug);
  spdlog::info("config: {}", data);
  XMetaObjectCompiler compiler(data.get<xdict>());
  compiler.compile();
}
