#include <xdev/xdev-variant.hpp>

#include <iostream>

namespace std {

template <>
struct hash<xdev::variant::Dict>
{
    std::size_t operator()(const xdev::variant::Dict& var) const
    {
        return var.hash();
    }
};

}

namespace xdev {
namespace variant {

Dict::Dict(): _value() {}
Dict::Dict(const Dict::init_list_t& value): _value(value) {}
Dict::Dict(Dict&&other): _value(std::move(other._value)) {}
Dict& Dict::operator=(Dict&&other) { _value = std::move(other._value); return *this; }
Dict::Dict(const Dict&other): _value(other._value) {}
Dict& Dict::operator=(const Dict&other) { _value = other._value; return *this; }


Dict::iterator Dict::begin() { return _value.begin(); }
Dict::iterator Dict::end() { return _value.end(); }
Dict::const_iterator Dict::begin() const { return _value.cbegin(); }
Dict::const_iterator Dict::end() const { return _value.cend(); }
size_t Dict::size() const { return _value.size(); }

size_t Dict::hash() const {
    return std::accumulate(begin(), end(), _value.size(), [](size_t seed, std::pair<Variant, Variant> ii){
        return seed ^ (ii.first.hash() ^ ii.second.hash()) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    });
}

std::string Dict::toString() const {
    std::string res = "{";
    res += tools::join(*this, ", ", [](auto&&item) {
        using T = std::decay_t<decltype(item.second)>;
        std::string tmp = std::string("\"") + item.first.toString() + "\": ";
        if (item.second.template is<std::string>())
            tmp += std::string("\"") + item.second.toString() + "\"";
        else tmp += item.second.toString();
        return std::move(tmp);
    });
    res += "}";
    return res;
}

Variant& Dict::operator[](Variant&& key) {
    auto item = _value.find(key);
    if (item == end())
        return _value.insert_or_assign(std::forward<Variant>(key), Variant()).first->second;
    else return item->second;
}

template <typename...RestT>
Variant& Dict::at(Variant&& key, RestT&&...rest) {
    try {
        auto& v = _value.at(std::forward<Variant>(key));
        if constexpr (sizeof...(rest) > 0) {
            return v.get<Dict>().at(std::forward<Variant>(rest)...);
        } else {
            return v;
        }
    } catch (const std::out_of_range& e) {
        if (!key.is<std::string>())
            throw std::move(e);
        // handle dot notation
        return dotAt(key.get<std::string>());
    }
}

template <typename...RestT>
Variant& Dict::at(const Variant& key, const RestT&...rest) {
    try {
        auto& v = _value.at(key);
        if constexpr (sizeof...(rest) > 0) {
            return v.get<Dict>().at(rest...);
        } else {
            return v;
        }
    } catch (const std::out_of_range& e) {
        if (!key.is<std::string>())
            throw std::move(e);
        // handle dot notation
        return dotAt(key.get<std::string>());
    }
}

template <typename...RestT>
const Variant& Dict::at(const Variant &key, const RestT&...rest) const {
    try {
        auto& v = _value.at(key);
        if constexpr (sizeof...(rest) > 0) {
            return v.get<Dict>().at(rest...);
        } else {
            return v;
        }
    } catch (const std::out_of_range& e) {
        if (!key.is<std::string>())
            throw std::move(e);
        // handle dot notation
        return dotAt(key.get<std::string>());
    }
}

Variant& Dict::dotAt(const std::string& key) {
    auto skey = tools::split(key, '.');
    Dict* d = this;
    auto it = skey.begin();
    auto end = skey.end();
    --end;
    for(; it != end; ++it) {
        try {
            d = &d->_value.at(*it).get<Dict>();
        } catch (std::bad_variant_access&) {
            throw std::out_of_range("Cannot resolve dot notation");
        }
    }
    return d->_value.at(*it);
}

const Variant& Dict::dotAt(const std::string& key) const {
    auto skey = tools::split(key, '.');
    const Dict* d = this;
    auto it = skey.begin();
    auto end = skey.end();
    --end;
    for(; it != end; ++it) {
        try {
            d = &d->_value.at(*it).get<Dict>();
        } catch (std::bad_variant_access&) {
            throw std::out_of_range("Cannot resolve dot notation");
        }
    }
    return d->_value.at(*it);
}

}
}
