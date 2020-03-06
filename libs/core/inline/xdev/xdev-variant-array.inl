//#include <xdev/xdev-variant.hpp>
#include <xdev/xdev-tools.hpp>

#include <numeric>

namespace xdev::variant {

List::List(): _value() {}

List::List(List&&other): _value(std::move(other._value)) {}
List& List::operator=(List&&other) { _value = std::move(other._value); return *this; }
List::List(const List&other): _value(other._value) {}
List& List::operator=(const List&other) { _value = other._value; return *this; }

List::List(std::initializer_list<Variant> value): _value(value) {}

template <typename...Ts>
    requires (not one_of<std::decay_t<Ts>, Value, Variant, List> && ...)
inline List::List(Ts&&...args): List(Variant{std::forward<Ts>(args)}...) {

}

List::iterator List::begin() { return _value.begin(); }
List::iterator List::end() { return _value.end(); }
Variant& List::front() { return _value.front(); }
Variant& List::back() { return _value.back(); }
List::const_iterator List::begin() const { return _value.cbegin(); }
List::const_iterator List::end() const { return _value.cend(); }
size_t List::size() const { return _value.size(); }
Variant& List::operator[](size_t index) {
    if (_value.size() <= index) {
        throw std::out_of_range("querying index greater than size");
    }
    auto it = begin();
    std::advance(it, index);
    return *it;
}

const Variant& List::operator[](size_t index) const {
    if (_value.size() <= index) {
        throw std::out_of_range("querying index greater than size");
    }
    auto it = begin();
    std::advance(it, index);
    return *it;
}

std::string List::toString() const {
    std::string res = "[";
    res += tools::join(*this, ", ", [](auto&&item) {
        if (item.template is<std::string>())
            return std::string("\"") + item.toString() + "\"";
        else return item.toString();
    });
    res += "]";
    return res;
}

template <List::Direction direction>
void List::pop(size_t count) {
    while(count--) {
        if constexpr (direction == Front)
            _value.pop_front();
        else _value.pop_back();
    }
}

template <List::Direction direction, typename First, typename...Rest>
void List::push(First&&first, Rest&&...rest) {
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

List::iterator List::find(Variant &&value) {
    return std::find(begin(), end(), std::forward<Variant>(value));
}

List& operator>>(Variant&&var, List& lst) {
    lst._value.push_front(std::forward<Variant>(var));
    return lst;
}

List& operator-(size_t count, List& lst) {
    while(count--)
        lst._value.pop_front();
    return lst;
}

List& operator<<(List& lst, Variant&&var) {
    lst._value.push_back(std::forward<Variant>(var));
    return lst;
}

List& operator-(List& lst, size_t count) {
    while(count--)
        lst._value.pop_back();
    return lst;
}

inline bool operator==(const List& lhs, const List& rhs) {
    return lhs._value == rhs._value;
}

}
