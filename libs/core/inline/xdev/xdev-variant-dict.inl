#include <xdev/xdev-variant.hpp>
#include <spdlog/spdlog.h>

#include <iostream>

namespace xdev {
namespace variant {

Dict::Dict(): _value() {}
Dict::Dict(const DictInitList &value): _value(value) {}
Dict::Dict(Dict&&other): _value(std::move(other._value)) {}
Dict& Dict::operator=(Dict&&other) { _value = std::move(other._value); return *this; }
Dict::Dict(const Dict&other): _value(other._value) {}
Dict& Dict::operator=(const Dict&other) { _value = other._value; return *this; }


Dict::iterator Dict::begin() { return _value.begin(); }
Dict::iterator Dict::end() { return _value.end(); }
Dict::const_iterator Dict::begin() const { return _value.cbegin(); }
Dict::const_iterator Dict::end() const { return _value.cend(); }
size_t Dict::size() const { return _value.size(); }

std::string Dict::toString() const {
    std::string res = "{";
    res += tools::join(*this, ", ", [](auto&&item) {
        std::string tmp = std::string("\"") + item.first.toString() + "\": ";
        if (item.second.template is<std::string>())
            tmp += std::string("\"") + item.second.toString() + "\"";
        else tmp += item.second.toString();
        return tmp;
    });
    res += "}";
    return res;
}

Variant& Dict::operator[](const Value& key) {
    auto item = _value.find(key);
    if (item == end()) {
        if (key.is<std::string>()) {
            auto skey = tools::split(key.get<std::string>(), '.');
            Dict* d = this;
            auto it = skey.begin();
            auto end = skey.end();
            --end;
            Value k;
            for(; it != end; ++it) {
                k = std::move(*it);
                auto& tmp = d->operator[](k);
                if (tmp.empty()) {
                    tmp = Dict{};
                }
                d = &tmp.get<Dict>();
            }
            k = std::move(*it);
            if(!d->contains(k)) {
                return d->_value.insert_or_assign(k, Variant{}).first->second;
            } else {
                return d->_value.at(k);
            }
        } else {
            return _value.insert_or_assign(key, Variant{}).first->second;
        }
    }
    else return item->second;
}

bool Dict::operator==(const Dict &other) const {
  return _value == other._value;
}

auto Dict::operator<=>(const Dict &other) const {
    return _value <=> other._value;
}

inline Dict& Dict::update(Dict&& other) {
    for (auto&& [k, v]: other._value) {
        _value.emplace(std::move(k), std::move(v));
    }
    return *this;
}

template <typename...RestT>
Variant& Dict::at(Value&& key, RestT&&...rest) {
    try {
        auto& v = _value.at(std::forward<Value>(key));
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
Variant& Dict::at(const Value &key, const RestT&...rest) {
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
const Variant& Dict::at(const Value &key, const RestT&...rest) const {
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
            d = &d->_value.at(Value{*it}).get<Dict>();
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
