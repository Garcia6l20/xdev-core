/**
 * @file xdev-basic-rc.hpp
 */
#pragma once

#include <xdev.hpp>
#include <xdev/xdev-rc-lib-export.hpp>

#include <list>

namespace xdev::rc
{
class XDEV_RC_LIB_EXPORT XBasicResourceCompiler {
public:
	class CompileError : public XException {
	public:
        using XException::XException;
	};

    XBasicResourceCompiler(const string& name, const filesystem::path& resource_path, bool verbose = false);
    void compile();

private:
	void processFile(const filesystem::path& path, const string& root);
	void createResourceFiles();

	string m_name;
	filesystem::path m_path;
	bool _verbose = false;

	struct rc_map {
		std::string key;
		std::string resource_path;
		std::string resource_name;
	};
	list<rc_map> m_rcMaps;
};
} // namespace xdev::rc
