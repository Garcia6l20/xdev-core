#include <catch2/catch.hpp>

#include <xdev/xdev.hpp>
#include <cstring>

using namespace xdev;


struct Tester {
    std::variant<int, std::string> _value;
    //int data;
    //auto operator<=>(const Tester& other) const = default;
    bool operator==(const Tester& b) const {
        return tools::visit_2way(tools::overloaded{
           []<typename T>(const T& lhs, const T& rhs){
               return lhs == rhs;
           },
           []<typename T, typename U>(const T&, const U&){
               return false;
           },
           [](const std::string& lhs, const std::string& rhs){
              return lhs.size() == rhs.size() &&
                std::strcmp(lhs.c_str(), rhs.c_str()) == 0;
           }
        }, _value, b._value);
    }
    auto operator<=>(const Tester& b) const {
        return tools::visit_2way(tools::overloaded{
           []<typename T, typename U>(const T&lhs, const U&rhs){
               return lhs <=> rhs;
           },
          []<typename T>(const T&, const std::string&){
              return std::strong_ordering::less;
          },
          []<typename T>(const std::string&, const T&){
              return std::strong_ordering::less;
          },
           [](const std::string& lhs, const std::string& rhs){
              return std::strcmp(lhs.c_str(), rhs.c_str()) <=> 0;
           }
        }, _value, b._value);
    }

    bool operator==(char const* b) const {
        return std::visit(tools::overloaded{
           []<typename T>(const T&){
               return false;
           },
           [b](const std::string& lhs){
              return std::strcmp(lhs.c_str(), b) == 0;
           }
        }, _value);
    }
    auto operator<=>(const char* b) const {
        return std::visit(tools::overloaded{
           []<typename T>(const T&){
              return std::strong_ordering::less;
           },
           [b](const std::string& lhs){
              return std::strcmp(lhs.c_str(), b) <=> 0;
           }
        }, _value);
    }
};

SCENARIO("basic type are accessible as normal types", "[core.api.variant.v1.0]") {

    Tester test{"hello"};
    REQUIRE(test > Tester{"12"});
//    Tester test{11};
//    REQUIRE(test < Tester{12});

    GIVEN("An integer variant") {
        XVariant var = 42;
        WHEN("a new value is assigned") {
            var = 43;
            THEN("the variant is updated with the new value") {
                REQUIRE(var == 43);
            }
        }
        WHEN("the value is less compared") {
            auto is_less_true = var < 100;
            auto is_less_false = var < 0;
//            auto is_less_rev_true = 0 < var;
//            auto is_less_rev_false = 100 < var;
            THEN("the comparaison results shoul be exact") {
                REQUIRE(is_less_true == true);
                REQUIRE(is_less_false == false);
//                REQUIRE(is_less_rev_true == true);
//                REQUIRE(is_less_rev_false == false);
            }
        }
        WHEN("it is post-incremented") {
            auto returned = var++;
            THEN("the variant is incremented and the returned value is the previous one") {
                REQUIRE(var == 43);
                REQUIRE(returned == 42);
            }
        }
        WHEN("it is pre-incremented") {
            auto returned = ++var;
            THEN("the variant is incremented and the returned value is the variant it-self") {
                REQUIRE(var == 43);
                REQUIRE(returned == var);
            }
        }
        WHEN("it is post-decremented") {
            auto returned = var--;
            THEN("the variant is decremented and the returned value is the previous one") {
                REQUIRE(var == 41);
                REQUIRE(returned == 42);
            }
        }
        WHEN("it is pre-decremented") {
            auto& returned = --var;
            THEN("the variant is decremented and the returned value is the variant it-self") {
                REQUIRE(var == 41);
                REQUIRE(returned == var);
            }
        }
    }
}
