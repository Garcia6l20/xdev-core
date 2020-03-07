#include <xdev/xdev-yaml.hpp>
#include <xdev/xdev-tools.hpp>

#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <unordered_map>
#include <regex>
#include <stack>

namespace xdev {
namespace yaml {

template <typename Iterator, typename Callable>
inline void for_each_lines(const Iterator& begin, const Iterator& end, Callable callable) {
    auto lstart = begin;
    auto lend = std::find(lstart, end, '\n');
    while (lend != end) {
        if (!callable(lstart, ++lend))
            return;
        std::swap(lend, lstart);
        lend = std::find(lstart, end, '\n');
    }
}

template <typename Iterator>
inline ssize_t getindent(Iterator& begin, const Iterator& end) {
    if (*begin == '\n') ++begin;
    return std::distance(begin, std::find_if_not(begin, end, static_cast<int(*)(int)>(std::isspace)));
}

struct Node {
    virtual ~Node() noexcept = default;
    using iterator = std::string::const_iterator;
    virtual void process(iterator &, const iterator &) {};
    static Node Parse(iterator &begin, const iterator &end);
    XVariant value;
};

struct ScalarNode: Node {
    virtual void process(iterator &begin, const iterator &end) {
        auto indent = getindent(begin, end);
        auto valstart = begin + indent;
        static const char end_of_value_tokens[] = {'\n', ' ', ','};
        auto valend = std::find_first_of(valstart, end, end_of_value_tokens, end_of_value_tokens + sizeof(end_of_value_tokens));
        smatch match;
        // null
        if (regex_match(valstart, valend, match, regex(R"(null|Null|NULL|~)"))) {
            value = XNone{};
            begin = valstart + match.length();
            return;
        }
        // boolean
        if (regex_match(valstart, valend, match, regex(R"(true|True|TRUE|false|False|FALSE)"))) {
            value = *valstart == 't' || *valstart == 'T' ? true : false;
            begin = valstart + match.length();
            return;
        }
        // int base 8
        if (regex_match(valstart, valend, match, regex(R"(0o([0-7]+))"))) {
            size_t sz;
            value = stoi(match[1].str(), &sz, 8);
            begin = valstart + match.length();
            return;
        }
        // int base 16
        if (regex_match(valstart, valend, match, regex(R"(0x([0-9a-fA-F]+))"))) {
            size_t sz;
            value = stoi(match[1].str(), &sz, 16);
            begin = valstart + match.length();
            return;
        }
        // int base 10
        if (regex_match(valstart, valend, match, regex(R"([-+]?[0-9]+)"))) {
            value = stoi(match.str());
            begin = valstart + match.length();
            return;
        }
        // float
        if (regex_match(valstart, valend, match, regex(R"([-+]?(\.[0-9]+|[0-9]+(\.[0-9]*)?)([eE][-+]?[0-9]+)?)"))) {
            value = stod(match.str());
            begin = valstart + match.length();
            return;
        }
        // nan
        if (regex_match(valstart, valend, match, regex(R"(\.nan|\.NaN|\.NAN)"))) {
            value = numeric_limits<double>::quiet_NaN();
            begin = valstart + match.length();
            return;
        }
        // infinity
        if (regex_match(valstart, valend, match, regex(R"([-+]?(\.inf|\.Inf|\.INF))"))) {
            value = numeric_limits<double>::infinity();
            begin = valstart + match.length();
            return;
        }
        if (*valstart == '"') {
            ++valstart;
            auto strend = valstart;
            while ((strend = std::find(strend, end, '"')) != end) {
                if (*(strend - 1) != '\\')
                    break;
            }
            value = string(valstart, strend);
            begin = strend + 1;
        } else {
            auto strend = valstart;
            string str;
            while ((strend = std::find(strend, end, '\n')) != end) {
                if (*(strend - 1) == '\\') {
                    str += string(valstart, strend - 1);
                    ++strend;
                    valstart = strend;
                } else {
                    str += string(valstart, strend);
                    break;
                }
            }
            value = str;
            begin = strend + 1;
        }
    }
};

template <typename Iterator>
Iterator get_closing_token(const Iterator& begin, const Iterator& end, char open_token, char close_token) {
    int level = 1;
    Iterator cur = begin;
    char toks[] = {open_token, close_token};
    do {
        cur = std::find_first_of(cur, end, toks, toks + sizeof(toks));
        if (cur == end) return end;
        else if (*cur == open_token) ++level;
        else --level;
    } while (level >= 0);
    return cur;
}

struct MappingNode: Node {
    virtual void process(iterator &begin, const iterator &end) {
        value = XDict();
        auto root_indent = getindent(begin, end);
        auto lstart = begin;
        auto lend = std::find(lstart, end, '\n');
        for (; lend != end; std::swap(lend, lstart), lend = std::find(lstart, end, '\n')) {
            if (std::distance(lstart, lend) == 0) {
                ++lend;
                continue;
            }
            //std::cout << std::string(lstart, lend) << std::endl;
            //std::cout << getindent(lstart, lend) << std::endl;
            auto indent = getindent(lstart, lend);
            if (indent != root_indent) {
                begin = lstart;
                return;
            }
            auto sep = std::find(lstart, lend, ':');
            string key{lstart, sep};
            tools::trim(key);
            begin = sep + 1;
            auto node = Node::Parse(begin, end);
            value.get<XDict>()[key] = node.value;
            lend = begin;
            //std::cout << value.get<XDict>()[key] << std::endl;
        }
    }
};

struct JsonMappingNode: Node {
    virtual void process(iterator &begin, const iterator &end) {
        begin = std::find(begin, end, '{') + 1;
        auto this_end = get_closing_token(begin, end, '{', '}');
        value = XDict();
        while(begin != end) {
            if (*begin == '}') {
                ++begin;
                return;
            }
            auto sep = std::find(begin, this_end, ':');
            string key{begin, sep};
            tools::trim(key);
            begin = sep + 1;
            auto node = Node::Parse(begin, this_end);
            value.get<XDict>()[key] = node.value;
            //std::cout << value.get<XDict>()[key] << std::endl;
            static const char tokens[] = {',', '}'};
            auto next = find_first_of(begin, end, tokens, tokens + sizeof(tokens));
            begin = next + 1;
            if (*next == '}') {;
                return;
            }
        }
    }
};

struct SequenceNode: Node {
    virtual void process(iterator &begin, const iterator &end) {
        value = XList();
        auto root_indent = getindent(begin, end);
        auto lstart = begin;
        auto lend = std::find(lstart, end, '\n');
        for (; lend != end; std::swap(lend, lstart), lend = std::find(++lstart, end, '\n')) {
            if (std::distance(lstart, lend) == 0)
                continue;
            //std::cout << std::string(lstart, lend) << std::endl;
            //std::cout << getindent(lstart, lend) << std::endl;
            auto indent = getindent(lstart, lend);
            if (indent != root_indent) {
                begin = lstart;
                return;
            }
            auto sep = std::find(lstart, lend, '-');
            begin = sep + 1;
            auto node = Node::Parse(begin, end);
            value.get<XList>().push(node.value);
            //std::cout << value.get<XList>().back() << std::endl;
        }
    }
};


struct JsonSequenceNode: Node {
    virtual void process(iterator &begin, const iterator &end) {
        begin = std::find(begin, end, '[') + 1;
        auto this_end = get_closing_token(begin, end, '[', ']');
        value = XList();
        while(begin != end) {
            if (*begin == ']') {
                ++begin;
                return;
            }
            auto node = Node::Parse(begin, this_end);
            value.get<XList>().push(node.value);
            // std::cout << value.get<XList>().back() << std::endl;
            static const char tokens[] = {',', ']'};
            auto next = find_first_of(begin, end, tokens, tokens + sizeof(tokens));
            begin = next + 1;
            if (*next == ']') {;
                return;
            }
        }
    }
};

Node Node::Parse(iterator &begin, const iterator &end) {
    auto subbegin = find_if_not(begin, end, [] (auto&c) { return isspace(c); });
    static const char non_scalar_seps[] = {'{', '[', ':', '-', '\n'};
    iterator test = find_first_of(subbegin, end, non_scalar_seps, non_scalar_seps + sizeof (non_scalar_seps));

    //if (first_alnum < test) test = first_alnum;
    if (*test == '-' && (*(test + 1) == ' ' || *(test + 1) == '\n')) {
        SequenceNode node;
        node.process(begin, end);
        return node;
    } else if (*test == '[') {
        JsonSequenceNode node;
        node.process(begin, end);
        return node;
    } else if (*test == ':') {
        MappingNode node;
        node.process(begin, end);
        return node;
    } else if (*test == '{') {
        JsonMappingNode node;
        node.process(begin, end);
        return node;
    } else {
        ScalarNode node;
        node.process(begin, end);
        return node;
    }
}

Parser::Parser(const std::string& data):
    _data(data)
{
}

XVariant Parser::operator()()
{
    auto begin = _data.begin();
    auto root = Node::Parse(begin, _data.end());
    return root.value;
    // return process(_data.begin(), _data.end()).value();
}

}
}
