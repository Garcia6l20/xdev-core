#include <test-lib.hpp>

#include <fmt/format.h>

xdev::property<int> TestObject::InstanceCounter = 0;

void TestObject::call_me() {
    fmt::print("Good !\n");
}

std::string TestObject::say_my_name(const std::string& your_name) {
    fmt::print("Hum... {} !\n", your_name);
    return "Heisenberg ?";
}
