#include <xdev/xdev-library.hpp>
#include <xdev/xdev-exception.hpp>

#include <dlfcn.h>

#include <xdev/xdev-tools.hpp>

#include <spdlog/spdlog.h>
#include <fmt/format.h>

namespace xdev {

namespace fs = filesystem;

XLibrary::XLibrary(const string &name):
    _name(name),
    _handle(nullptr)
{
    if (fs::exists(_name)) {
        _path = _name;
        _name = _path.stem().string();
    } else {
        vector<fs::path> paths = {""s};
        if (const char * env_paths = getenv("XDEV_LIB_PATH")) {
            for(auto p: tools::split(env_paths, ':', true)) {
                paths.push_back(p);
            }
        }
        for (auto p: paths) {
            auto fpath = p / fs::path{"lib" + name + ".so"};
            if (fs::exists(fpath)) {
                _path = fs::absolute(fpath);
                spdlog::debug("XLibrary: {} found at {}", name, _path.string());
                break;
            }
        }
    }

    if (_path.empty()) {
        throw XException(fmt::format("Cannot find library: {}", _name));
    }
    _handle = dlopen(_path.string().c_str(), RTLD_LAZY);
    if (_handle == nullptr) {
        throw XException(fmt::format("Failed to load library: {} from {} ({})", _name, _path.string(), dlerror()));
    }
}

XLibrary::~XLibrary() {
    if (_handle != nullptr) {
        dlclose(_handle);
        spdlog::debug("XLibrary: {} released", _name);
    }
    _handle = nullptr;
}

XLibrary::ptr XLibrary::Load(const string &name) {
    return make_shared<XLibrary>(name);
}

}
