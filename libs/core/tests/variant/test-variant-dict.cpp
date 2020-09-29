#include <catch2/catch.hpp>

#include <xdev.hpp>

using namespace xdev;

using namespace std::literals;

namespace detail {
  template <class F, class... Args, class = decltype(std::declval<F &&>()(std::declval<Args &&>()...))>
  auto constexpr is_valid_impl(int) -> std::true_type;

  template <class F, class... Args>
  auto constexpr is_valid_impl(...) -> std::false_type;

  template <class F>
  constexpr auto is_valid(F &&) {
    return [](auto &&... args) { return decltype(is_valid_impl<F &&, decltype(args) &&...>(int{})){}; };
  }
  struct checker {
    auto static constexpr call(...) { return true; }
  };
  template <auto...>
  struct auto_list {};

}// namespace detail

#define is_constexpr(...) \
  detail::is_valid([](auto checker) -> detail::auto_list<decltype(checker)::call(__VA_ARGS__)> {})(detail::checker{})

static_assert(is_constexpr(std::string_view{""}));
static_assert(!is_constexpr(std::string{""}));

SCENARIO("dict types are accessible as normal dicts", "[core.api.variant.v1.0]") {
  GIVEN("A simple dict") {
    xvar val = xdict{
      {1, "1"},
      {"1", 1},
      {"3", "three"},
    };
    WHEN("elements are accesseed") {
      auto v1  = val["1"];
      auto v1_ = val[1];
      auto v3  = val["3"];
      THEN("the resluts should be consistant") {
        REQUIRE(v1 == 1);
        REQUIRE(v1_ == "1");
        REQUIRE(v3 == "three");
      }
    }
    WHEN("a new element is added") {
      val["new"] = true;
      THEN("the new element exists") { REQUIRE(val["new"] == true); }
      AND_THEN("other elements are still present") {
        REQUIRE(val["1"] == 1);
        REQUIRE(val[1] == "1");
        REQUIRE(val["3"] == "three");
      }
    }
    WHEN("a new element is added with dotted notation") {
      val["root.new"] = true;
      THEN("the root element exists") { REQUIRE(val["root"].is<xdict>()); }
      AND_THEN("the new element exists") { REQUIRE(val["root.new"] == true); }
      AND_THEN("it should also be accessible through accessor") { REQUIRE(val["root"]["new"] == true); }
      AND_THEN("other elements are still present") {
        REQUIRE(val["1"] == 1);
        REQUIRE(val[1] == "1");
        REQUIRE(val["3"] == "three");
      }
    }
    WHEN("a new element is added with accessor") {
      val["root"]["new"] = true;
      THEN("the root element exists") { REQUIRE(val["root"].is<xdict>()); }
      AND_THEN("the new element exists") { REQUIRE(val["root"]["new"] == true); }
      AND_THEN("it should also be accessible through the dotted notation") { REQUIRE(val["root.new"] == true); }
      AND_THEN("other elements are still present") {
        REQUIRE(val["1"] == 1);
        REQUIRE(val[1] == "1");
        REQUIRE(val["3"] == "three");
      }
    }
    WHEN("the dict is updated") {
      val.update({
        {"question", "what is the answer to life the universe and everything"},
        {"response", 42},
      });
      THEN("the new elements exists") {
        REQUIRE(val["question"] == "what is the answer to life the universe and everything");
        REQUIRE(val["response"] == 42);
      }
      AND_THEN("other elements are still present") {
        REQUIRE(val["1"] == 1);
        REQUIRE(val[1] == "1");
        REQUIRE(val["3"] == "three");
      }
    }
  }
}


SCENARIO("dict types should be comparable", "[core.api.variant.v1.0]") {
  GIVEN("A simple list") {
    xvar val = xdict{
      {1, "1"},
      {"1", 1},
      {"3", "three"},
    };
    WHEN("the dict is compared with actual values") {
      auto result = val == xdict{
                             {1, "1"},
                             {"1", 1},
                             {"3", "three"},
                           };
      THEN("the resluts should be consistant") { REQUIRE(result == true); }
    }
    WHEN("the list is compared with other values") {
      auto result = val == xdict{
                             {1, "1"},
                             {"1", 1},
                             {"3", "tree"},
                           };
      THEN("the resluts should be consistant") { REQUIRE(result == false); }
    }
  }
}
SCENARIO("dict types should be iterable", "[core.api.variant.v1.0]") {
  GIVEN("A simple list") {
    xvar val = xdict{
      {1, "1"},
      {"1", 1},
      {"3", "three"},
    };
    THEN("a normal iteration can be realized") {
      for (auto &&[k, v] : val.get<xdict>()) { fmt::print("- {:f}: {:f}\n", k, v); }
    }
  }
}
SCENARIO("dict types should accessible via dotted notation", "[core.api.variant.v1.0]") {
  GIVEN("A simple dict") {
    xvar val = xdict{{1, "1"}, {"1", 1}, {"3", "three"}, {"root", xdict{{"test", "OK"}}}};
    THEN("it should be accessible via dotted notation") {
      REQUIRE(val.get<xdict>().dotAt("root.test") == "OK");
    }
  }
}
