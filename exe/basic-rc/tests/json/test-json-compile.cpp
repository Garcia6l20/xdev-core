#include <catch2/catch.hpp>

#include <xdev/basic-rc.hpp>

using namespace xdev;
using namespace xdev::rc;

inline filesystem::path source_dir() {
    return filesystem::path(__FILE__).parent_path();
}

TEST_CASE("JSonResources.Compilation") {
    XBasicResourceCompiler("test-json", source_dir() / "resources").compile();
    REQUIRE(42.0 == 42.0);
}
