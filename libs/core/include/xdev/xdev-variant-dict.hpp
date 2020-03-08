/**
 * @file xdev-variant-dict.hpp
 */
#pragma once

#include <map>

namespace xdev::variant {

class Variant;
class Value;

using DictNode = std::pair<const Value, Variant>;
using DictInitList = std::initializer_list<DictNode>;

class Dict {
public:
    using map_t = std::map<Value, Variant>;

    inline Dict();
    inline Dict(const Dict&other);
    inline Dict& operator=(const Dict&other);
    inline Dict(Dict&&other);
    inline Dict& operator=(Dict&&other);
    inline Dict(const DictInitList& value);

    using iterator = map_t::iterator;
    using const_iterator = map_t::const_iterator;
    inline iterator begin();
    inline iterator end();
    inline const_iterator begin() const;
    inline const_iterator end() const;
    inline size_t size() const;
    inline Variant& operator[](const Value& index);

    template <typename...RestT>
    inline Variant& at(Value&& key, RestT&&...rest);

    template <typename...RestT>
    inline Variant& at(const Value& key, const RestT&...rest);

    template <typename...RestT>
    inline const Variant& at(const Value& key, const RestT&...rest) const;
    inline Variant& dotAt(const std::string& key);
    inline const Variant& dotAt(const std::string& key) const;
    inline std::string toString() const;

    inline Dict& update(Dict&& other);

    auto operator<=>(const Dict&) const = default;

    template <typename T>
    inline bool contains(const T& key) const {
        return _value.contains(key);
    }

    static constexpr const char* ctti_nameof() {
        return "xdict";
    }

private:
    map_t _value;
};

}
