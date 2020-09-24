#include <xdev/variant.hpp>
#include <spdlog/spdlog.h>

#include <iostream>

namespace xdev {
namespace variant {

template <typename StringPolicy>
Dict<StringPolicy>::Dict(): _value() {}

template <typename StringPolicy>
Dict<StringPolicy>::Dict(const DictInitList<StringPolicy> &value): _value(value) {}

template <typename StringPolicy>
Dict<StringPolicy>::Dict(Dict&&other): _value(std::move(other._value)) {}

template <typename StringPolicy>
Dict<StringPolicy>& Dict<StringPolicy>::operator=(Dict&&other) { _value = std::move(other._value); return *this; }

template <typename StringPolicy>
Dict<StringPolicy>::Dict(const Dict&other): _value(other._value) {}

template <typename StringPolicy>
Dict<StringPolicy>& Dict<StringPolicy>::operator=(const Dict&other) { _value = other._value; return *this; }


template <typename StringPolicy>
decltype(auto) Dict<StringPolicy>::begin() { return _value.begin(); }

template <typename StringPolicy>
decltype(auto) Dict<StringPolicy>::end() { return _value.end(); }

template <typename StringPolicy>
decltype(auto) Dict<StringPolicy>::begin() const { return _value.cbegin(); }

template <typename StringPolicy>
decltype(auto) Dict<StringPolicy>::end() const { return _value.cend(); }

template <typename StringPolicy>
size_t Dict<StringPolicy>::size() const { return _value.size(); }

template <typename StringPolicy>
std::string Dict<StringPolicy>::toString() const {
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

template <typename StringPolicy>
Variant<StringPolicy>& Dict<StringPolicy>::operator[](const Value<StringPolicy>& key) {
    auto item = _value.find(key);
    if (item == end()) {
        if (key.template is<std::string>()) {
            auto skey = tools::split(key.template get<std::string>(), '.');
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
                d = &tmp.template get<Dict>();
            }
            k = std::move(*it);
            if(!d->contains(k)) {
                return d->_value.insert_or_assign(k, Variant<StringPolicy>{}).first->second;
            } else {
                return d->_value.at(k);
            }
        } else {
            return _value.insert_or_assign(key, Variant<StringPolicy>{}).first->second;
        }
    }
    else return item->second;
}

template <typename StringPolicy>
bool Dict<StringPolicy>::operator==(const Dict &other) const {
  return _value == other._value;
}

template <typename StringPolicy>
auto Dict<StringPolicy>::operator<=>(const Dict &other) const {
    return _value <=> other._value;
}

template <typename StringPolicy>
inline Dict<StringPolicy>& Dict<StringPolicy>::update(Dict&& other) {
    for (auto&& [k, v]: other._value) {
        _value.emplace(std::move(k), std::move(v));
    }
    return *this;
}

template <typename StringPolicy>
template <typename...RestT>
Variant<StringPolicy>& Dict<StringPolicy>::at(Value<StringPolicy>&& key, RestT&&...rest) {
    try {
        auto& v = _value.at(xfwd(key));
        if constexpr (sizeof...(rest) > 0) {
            return v.template get<Dict<StringPolicy>>().at(xfwd(rest)...);
        } else {
            return v;
        }
    } catch (const std::out_of_range& e) {
        if (!key.template is<std::string>())
            throw e;
        // handle dot notation
        return dotAt(key.template get<std::string>());
    }
}

template <typename StringPolicy>
template <typename...RestT>
Variant<StringPolicy>& Dict<StringPolicy>::at(const Value<StringPolicy> &key, const RestT&...rest) {
    try {
        auto& v = _value.at(key);
        if constexpr (sizeof...(rest) > 0) {
            return v.template get<Dict>().at(rest...);
        } else {
            return v;
        }
    } catch (const std::out_of_range& e) {
        if (!key.template is<std::string>())
            throw std::move(e);
        // handle dot notation
        return dotAt(key.template get<std::string>());
    }
}

template <typename StringPolicy>
template <typename...RestT>
const Variant<StringPolicy>& Dict<StringPolicy>::at(const Value<StringPolicy> &key, const RestT&...rest) const {
    try {
        auto& v = _value.at(key);
        if constexpr (sizeof...(rest) > 0) {
            return v.template get<Dict>().at(rest...);
        } else {
            return v;
        }
    } catch (const std::out_of_range& e) {
        if (!key.template is<std::string>())
            throw e;
        // handle dot notation
        return dotAt(key.template get<std::string>());
    }
}

template <typename StringPolicy>
Variant<StringPolicy>& Dict<StringPolicy>::dotAt(const std::string& key) {
    auto skey = tools::split(key, '.');
    Dict* d = this;
    auto it = skey.begin();
    auto end = skey.end();
    --end;
    for(; it != end; ++it) {
        try {
            d = &d->_value.at(Value{*it}).template get<Dict>();
        } catch (std::bad_variant_access&) {
            throw std::out_of_range("Cannot resolve dot notation");
        }
    }
    return d->_value.at(*it);
}

template <typename StringPolicy>
const Variant<StringPolicy>& Dict<StringPolicy>::dotAt(const std::string& key) const {
    auto skey = tools::split(key, '.');
    const Dict* d = this;
    auto it = skey.begin();
    auto end = skey.end();
    --end;
    for(; it != end; ++it) {
        try {
            d = &d->_value.at(*it).template get<Dict>();
        } catch (std::bad_variant_access&) {
            throw std::out_of_range("Cannot resolve dot notation");
        }
    }
    return d->_value.at(*it);
}

}
}
