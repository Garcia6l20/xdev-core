/**
 * @file xdev-json.hpp
 */
#pragma once

#include <xdev/xdev-variant.hpp>

namespace xdev::json {

/**
 * @brief Json Parser class
 */
class Parser
{
public:
    inline XVariant operator()(const std::string& data);
private:
    template <typename IterT>
    static XVariant parse_object(IterT& begin, const IterT& end);
    template <typename IterT>
    static XVariant parse_array(IterT& begin, const IterT& end);
    template <typename IterT>
    static XVariant parse_value(IterT& begin, const IterT& end);
    template <typename IterT>
    static std::string parse_string(IterT& begin, const IterT& end);
    template <typename IterT>
    static XVariant parse_number(IterT& begin, const IterT& end);
};

inline
XVariant parse(const std::string& data)
{
    return Parser{}(data);
}

} // xdev::json

inline xdev::XVariant operator "" _xjson(const char* data, size_t size) {
    return xdev::json::parse(std::string{data, size});
}

#include <xdev/xdev-json.inl>
