#include <xdev/xdev-object.hpp>
#include <gtest/gtest.h>

using namespace xdev;

TEST(Properties, Access) {
    property<int> intProp = 2;
    ASSERT_EQ(intProp, 2);
}

TEST(Properties, Listen) {
    property<int> intProp = 2;
    auto lobj = intProp.listen([&](int value) {
        ASSERT_EQ(intProp, value);
    });
    ASSERT_EQ(intProp, 2);
}

TEST(Properties, ScopedListen) {
    property<int> intProp = 2;
    auto guard = intProp.listen([&](int value) {
        ASSERT_EQ(intProp, value);
    });
    intProp = 3;
    {
        auto inner_guard = intProp.listen([&](int value) {
            ASSERT_EQ(intProp, value);
        });
        intProp = 4;
    }
    intProp = 5;
    ASSERT_EQ(intProp, 5);
}

TEST(Properties, StopListen) {
    property<int> intProp = 2;
    bool first = true;
    auto lobj = intProp.listen([&](int/*value*/) {
        ASSERT_TRUE(first);
        first = false;
    });
    intProp.stopListening(lobj);
    intProp = 3;
    ASSERT_EQ(intProp, 3);
}
