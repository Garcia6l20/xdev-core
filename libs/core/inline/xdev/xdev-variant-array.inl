#include <xdev/xdev-variant.hpp>
#include <xdev/xdev-tools.hpp>

#include <numeric>

namespace std {

template <>
struct hash<xdev::variant::Array>
{
    std::size_t operator()(const xdev::variant::Array& var) const
    {
        return var.hash();
    }
};

}

namespace xdev {
namespace variant {

Array::Array(): _value() {}

Array::Array(Array&&other): _value(std::move(other._value)) {}
Array& Array::operator=(Array&&other) { _value = std::move(other._value); return *this; }
Array::Array(const Array&other): _value(other._value) {}
Array& Array::operator=(const Array&other) { _value = other._value; return *this; }

Array::Array(const std::initializer_list<Variant>&value): _value(value) {}

Array::iterator Array::begin() { return _value.begin(); }
Array::iterator Array::end() { return _value.end(); }
Variant& Array::front() { return _value.front(); }
Variant& Array::back() { return _value.back(); }
Array::const_iterator Array::begin() const { return _value.cbegin(); }
Array::const_iterator Array::end() const { return _value.cend(); }
size_t Array::size() const { return _value.size(); }
Variant& Array::operator[](size_t index) {
    if (_value.size() <= index) {
        throw std::out_of_range("querying index greater than size");
    }
    auto it = begin();
    std::advance(it, index);
    return *it;
}

const Variant& Array::operator[](size_t index) const {
    if (_value.size() <= index) {
        throw std::out_of_range("querying index greater than size");
    }
    auto it = begin();
    std::advance(it, index);
    return *it;
}

std::string Array::toString() const {
    std::string res = "[";
    res += tools::join(*this, ", ", [](auto&&item) {
        if (item.template is<std::string>())
            return std::string("\"") + item.toString() + "\"";
        else return item.toString();
    });
    res += "]";
    return res;
}

template <Array::Direction direction>
void Array::pop(size_t count) {
    while(count--) {
        if constexpr (direction == Front)
            _value.pop_front();
        else _value.pop_back();
    }
}

template <Array::Direction direction, typename First, typename...Rest>
void Array::push(First&&first, Rest&&...rest) {
    if constexpr (direction == Front) {
        if constexpr (sizeof...(rest))
            push<direction>(std::forward<Rest>(rest)...);
        _value.push_front(std::forward<First>(first));
    } else {
        _value.push_back(std::forward<First>(first));
        if constexpr (sizeof...(rest))
            push<direction>(std::forward<Rest>(rest)...);
    }
}

Array::iterator Array::find(Variant &&value) {
    return std::find(begin(), end(), std::forward<Variant>(value));
}

size_t Array::hash() const {
    return std::accumulate(begin(), end(), _value.size(), [](size_t seed, Variant ii){
        return seed ^ (ii.hash() + 0x9e3779b9 + (seed << 6) + (seed >> 2));
    });
}

Array& operator>>(Variant&&var, Array& array) {
    array._value.push_front(std::forward<Variant>(var));
    return array;
}

Array& operator-(size_t count, Array& array) {
    while(count--)
        array._value.pop_front();
    return array;
}

Array& operator<<(Array& array, Variant&&var) {
    array._value.push_back(std::forward<Variant>(var));
    return array;
}

Array& operator-(Array& array, size_t count) {
    while(count--)
        array._value.pop_back();
    return array;
}

}
}
