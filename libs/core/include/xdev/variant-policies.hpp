#pragma once

#include <string>
#include <string_view>

namespace xdev {

struct StdStringPolicy {
  using string_type = std::string;
};

struct StdStringViewPolicy {
  using string_type = std::string_view;
};

}
