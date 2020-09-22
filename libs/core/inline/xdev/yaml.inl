#include <xdev/yaml.hpp>
#include <xdev/tools.hpp>

#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <unordered_map>
#include <regex>
#include <stack>

// #define YAML_DEBUG

#ifdef YAML_DEBUG
#include <spdlog/spdlog.h>
#define _YAML_DEBUG(...)  spdlog::info(__VA_ARGS__)
#else
#define _YAML_DEBUG(...)
#endif

namespace xdev::yaml {

  template<typename Iterator, typename Callable>
  inline void for_each_lines(const Iterator &begin, const Iterator &end, Callable callable) {
    auto lstart = begin;
    auto lend = std::find(lstart, end, '\n');
    while (lend != end) {
      if (!callable(lstart, ++lend))
        return;
      std::swap(lend, lstart);
      lend = std::find(lstart, end, '\n');
    }
  }

  template<typename Iterator>
  inline ssize_t get_indent(Iterator &begin, const Iterator &end) {
    if (*begin == '\n')
      ++begin;
    return std::distance(begin, std::find_if_not(begin, end, static_cast<int (*)(int)>(std::isspace)));
  }

  struct Node {
    virtual ~Node() noexcept = default;
    using iterator = std::string::const_iterator;
    virtual void process(iterator &begin, const iterator &end) {
      _indent = get_indent(begin, end);
    }
    static inline Node Parse(iterator &begin, const iterator &end);
    ssize_t _indent;
    xvar value;
  };

  struct ScalarNode : Node {
    virtual void process(iterator &begin, const iterator &end) {
      Node::process(begin, end);
      auto valstart = begin + _indent;
      static const char end_of_value_tokens[] = {'\n', ' ', ','};
      auto valend =
        std::find_first_of(valstart, end, end_of_value_tokens, end_of_value_tokens + sizeof(end_of_value_tokens));
      smatch match;
      // null
      if (regex_match(valstart, valend, match, regex(R"(null|Null|NULL|~)"))) {
        value = xnone {};
        begin = valstart + match.length();
        return;
      }
      // boolean
      if (regex_match(valstart, valend, match, regex(R"(true|True|TRUE|false|False|FALSE)"))) {
        value = *valstart == 't' || *valstart == 'T';
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

  template<typename Iterator>
  Iterator get_closing_token(const Iterator &begin, const Iterator &end, char open_token, char close_token) {
    int level = 1;
    Iterator cur = begin;
    char toks[] = {open_token, close_token};
    do {
      cur = std::find_first_of(cur, end, toks, toks + sizeof(toks));
      if (cur == end)
        return end;
      else if (*cur == open_token)
        ++level;
      else
        --level;
    } while (level >= 0);
    return cur;
  }

  struct MappingNode : Node {
    void process(iterator &begin, const iterator &end) final {
      Node::process(begin, end);
      if (*(begin - 1) == '-') {
        ++_indent;
      }
      value = xdict();
      auto lstart = begin;
      auto lend = std::find(lstart, end, '\n');
      for (; lend != end; std::swap(lend, lstart), lend = std::find(lstart, end, '\n')) {
        if (std::distance(lstart, lend) == 0) {
          ++lend;
          continue;
        }
        auto indent = get_indent(lstart, lend);
        if (*(lstart - 1) == '-') {
          ++indent;
        }
        if (indent != _indent) {
          begin = lstart;
          return;
        }
        auto sep = std::find(lstart, lend, ':');
        string key {lstart, sep};
        tools::trim(key);
        begin = sep + 1;
        auto node = Node::Parse(begin, end);
        value.get<xdict>()[key] = node.value;
        lend = begin;
        _YAML_DEBUG("MappingNode: {} {}", key, node.value);
      }
      begin = lend;
    }
  };

  struct JsonMappingNode : Node {
    void process(iterator &begin, const iterator &end) final {
      Node::process(begin, end);
      begin = std::find(begin, end, '{') + 1;
      auto this_end = get_closing_token(begin, end, '{', '}');
      value = xdict();
      while (begin != end) {
        if (*begin == '}') {
          ++begin;
          return;
        }
        auto sep = std::find(begin, this_end, ':');
        string key {begin, sep};
        tools::trim(key);
        begin = sep + 1;
        auto node = Node::Parse(begin, this_end);
        value.get<xdict>()[key] = node.value;
        static const char tokens[] = {',', '}'};
        auto next = find_first_of(begin, end, tokens, tokens + sizeof(tokens));
        begin = next + 1;
        if (*next == '}') {
          return;
        }
      }
    }
  };

  struct SequenceNode : Node {
    void process(iterator &begin, const iterator &end) final {
      Node::process(begin, end);
      value = xlist();
      auto lstart = begin;
      auto lend = std::find(lstart, end, '\n');
      for (; lend != end; std::swap(lend, lstart), lend = std::find(lstart, end, '\n')) {
        _YAML_DEBUG("Processing: {}", std::string_view{lstart, lend});
        if (std::distance(lstart, lend) == 0) {
          lend++;
          continue;
        }
        auto indent = get_indent(lstart, lend);
        if (indent != _indent) {
          begin = lstart;
          return;
        }
        auto sep = std::find(lstart, lend, '-');
        begin = sep + 1;
        auto node = Node::Parse(begin, end);
        value.get<xlist>().push(node.value);
        _YAML_DEBUG("SequenceNode: {}", node.value);
        lstart = begin;
        lend = std::find(lstart, end, '\n');
        _YAML_DEBUG("Remaining: {}", std::string_view{lstart, lend});
        std::swap(lend, lstart);
      }
    }
  };


  struct JsonSequenceNode : Node {
    void process(iterator &begin, const iterator &end) final {
      begin = std::find(begin, end, '[') + 1;
      auto this_end = get_closing_token(begin, end, '[', ']');
      value = xlist();
      while (begin != end) {
        if (*begin == ']') {
          ++begin;
          return;
        }
        auto node = Node::Parse(begin, this_end);
        value.get<xlist>().push(node.value);
        static const char tokens[] = {',', ']'};
        auto next = find_first_of(begin, end, tokens, tokens + sizeof(tokens));
        begin = next + 1;
        if (*next == ']') {
          return;
        }
      }
    }
  };

  Node Node::Parse(iterator &begin, const iterator &end) {
    auto subbegin = find_if_not(begin, end, [](auto &c) { return isspace(c); });
    static const char non_scalar_seps[] = {'{', '[', ':', '-', '\n'};
    iterator test = find_first_of(subbegin, end, non_scalar_seps, non_scalar_seps + sizeof(non_scalar_seps));

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

  Parser::Parser(const std::string &data) : _data(data) {}

  xvar Parser::operator()() {
    auto begin = _data.begin();
    auto root = Node::Parse(begin, _data.end());
    return root.value;
  }

} // namespace xdev::yaml
