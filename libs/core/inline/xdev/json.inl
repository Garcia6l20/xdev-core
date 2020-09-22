#include <xdev/json.hpp>

#include <iostream>
#include <algorithm>
#include <unordered_map>

#include <stack>

namespace xdev {
namespace json {

template <typename IterT>
inline IterT it_find(const IterT& begin, const IterT& end, char c)
{
    auto ii = begin;
    for (; ii != end && *ii != c; ++ii) {}
    return ii;
}

template <typename IterT>
xvar Parser::parse_object(IterT& begin, const IterT& end)
{
    xdict result;
    for (; begin != end && *begin != '}'; ++begin) {
        if (!isgraph(*begin))
            continue;
        begin = it_find(begin, end, '"') + 1;
        std::string key = parse_string(begin, end);
        begin = it_find(begin, end, ':') + 1;
        result[key] = parse_value(begin, end);
    }
    return result;
}

template <typename IterT>
xvar Parser::parse_array(IterT& begin, const IterT& end)
{
    xlist result;
    for (; *begin != ']' && begin != end; ++begin) {
        if (!isgraph(*begin))
            continue;
        if (*begin == ',')
            continue;
        result.push(parse_value(begin, end));
    }
    return result;
}

template <typename IterT>
std::string Parser::parse_string(IterT& begin, const IterT& end)
{
    std::string result;
    for (; begin != end; ++begin) {
        switch (*begin) {
        case '"':
            return result;
        case '\\': {
            // handle escape char
            switch (*++begin) {
            case 'n':
                result += '\n'; break;
            case '\\':
                result += '\\'; break;
            case '/':
                result += '/'; break;
            case 'b':
                result += '\b'; break;
            case 'f':
                result += '\f'; break;
            case 'r':
                result += '\r'; break;
            case 't':
                result += '\t'; break;
            case 'u':
                throw std::runtime_error("Unicode not implemented yet !!");
            default:
                throw std::runtime_error(fmt::format("Unknown escape character: {}", *begin));
            }
            break;
        }
        default:
            if (isprint(*begin))
                result += *begin;
        }
    }
    throw std::runtime_error("Error occured while parsing string");
}

template <typename IterT>
xvar Parser::parse_number(IterT& begin, const IterT& end)
{
    double result = 0.;
    ssize_t sz = 0;
    result = std::stod(std::string(begin, end), reinterpret_cast<size_t*>(&sz));
    begin += sz - 1;
    if (sz == 0)
        throw std::runtime_error("Error occured while parsing number");
    return result;
}

template <typename IterT>
xvar Parser::parse_value(IterT& begin, const IterT& end)
{
    for (; begin != end; ++begin) {
        switch (*begin) {
        case ' ':
        case '\n':
        case '\t':
            continue;
        case 't': // assuming true
            begin += 3;
            return true;
        case 'f': // assuming false
            begin += 4;
            return false;
        case 'n': // assuming null
            begin += 3;
            return xvar();
        case '{':
            return parse_object(++begin, end);
        case '[':
            return parse_array(++begin, end);
        case '"':
            return parse_string(++begin, end);
        default:
            return parse_number(begin, end);
        }
    }
    throw std::runtime_error("Error occured while parsing value");
}

xvar Parser::operator()(const std::string& data)
{
    auto begin = data.begin();
    return parse_value(begin, data.end());
}

}
}
