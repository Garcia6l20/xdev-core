/**
 * @file xdev-moc.hpp
 * @author Garcia Sylvain
 */
#pragma once

#include <xdev/xdev-moc-lib-export.hpp>

#include <xdev/core.hpp>
#include <xdev/exception.hpp>
#include <xdev/template.hpp>

#include <span>

namespace xdev {
class XDEV_MOC_LIB_EXPORT XMetaObjectCompiler {
public:
  class CompileError : public XException {
  public:
    CompileError(const string &what);
    CompileError(const string &what, const string &file_path, int line);
    inline const string &filePath() const { return _file_path; }
    inline int line() const { return _line; }

  private:
    string _file_path;
    int _line;
  };


  XMetaObjectCompiler(const xdict &metadata);
  void compile();

private:
  string _project_name;
  filesystem::path _src_path;
  filesystem::path _bin_path;
  xdict _metadata;
  std::string _macroArgs;

  void createPools();
  void registerPools(xdict &meta);
  void processPools(xdict &meta);
  void processFile(xdict &meta);

  xtemplate::ptr _header_template;
  xtemplate::ptr _source_template;
  xtemplate::ptr _pools_header_template;
  xtemplate::ptr _pools_source_template;

  xvar _pools;
  xlist _pools_includes;
};
} // namespace xdev
