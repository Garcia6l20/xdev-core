/**
 * @file xdev-variant-dict.hpp
 */
#pragma once

#include <map>

namespace xdev::variant {

class Variant;

class Dict {
public:
    using map_t = std::map<Variant, Variant>;
    using node_t = std::pair<const Variant, Variant>;
    using init_list_t = std::initializer_list<node_t>;

    inline Dict();
    inline Dict(const Dict&other);
    inline Dict& operator=(const Dict&other);
    inline Dict(Dict&&other);
    inline Dict& operator=(Dict&&other);
    inline Dict(const init_list_t& value);

    inline size_t hash() const;

    using iterator = map_t::iterator;
    using const_iterator = map_t::const_iterator;
    inline iterator begin();
    inline iterator end();
    inline const_iterator begin() const;
    inline const_iterator end() const;
    inline size_t size() const;
    inline Variant& operator[](Variant&& index);

    template <typename...RestT>
    inline Variant& at(Variant&& key, RestT&&...rest);

    template <typename...RestT>
    inline Variant& at(const Variant& key, const RestT&...rest);

    template <typename...RestT>
    inline const Variant& at(const Variant& key, const RestT&...rest) const;
    inline Variant& dotAt(const std::string& key);
    inline const Variant& dotAt(const std::string& key) const;
    inline std::string toString() const;

    template <typename T>
    inline bool has(const T& key) const {
        try {
            at(key);
            return true;
        } catch (const std::out_of_range&) {
            return false;
        }
    }

    static constexpr const char* ctti_nameof()
    {
        return "XDict";
    }

private:
    map_t _value;
};

}
