#include <xdev/ct/tuple.hpp>
#include <fmt/format.h>

using namespace xdev;

using test_tuple1 = ct::tuple<int, bool>;

static_assert(ct::size<test_tuple1> == 2);
static_assert(std::same_as<ct::front_t<test_tuple1>, int>);

using test_tuple2 = ct::push_t<test_tuple1, nullptr_t>;

static_assert(std::same_as<ct::back_t<test_tuple2>, nullptr_t>);

int main() {
  ct::foreach<test_tuple2>([]<typename ElemT>() {
    fmt::print("- {}\n", ctti::nameof<ElemT>().str());
  });
}
