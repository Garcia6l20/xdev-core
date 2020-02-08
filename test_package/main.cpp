#include <xdev/xdev.hpp>

int main(int argc, char** argv) {
    auto d = xdev::XDict{
        {"hello", "conan"}
    };
    std::cout << d.toString() << std::endl;
    auto v = xdev::XVariant::FromJSON(R"({
        "hello": "conan"
    })");
    std::cout << v.toString() << std::endl;
    return 0;
}
