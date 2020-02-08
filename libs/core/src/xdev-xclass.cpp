#include <xdev/xdev-xclass.hpp>

namespace xdev {

shared_ptr<XClass> XClass::_instance = [](){
    struct XClassMaker: XClass {};
    auto instance = make_shared<XClassMaker>();
    if (instance == nullptr)
        throw XException("Failed to create XClass singleton");
    return instance;
}();

XClass::XClass() {
}

XClass::~XClass() {
}

void XClass::Register(const XStaticClass::ptr &clazz, create_func_t create) {
    Get()._classes.push_back({clazz, create});
}

void XClass::UnRegister(const XStaticClass* clazz) {    
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
        out.push_back(clazz.staticClass);
    }
    return out;
}

XStaticClass::ptr XClass::Class(const string &name) {
    for (auto& clazz: Get()._classes) {
        if (clazz == name) {
            return clazz.staticClass;
        }
    }
    throw out_of_range("class " + name + " not found");
}

XClass& XClass::Get() {
    return *_instance;
}

}

