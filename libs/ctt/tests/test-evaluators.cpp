#include <catch2/catch.hpp>

#include <xdev/ctt/evaluator.hpp>

using namespace xdev;

SCENARIO("basic evaluator should yield internal data", "[xdev-ctt][evaluators]") {
  auto eval = ctt::evaluator_of<"hello", 0, 5>();
  REQUIRE(eval({{"hello", "world"}}) == "world");
}
