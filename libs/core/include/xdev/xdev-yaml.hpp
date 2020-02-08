/**
 * @file xdev-yaml.hpp
 */
#pragma once

#include <xdev/xdev-variant.hpp>

namespace xdev::yaml {

/**
 * @brief Yaml Parser class
 */
class Parser
{
public:
    Parser(const std::string& data);
    inline XVariant operator()();
private:

    const std::string& _data;
};

inline
XVariant parse(const std::string& data)
{
    return Parser{data}();
}

} // xdev::yaml

#include "../inline/xdev-yaml.inl"
