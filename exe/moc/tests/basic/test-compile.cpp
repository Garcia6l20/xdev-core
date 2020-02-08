#include <xdev.hpp>
#include <xdev/xdev-moc.hpp>
#include <gtest/gtest.h>

using namespace xdev;

inline filesystem::path source_dir() {
    return filesystem::path(__FILE__).parent_path();
}

TEST(MOCBasicObject, Compilation) {
    XMetaObjectCompiler compiler("MOCBasicObject-Compilation-Test" , {source_dir() / "test_object.h"});
    compiler.compile();
}
