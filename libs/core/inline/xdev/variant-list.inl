//#include <xdev/xdev-variant.hpp>
#include <xdev/tools.hpp>

#include <numeric>

namespace xdev::variant {

template <typename StringPolicy>
List<StringPolicy>::List() : _value() {}

template <typename StringPolicy>
List<StringPolicy>::List(List &&other) : _value(std::move(other._value)) {}

template <typename StringPolicy>
List<StringPolicy> &List<StringPolicy>::operator=(List &&other) {
  _value = std::move(other._value);
  return *this;
}

template <typename StringPolicy>
List<StringPolicy>::List(const List &other) : _value(other._value) {}

template <typename StringPolicy>
List<StringPolicy> &List<StringPolicy>::operator=(const List &other) {
  _value = other._value;
  return *this;
}

template <typename StringPolicy>
List<StringPolicy>::List(const ListInitList<StringPolicy> &value) : _value {value} {}

template <typename StringPolicy>
List<StringPolicy>::List(XListConvertible<StringPolicy> auto &&container) {
  for (auto &&item : container) { _value.emplace_back(std::move(item)); }
}

template <typename StringPolicy>
List<StringPolicy>::List(const XListConvertible<StringPolicy> auto &container) {
  for (const auto &item : container) { _value.emplace_back(item); }
}


template <typename StringPolicy>
typename List<StringPolicy>::iterator List<StringPolicy>::begin() { return _value.begin(); }

template <typename StringPolicy>
typename List<StringPolicy>::iterator List<StringPolicy>::end() { return _value.end(); }

template <typename StringPolicy>
auto &List<StringPolicy>::front() { return _value.front(); }

template <typename StringPolicy>
auto &List<StringPolicy>::back() { return _value.back(); }

template <typename StringPolicy>
typename List<StringPolicy>::const_iterator List<StringPolicy>::begin() const { return _value.cbegin(); }

template <typename StringPolicy>
typename List<StringPolicy>::const_iterator List<StringPolicy>::end() const { return _value.cend(); }

template <typename StringPolicy>
size_t List<StringPolicy>::size() const { return _value.size(); }

template <typename StringPolicy>
auto &List<StringPolicy>::operator[](size_t index) {
  if (_value.size() <= index) { throw std::out_of_range("querying index greater than size"); }
  auto it = begin();
  std::advance(it, index);
  return *it;
}

template <typename StringPolicy>
const auto &List<StringPolicy>::operator[](size_t index) const {
  if (_value.size() <= index) { throw std::out_of_range("querying index greater than size"); }
  auto it = begin();
  std::advance(it, index);
  return *it;
}


template <typename StringPolicy>
std::string List<StringPolicy>::toString() const {
  std::string res = "[";
  res += tools::join(*this, ", ", [](auto &&item) {
    if (item.template is<std::string>())
      return std::string("\"") + item.toString() + "\"";
    else
      return item.toString();
  });
  res += "]";
  return res;
}


template <typename StringPolicy>
template<typename List<StringPolicy>::Direction direction>
void List<StringPolicy>::pop(size_t count) {
  while (count--) {
    if constexpr (direction == Front)
      _value.pop_front();
    else
      _value.pop_back();
  }
}


template <typename StringPolicy>
template<typename List<StringPolicy>::Direction direction, typename First, typename... Rest>
void List<StringPolicy>::push(First &&first, Rest &&... rest) {
  if constexpr (direction == Front) {
    if constexpr (sizeof...(rest)) push<direction>(std::forward<Rest>(rest)...);
    _value.push_front(std::forward<First>(first));
  } else {
    _value.push_back(std::forward<First>(first));
    if constexpr (sizeof...(rest)) push<direction>(std::forward<Rest>(rest)...);
  }
}

template <typename StringPolicy>
typename List<StringPolicy>::iterator List<StringPolicy>::find(Variant<StringPolicy> &&value) { return std::find(begin(), end(), std::forward<Variant>(value)); }

template <typename StringPolicy>
List<StringPolicy> &operator>>(Variant<StringPolicy> &&var, List<StringPolicy> &lst) {
  lst._value.push_front(std::forward<Variant>(var));
  return lst;
}

template <typename StringPolicy>
List<StringPolicy> &operator-(size_t count, List<StringPolicy> &lst) {
  while (count--) lst._value.pop_front();
  return lst;
}

template <typename StringPolicy>
List<StringPolicy> &operator<<(List<StringPolicy> &lst, Variant<StringPolicy> &&var) {
  lst._value.push_back(std::forward<Variant>(var));
  return lst;
}

template <typename StringPolicy>
List<StringPolicy> &operator-(List<StringPolicy> &lst, size_t count) {
  while (count--) lst._value.pop_back();
  return lst;
}

template <typename StringPolicy>
constexpr bool operator==(const List<StringPolicy> &lhs, const List<StringPolicy> &rhs) { return lhs._value == rhs._value; }

} // namespace xdev::variant
