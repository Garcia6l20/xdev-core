#include <xdev/xdev-variant.hpp>
#include <xdev/xdev-json.hpp>
#include <xdev/xdev-yaml.hpp>

using namespace xdev;
using namespace xdev::variant;

Variant Variant::FromJSON(const std::string& input) {
    return json::parse(input);
}

Variant Variant::FromYAML(const std::string& input) {
    return yaml::parse(input);
}
