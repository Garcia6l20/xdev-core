#include <xdev/xdev-object.hpp>
#include <gtest/gtest.h>

using namespace xdev;

TEST(Properties, Access) {
    {
        property<int> intProp = 2;
        ASSERT_EQ(intProp, 2);
        int& propRef = *intProp;
        ASSERT_EQ(++propRef, 3);
    }

    {
        property<int, PropertyAccess::ReadOnly> roProp = 2;
        // ASSERT_EQ(*roProp, 3); // nope property should be const
        // ASSERT_THROW(roProp = 3, XPropertyBase::IllegalAccess); // does not compile (ro property)
        ASSERT_THROW(roProp = XVariant{3}, XPropertyBase::IllegalAccess); // Variant-based access checked at runtime
        // int& recRoProp = *roProp; // does not compile (ro property)
        const int& crefRoProp = *roProp.as_const(); // const acces OK
        ASSERT_EQ(crefRoProp, 3);
    }

    {
        const property<int, PropertyAccess::ReadOnly> croProp = 2;
        ASSERT_EQ(*croProp, 3);
        auto crefRoProp = *croProp;
        ASSERT_EQ(crefRoProp, 3);
    }
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
