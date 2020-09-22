/**
 * @file xdev-yaml.hpp
 */
#pragma once

#include <xdev/variant.hpp>

namespace xdev::yaml {

/**
 * @brief Yaml Parser class
 */
class Parser {
public:
    inline Parser(const std::string& data);
    inline xvar operator()();
private:

    const std::string& _data;
};

inline xvar parse(const std::string& data) {
    return Parser{data}();
}

} // xdev::yaml

namespace xdev {

inline xdev::xvar operator "" _xyaml(const char* data, size_t size) {
    return xdev::yaml::parse(std::string{data, size});
}

} // xdev

#include <xdev/yaml.inl>
