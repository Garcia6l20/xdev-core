#pragma once

#include <xdev/xdev-variant.hpp>

//
// fmt formatter
//

template <>
struct fmt::formatter<xdev::XVariant>: fmt::formatter<std::string> {
  using base = fmt::formatter<std::string>;
  // presentation format 'j' - json, 't' - typename, 'f' - full <value>[<type>]
  char presentation = 'j';
  constexpr auto parse(format_parse_context& ctx) {
    // Parse the presentation format and store it in the formatter:
    auto it = ctx.begin(), end = ctx.end();
    if (it != end && (*it == 'j' || *it == 't' || *it == 'f')) presentation = *it++;

    // Check if reached the end of the range:
    if (it != end && *it != '}')
      throw format_error("invalid format");

    // Return an iterator past the end of the parsed range:
    return it;
  }

  // Formats the point p using the parsed format specification (presentation)
  // stored in this formatter.
  template <typename FormatContext>
  auto format(const xdev::XVariant& v, FormatContext& ctx) {
      return format_to(
              ctx.out(),
              presentation == 'j' ? "{0}" : presentation == 't' ? "{1}" : "{0}[{1}]",
              v.toString(), v.typeName());
  }
};
