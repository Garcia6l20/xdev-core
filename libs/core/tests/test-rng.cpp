#include <catch2/catch.hpp>

#include <xdev/rng.hpp>

#include <spdlog/spdlog.h>

using namespace xdev;

using namespace std::literals;

constexpr auto to_sv = views::transform([](auto &&r) {
  auto sz = size_t(rng::size(r));
  auto data = &*r.begin();
  auto tmp = std::string_view(data, sz);
  spdlog::info("evaluating: {}", tmp);
  return tmp;
});

SCENARIO("rng::size should work", "[xdev-core][rng]") {
  auto test = "root.test"sv | views::split('.') | to_sv;
  REQUIRE(rng::size(test) == 2);
  REQUIRE(rng::at(test, 0) == "root");
  REQUIRE(rng::at(test, 1) == "test");
}

SCENARIO("rng::to should work", "[xdev-core][rng]") {
  auto test = rng::to<std::vector>("root.test"sv | views::split('.') | to_sv);
  REQUIRE(test.size() == 2);
  REQUIRE(test.at(0) == "root");
  REQUIRE(test.at(1) == "test");
}
