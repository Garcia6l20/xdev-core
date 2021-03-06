#include <xdev/xdev-xclass.hpp>

#include <spdlog/spdlog.h>

namespace xdev {

void XClass::Register(const XStaticClass::ptr &clazz, create_func_t create) {
    spdlog::debug("XClass: class registered: {}", clazz->name());
    Get()._classes.push_back({clazz, create});
}

void XClass::UnRegister(const XStaticClass* clazz) {
    spdlog::debug("XClass: class unregistered: {}", clazz->name());
    Get()._classes.erase(remove_if(Get()._classes.begin(), Get()._classes.end(), [clazz](auto& item) {
        return item == clazz;
    }));
}

XObjectBase::ptr XClass::Create(const string &name) {
    for (auto& clazz: Get()._classes) {
        if (clazz == name) {
            return clazz.create();
        }
    }
    throw out_of_range("class " + name + " not found");
}

vector<XStaticClass::ptr> XClass::Classes() {
    vector<XStaticClass::ptr> out;
    for (const auto& clazz: Get()._classes) {
        out.push_back(clazz.staticClass.lock());
    }
    return out;
}

XStaticClass::ptr XClass::Class(const string &name) {
    for (auto& clazz: Get()._classes) {
        if (clazz == name) {
            return clazz.staticClass.lock();
        }
    }
    throw out_of_range("class " + name + " not found");
}

XClass& XClass::Get() {
    static XClass instance;
    return instance;
}

}
