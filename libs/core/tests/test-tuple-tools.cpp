#include <catch2/catch.hpp>

#include <xdev/tuple-tools.hpp>
#include <ctti/type_id.hpp>

#include <spdlog/spdlog.h>
#include <ctti/nameof.hpp>

using namespace xdev;

SCENARIO("tuples should be iterables", "[xdev-core][tuple-tools]") {
  GIVEN("A tuple") {
    using tuple_type = std::tuple<bool, int, std::string_view>;
    tuple_type tuple {true, 42, "any question ?"};
    WHEN("i iterate over it") {
      size_t index = 0;
      tt::foreach (tuple, [&index]<typename T>(T const &elem) {
        ++index;
        if constexpr (std::same_as<T, bool>) {
          REQUIRE(elem == true);
        } else if constexpr (std::same_as<T, int>) {
          REQUIRE(elem == 42);
        } else if constexpr (std::same_as<T, std::string_view>) {
          REQUIRE(elem == std::string_view {"any question ?"});
        } else {
          auto type_name = ctti::type_id(elem).name();
          FAIL("got unwanted type: " + std::string {std::begin(type_name), std::end(type_name)});
        }
      });
      THEN("all variants should have been iterated") { REQUIRE(index == 3); }
    }
  }
}

SCENARIO("foreach loop is breakable", "[xdev-core][tuple-tools]") {
  GIVEN("A tuple") {
    using tuple_type = std::tuple<bool, int, std::string_view>;
    static constexpr tuple_type tuple {true, 42, "any question ?"};
    WHEN("i iterate over it") {
      constexpr auto result = tt::foreach (tuple, [index = 0]<typename T>(T const &elem) mutable {
        ++index;
        if constexpr (std::same_as<T, bool>) {
        } else if constexpr (std::same_as<T, int>) {
          return std::make_tuple(index, elem);
        } else {
          throw std::runtime_error {"nope !"};
        }
      });
      THEN("all variants should have been iterated until 42") {
        STATIC_REQUIRE(std::get<0>(result) == 2);
        STATIC_REQUIRE(std::get<1>(result) == 42);
      }
    }
  }
}

SCENARIO("tt::transform can manipulate types", "[xdev-core][tuple-tools]") {
  GIVEN("A tuple") {
    using tuple_type = std::tuple<bool, int, std::string_view>;
    static constexpr tuple_type tuple {true, 42, "any question ?"};
    WHEN("i iterate over it") {
      constexpr auto result = tt::transform(tuple, []<typename T>(T const &elem) mutable {
        if constexpr (std::same_as<T, bool>) {
          return !elem;
        } else if constexpr (std::same_as<T, int>) {
          return std::make_tuple(elem, elem * elem);
        }
      });
      THEN("all variants should have been iterated until 42") {
        STATIC_REQUIRE(std::tuple_size_v<decltype(result)> == 2);
        STATIC_REQUIRE(tt::at<0>(result) == false);
        STATIC_REQUIRE(tt::at<1>(result) == std::make_tuple(42, 1764));
      }
    }
  }
}

template<class T>
concept has_test_type = requires() {
  typename T::test;
};
struct test_with_test {
  using test = std::true_type;
};
struct test_without_test {};

SCENARIO("tt::transform can be used on tuple types", "[xdev-core][tuple-tools]") {
  GIVEN("A tuple") {
    using tuple_type = std::tuple<test_with_test, test_without_test>;
    WHEN("i iterate over it") {
      using filtered_type = tt::transform_t<tuple_type, []<typename T>(T) mutable {
        if constexpr (has_test_type<T>) {
          return T {};
        }
      }>;
      THEN("...") {
        STATIC_REQUIRE(tt::size<filtered_type> == 1);
        STATIC_REQUIRE(xdev::decays_to<tt::at_t<0, filtered_type>, test_with_test>);
      }
    }
  }
}

SCENARIO("tt::transform can be used on tuple types with index", "[xdev-core][tuple-tools]") {
    GIVEN("A tuple") {
        using tuple_type = std::tuple<test_with_test, test_without_test>;
        WHEN("i iterate over it") {
            using filtered_type = tt::transform_t<tuple_type, []<size_t index, typename T>(T) mutable {
                if constexpr (has_test_type<T>) {
                    return std::make_tuple(index, T {});
                }
            }>;
            THEN("...") {
                STATIC_REQUIRE(tt::size<filtered_type> == 1);
                STATIC_REQUIRE(xdev::decays_to<tt::at_t<1, tt::at_t<0, filtered_type>>, test_with_test>);
                STATIC_REQUIRE(xdev::decays_to<tt::at_t<0, tt::at_t<0, filtered_type>>, size_t>);
            }
        }
    }
}

SCENARIO("tt::transform can make reference tuples", "[xdev-core][tuple-tools]") {
  GIVEN("A tuple") {
    auto original = std::make_tuple(true, 42, "any question ?");
    WHEN("I transform is to reference tuple") {
      auto result = tt::transform(original, []<typename T>(T &elem) mutable {
        if constexpr (decays_to<T, bool>) {
          return std::ref(elem);
        } else if constexpr (decays_to<T, int>) {
          return std::ref(elem);
        }
      });
      spdlog::info("{}", ctti::nameof_v<decltype(result)>.str());
      THEN("we should get 2 references to original data ") {
        STATIC_REQUIRE(tt::size<decltype(result)> == 2);
        STATIC_REQUIRE(std::same_as<tt::at_t<0, decltype(original)> &, tt::at_t<0, decltype(result)>>);
        STATIC_REQUIRE(std::same_as<tt::at_t<1, decltype(original)> &, tt::at_t<1, decltype(result)>>);
        REQUIRE(&tt::at<0>(original) == &tt::at<0>(result));
        REQUIRE(&tt::at<1>(original) == &tt::at<1>(result));
        AND_WHEN("I modify those references") {
          tt::at<0>(result) = false;
          tt::at<1>(result) = 55;
          THEN("original tuple may have been modified") {
            REQUIRE(tt::at<0>(original) == false);
            REQUIRE(tt::at<1>(original) == 55);
          }
        }
      }
    }
  }
}

SCENARIO("tt::transform can be used with index", "[xdev-core][tuple-tools]") {
    GIVEN("A tuple") {
        auto original = std::make_tuple(true, 42, "any question ?");
        WHEN("I transform is to reference tuple") {
            auto result = tt::transform(original, []<size_t index, typename T>(T &elem) mutable {
                if constexpr (decays_to<T, bool>) {
                    return std::make_tuple(index, std::ref(elem));
                } else if constexpr (decays_to<T, int>) {
                    return std::make_tuple(index, std::ref(elem));
                }
            });
            spdlog::info("{}", ctti::nameof_v<decltype(result)>.str());
            THEN("we should get 2 references to original data ") {
                STATIC_REQUIRE(tt::size<decltype(result)> == 2);
                STATIC_REQUIRE(std::same_as<tt::at_t<0, decltype(original)> &,
                                   tt::at_t<1, tt::at_t<0, decltype(result)>>>);
                STATIC_REQUIRE(std::same_as<tt::at_t<1, decltype(original)> &,
                                   tt::at_t<1, tt::at_t<1, decltype(result)>>>);
                REQUIRE(&tt::at<0>(original) == &tt::at<1>(tt::at<0>(result)));
                REQUIRE(tt::at<0>(tt::at<0>(result)) == 0);
                REQUIRE(&tt::at<1>(original) == &tt::at<1>(tt::at<1>(result)));
                REQUIRE(tt::at<0>(tt::at<1>(result)) == 1);
                AND_WHEN("I modify those references") {
                    tt::at<1>(tt::at<0>(result)) = false;
                    tt::at<1>(tt::at<1>(result)) = 55;
                    THEN("original tuple may have been modified") {
                        REQUIRE(tt::at<0>(original) == false);
                        REQUIRE(tt::at<1>(original) == 55);
                    }
                }
            }
        }
    }
}

SCENARIO("tt::generate can generate tuples", "[xdev-core][tuple-tools]") {
    GIVEN("A number N of items") {
        constexpr size_t n_items = 42;
        WHEN("I generate a tuple of of N items") {
            constexpr auto result = tt::generate([]<size_t index>() {
                if constexpr (index >= n_items) {
                    return; // break tt::generate - note: never invoked
                } else {
                    return index;
                }
            });
            THEN("It shall contains N items") {
                STATIC_REQUIRE(tt::size<decltype(result)> == n_items);
                STATIC_REQUIRE(tt::at<0>(result) == 0);
                STATIC_REQUIRE(tt::at<n_items - 1>(result) == n_items - 1);
            }
        }
    }
}

SCENARIO("tt::flatten can flatten tuple of tuples", "[xdev-core][tuple-tools]") {
  GIVEN("An input tuple containing an other tuple") {
    constexpr auto input_tuple = std::make_tuple(1, std::make_tuple(2, std::make_tuple(3)));
    WHEN("I flatten it") {
      constexpr auto flat_tuple = tt::flatten<true>(input_tuple);
      THEN("It shall contains N items") {
        STATIC_REQUIRE(flat_tuple == std::make_tuple(1, 2, 3));
      }
    }
  }
}
