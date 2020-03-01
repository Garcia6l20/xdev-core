/**
 * @file xdev-library.hpp
 */
#pragma once

#include <xdev/xdev-core.hpp>
#include <xdev/xdev-core-export.hpp>

namespace xdev {

class XDEV_CORE_EXPORT XLibrary: public std::enable_shared_from_this<XLibrary>
{
public:
    using ptr = shared_ptr<XLibrary>;
    static ptr Load(const string& name);
    ~XLibrary();
    XLibrary(const string& name);
    const filesystem::path& path() const { return _path; }
    const std::string& name() const { return _name; }
    bool operator==(const filesystem::path& p) const {
        return _path == p;
    }
    bool operator==(const string& name) const {
        return _name == name;
    }
private:
    string _name;
    filesystem::path _path;
    void *_handle;
};

}
