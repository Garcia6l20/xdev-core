#include <catch2/catch.hpp>

#include <xdev/ct/string.hpp>

using namespace xdev;
using namespace xdev::ct::literals;

template <ct::string input>
struct test1_t {
  static constexpr auto view = input.view();
};

using hello = test1_t<"hello">;

SCENARIO("ct::string can be used as template parameter", "[xdev-ct][string]") {
  STATIC_REQUIRE(hello::view == "hello");
}
