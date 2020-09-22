#include <xdev.hpp>
#include <MetaObjectCompiler.h>
#include <gtest/gtest.h>

using namespace xdev;

inline filesystem::path source_dir() {
    return filesystem::path(__FILE__).parent_path();
}

TEST(MOCBasicObject, Compilation) {
    XMetaObjectCompiler(source_dir()).compile();
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
