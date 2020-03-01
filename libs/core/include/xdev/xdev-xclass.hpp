/**
 * @file xdev-xclass.hpp
 */
#pragma once

#include <xdev/xdev-object.hpp>
#include <xdev/xdev-core-export.hpp>

namespace xdev {

class XDEV_CORE_EXPORT XClass
{
public:
    using create_func_t = XObjectBase::ptr (*)();
    static void Register(const XStaticClass::ptr& clazz, create_func_t create);
    static void UnRegister(const XStaticClass* clazz);
    template <typename ObjectT>
    static typename ObjectT::ptr Create(const string& name);
    static XObjectBase::ptr Create(const string& name);
    static vector<XStaticClass::ptr> Classes();
    template <typename BaseObjectT>
    static vector<XStaticClass::ptr> SubClassesOf();
    static XStaticClass::ptr Class(const string& name);
private:
    static XClass& Get();
    struct XDEV_CORE_EXPORT ClassData {
        XStaticClass::wptr staticClass;
        create_func_t create;
        ClassData(const XStaticClass::ptr& clazz, create_func_t create_func): staticClass(clazz), create(create_func) {}
        bool operator==(const string& name) const {
            return staticClass.lock()->name() == name;
        }
        bool operator==(const XStaticClass* clazz) const {
            return staticClass.lock().get() == clazz;
        }
    };
    vector<ClassData> _classes;
};

template<typename ObjectT>
typename ObjectT::ptr XClass::Create(const string &name) {
    return Create(name)->cast<ObjectT>();
}

template <typename BaseObjectT>
vector<XStaticClass::ptr> XClass::SubClassesOf() {
    throw std::runtime_error("not implemented");
}

}
