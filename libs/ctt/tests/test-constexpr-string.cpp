#include <vector>
#include <vector>
#include <numeric>
#include <string>
#include <functional>
#include <algorithm>
#include <ranges>
#include <variant>
#include <cstring>

#include <fmt/format.h>

constexpr auto test() {
//  if (std::is_constant_evaluated()) {
//
//    auto *src = new int[4];
//    auto *dst = new int[4];
////    std::memmove(src, dst, 4 * sizeof(int));
//    __builtin_memmove(src, dst, 4 * sizeof(int));
//    delete[] src;
//    delete[] dst;
//  }
//
  std::string str = std::string("hello ") + "world";
  return str;

//  std::vector<int> test{1, 2, 3};
//  test.push_back(4);
//  test.emplace_back(2);
//  test.pop_back();
//  int result = std::accumulate(begin(test), end(test), 0);
//  return result;
}

static_assert(test() == "hello world");

#include <cassert>

int main() {
//  std::string test = "hello";
//  auto test2 = std::variant<int, bool, std::string>{true};
  assert(test() == 10);
  return 0;
}