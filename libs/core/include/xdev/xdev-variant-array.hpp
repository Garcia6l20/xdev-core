/**
 * @file xdev-variant-array.hpp
 */
#pragma once

#include <list>
#include <string>

#include <xdev/xdev-typetraits.hpp>

namespace xdev::variant {

class Variant;

class Array {
public:
    using array_t = std::list<Variant>;

    inline Array();

    inline Array(Array&&other);
    inline Array& operator=(Array&&other);
    inline Array(const Array&other);
    inline Array& operator=(const Array&other);

    inline Array(const std::initializer_list<Variant>&value);

    inline size_t hash() const;

    using iterator = array_t::iterator;
    using const_iterator = array_t::const_iterator;
    inline iterator begin();
    inline iterator end();
    inline Variant& front();
    inline Variant& back();
    inline const_iterator begin() const;
    inline const_iterator end() const;
    inline size_t size() const;
    inline Variant& operator[](size_t index);
    inline const Variant& operator[](size_t index) const;
    inline std::string toString() const;
    template <typename...ArgsT>
    iterator emplace(const_iterator pos, ArgsT...args) {
        return _value.emplace(pos, xfwd(args)...);
    }
    template <typename...ArgsT>
    iterator emplace_front(ArgsT...args) {
        return _value.emplace_front(xfwd(args)...);
    }
    template <typename...ArgsT>
    iterator emplace_back(ArgsT...args) {
        return _value.emplace_back(xfwd(args)...);
    }

    enum Direction { Front, Back };
    template <Direction direction = Front>
    inline void pop(size_t count = 1);

    template <Direction direction = Back, typename First, typename...Rest>
    inline void push(First&& first, Rest&&...var);

    inline Array::iterator find(Variant&&value);

    template <typename IteratorT>
    auto erase(IteratorT&& iter) { return _value.erase(xfwd(iter)); }

    friend inline Array& operator>>(Variant&&var, Array& array);
    friend inline Array& operator-(size_t count, Array& array);

    friend inline Array& operator<<(Array& array, Variant&&var);
    friend inline Array& operator-(Array& array, size_t count);

    auto operator<=>(const Array&) const = default;

    static constexpr const char* ctti_nameof()
    {
        return "XArray";
    }

private:
    array_t _value;
};

}
