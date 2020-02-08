#include <xdev/xdev-variant.hpp>
#include <xdev/xdev-json.hpp>

using namespace xdev;
using namespace xdev::variant;

Variant Variant::FromJSON(const std::string& input) {
    return json::parse(input);
}
