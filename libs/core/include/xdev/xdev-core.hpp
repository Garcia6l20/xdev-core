/**
 * @file xdev-core.hpp
 * @author Garcia Sylvain
 */
#pragma once

#include <xdev/xdev-core-export.hpp>

#include <string>
#include <filesystem>
#include <regex>

namespace xdev {
	using namespace std;
#ifdef _WIN32
	namespace filesystem = std::filesystem;
#endif
}
