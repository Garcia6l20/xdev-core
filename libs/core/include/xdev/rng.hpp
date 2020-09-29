/** @brief std::ranges extra utilities
 *
 */
#pragma once

namespace xdev {
  namespace views = std::views;
}

namespace xdev::rng {
  using namespace std::ranges;

  constexpr auto size(range auto &&r) noexcept {
    return rng::distance(rng::begin(r), rng::end(r));
  }

  constexpr auto at(range auto &&r, size_t index) noexcept {
    auto b = rng::begin(std::forward<decltype(r)>(r));
    rng::advance(b, std::iter_difference_t<decltype(b)>(index));
    return *b;
  }

  template <template <class ...> typename ContainerT>
  constexpr auto to(range auto &&r) {
    using range_t = std::decay_t<decltype(r)>;
    using elem_t = std::decay_t<rng::range_value_t<range_t>>;
    return ContainerT<elem_t>{r.begin(), r.end()};
  }
}
