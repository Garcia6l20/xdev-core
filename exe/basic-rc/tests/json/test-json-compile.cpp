#include <xdev/xdev-basic-rc.hpp>
#include <gtest/gtest.h>

using namespace xdev;
using namespace xdev::rc;

inline filesystem::path source_dir() {
    return filesystem::path(__FILE__).parent_path();
}

TEST(JSonResources, Compilation) {
    XBasicResourceCompiler("test-json", source_dir() / "resources").compile();
    ASSERT_EQ(42.0, 42.0);
}
