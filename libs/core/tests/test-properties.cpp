#include <xdev/xdev-object.hpp>
#include <catch2/catch.hpp>

using namespace xdev;

//template <typename T>
//T& operator =(T&ref, property<T>& prop) {
//    ref = *prop;
//    return ref;
//}

SCENARIO("properties should have access rules") {

    GIVEN("an integer property") {
        property<int> intProp = 2;
        REQUIRE(intProp == 2);
        WHEN("a reference is picked from it") {
            int& propRef = *intProp;
            THEN("im able to increment this property") {
                REQUIRE(++propRef == 3);
            }
        }
    }

    GIVEN("a non-constant readonly property") {
        property<int, PropertyAccess::ReadOnly> roProp = 2;
        // REQUIRES(*roProp == 3); // nope property should be const
        // REQUIRE_THROWS(roProp = 3, XPropertyBase::IllegalAccess); // does not compile (ro property)
        auto bad_access = [&]() { roProp = XVariant{3}; };
        REQUIRE_THROWS_AS(bad_access(), XPropertyBase::IllegalAccess); // Variant-based access checked at runtime
        // int& recRoProp = *roProp; // does not compile (ro property)
        const int& crefRoProp = *roProp.as_const(); // const acces OK
        REQUIRE(crefRoProp == 2);
    }

    GIVEN("a constant readonly property") {
        const property<int, PropertyAccess::ReadOnly> croProp = 2;
        REQUIRE(*croProp == 2);
        auto crefRoProp = *croProp;
        REQUIRE(crefRoProp == 2);
    }
}

SCENARIO("properties should be lisenable") {

    GIVEN("an integer property") {
        property<int> intProp = 2;
        WHEN("a listener is registered") {
            int count = 0;
            auto guard = intProp.listen([&](int value) {
                REQUIRE(intProp == value);
                ++count;
            });
            THEN("the listener should be notfified of any changes") {
                REQUIRE(count == 1);
                intProp = 2;
                REQUIRE(count == 2);
                intProp = 55;
                REQUIRE(count == 3);
                intProp = 42;
                REQUIRE(count == 4);
            }
        }
    }

    GIVEN("an integer property with a listener") {
        property<int> intProp = 2;
        int count = 0;
        auto guard = intProp.listen([&](int value) {
            REQUIRE(intProp == value);
            ++count;
        });
        REQUIRE(count == 1);
        WHEN("the guard is destroyed") {
            guard = {};
            THEN("the listener should not receive any notification") {
                intProp = 2;
                REQUIRE(count == 1);
            }
        }
    }
}
