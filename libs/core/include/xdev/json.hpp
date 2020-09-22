/**
 * @file xdev-json.hpp
 */
#pragma once

#include <xdev/variant.hpp>

namespace xdev::json {

/**
 * @brief Json Parser class
 */
class Parser
{
public:
    inline xvar operator()(const std::string& data);
private:
    template <typename IterT>
    static xvar parse_object(IterT& begin, const IterT& end);
    template <typename IterT>
    static xvar parse_array(IterT& begin, const IterT& end);
    template <typename IterT>
    static xvar parse_value(IterT& begin, const IterT& end);
    template <typename IterT>
    static std::string parse_string(IterT& begin, const IterT& end);
    template <typename IterT>
    static xvar parse_number(IterT& begin, const IterT& end);
};

inline
xvar parse(const std::string& data)
{
    return Parser{}(data);
}

} // xdev::json

namespace xdev {
inline xdev::xvar operator "" _xjson(const char* data, size_t size) {
    return xdev::json::parse(std::string{data, size});
}
} // xdev

#include <xdev/json.inl>
