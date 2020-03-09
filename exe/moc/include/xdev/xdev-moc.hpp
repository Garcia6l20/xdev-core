/**
 * @file xdev-moc.hpp
 * @author Garcia Sylvain
 */
#pragma once

#include <xdev/xdev-moc-lib-export.hpp>

#include <xdev/xdev-core.hpp>
#include <xdev/xdev-exception.hpp>
#include <xdev/xdev-template.hpp>

namespace xdev {
class XDEV_MOC_LIB_EXPORT XMetaObjectCompiler
{
public:

	class CompileError : public XException
	{
	public:
		CompileError(const string& what);
		CompileError(const string& what, const string& file_path, int line);
		inline const string& filePath() const { return m_filePath;  }
		inline int line() const { return m_line;  }
	private:
		string m_filePath;
		int m_line;
	};


    XMetaObjectCompiler(const std::string& project_name, const std::vector<xdev::filesystem::path>& files);
	void compile();
private:
    string m_projectName;
    std::vector<xdev::filesystem::path> m_files;
	std::string m_macroArgs;

    void createPools();
    void registerPools(const filesystem::path& path);
    void processPools(const filesystem::path& path);
	void processFile(const filesystem::path& path);

    xtemplate::ptr m_headerTemplate;
    xtemplate::ptr m_sourceTemplate;
    xtemplate::ptr m_poolsHeaderTemplate;
    xtemplate::ptr m_poolsSourceTemplate;

    xvar m_pools;
    xvar m_poolsIncludes;
};
}
