/** @file xdev/ctt/concepts.hpp
 *
 */
#pragma once

#include <string_view>

namespace xdev::ctt {
  template <typename T>
  concept dictionary = requires(T val) {
    { val.at(std::string_view{}) };
  };
}
