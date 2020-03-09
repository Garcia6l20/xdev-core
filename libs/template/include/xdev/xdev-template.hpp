/**
 * @file xdev-template.hpp
 * @author Garcia Sylvain
 */
#pragma once

#include <xdev/xdev-template-blocks.hpp>

namespace xdev {

using xtemplate = temp::RootBlock;

inline xtemplate operator "" _xtemplate(const char* data, size_t size) {
    return *xdev::xtemplate::Compile(std::string{data, size});
}

} // namespace xdev
