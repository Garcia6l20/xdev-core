#pragma once

#include <utility>

namespace xdev {

template <typename ActionT>
struct finally {
  explicit finally(ActionT &&action) noexcept : action_{std::move(action)} {
  }

private:
  ActionT action_;
};

}
