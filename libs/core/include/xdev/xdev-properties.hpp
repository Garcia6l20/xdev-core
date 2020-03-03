/**
 * @file xdev-properties.hpp
 * @author Garcia Sylvain
 */
#pragma once

#include <string>
#include <map>
#include <typeinfo>
#include <typeindex>
#include <ctti/type_id.hpp>

#include <xdev/xdev-exception.hpp>

namespace xdev {

using namespace std;

namespace variant {
    class Variant;
    class Dict;
}

using XVariant = variant::Variant;
using XDict = variant::Dict;

class XObjectBase;

enum PropertyAccess {
    ReadWrite,
    ReadOnly,
};
enum PropertyKind {
    Normal,
    Static,
};

class XMetaPropertyBase
{
public:

    struct IllegalAccess: XException {
        using XException::XException;
    };

    using list_t = vector<XMetaPropertyBase>;
    using map_t = map<string, XMetaPropertyBase>;
    using setter_t = function<void(XObjectBase*, const XVariant&)>;
    using getter_t = function<XVariant(XObjectBase*)>;
    XMetaPropertyBase(const string& name,
                  const type_index& typeindex,
                  const ctti::type_id_t& type_id,
                  PropertyAccess access = ReadWrite,
                  PropertyKind kind = Normal) :
        _name(name),
        _typeId(type_id),
        _typeIndex(typeindex),
        _access(access),
        _kind(kind)
    {
    }
    const string& name() const { return _name; }
    string typeName() const { return _typeId.name().str(); }
    const ctti::type_id_t& typeId() const { return _typeId; }
    const type_index& typeIndex() const { return _typeIndex; }
    inline PropertyAccess access() const { return _access; }
    inline PropertyKind kind() const { return _kind; }
private:
    string _name;
    ctti::type_id_t _typeId;
    type_index _typeIndex;
    PropertyAccess _access;
    PropertyKind _kind;
};

template <typename T, PropertyAccess t_access = ReadWrite, PropertyKind t_kind = Normal>
class XMetaProperty: public XMetaPropertyBase
{
public:
    XMetaProperty(const string& name) :
        XMetaPropertyBase(name, typeid(T), ctti::type_id<T>(), t_access, t_kind)
    {
    }
};

struct XPropertyBase;

/**
 * @brief Safe property listener
 *
 * PropertyListenerBase's derived objects handles automatic unregistration when the listener object is destroyed.
 * Listeners object can only be created from XPropertyBase object.
 */
struct PropertyListenerBase {
    using ptr = shared_ptr<PropertyListenerBase>;
    using wptr = weak_ptr<PropertyListenerBase>;
    inline virtual ~PropertyListenerBase();
    virtual void notify(const XVariant&) const = 0;
protected:
    inline PropertyListenerBase(const reference_wrapper<XPropertyBase>& prop);
    reference_wrapper<XPropertyBase> _property;
    bool _valid = true;
    friend struct XPropertyBase;
};

template <typename T = XVariant>
struct PropertyListener: PropertyListenerBase {
    typedef void(*TargetType)(T);
    using WatchFunc = function<void(T)>;
    PropertyListener(WatchFunc watcher, const reference_wrapper<XPropertyBase>& prop);
    virtual ~PropertyListener() override;
    void notify(const XVariant&) const override;
private:
    WatchFunc _watcher;
};

struct XPropertyBase {
    using Access = PropertyAccess;
    using Kind = PropertyKind;
    using IllegalAccess = XMetaPropertyBase::IllegalAccess;
    inline virtual void operator=(const XVariant&);
    inline virtual XVariant value() const;
    ctti::type_id_t typeId() const { return _typeId; }

    template <typename T>
    bool operator==(T&& other) const;

    template <typename T>
    bool operator==(const T& other) const;

    inline virtual ~XPropertyBase();
    inline void listen(const PropertyListenerBase::ptr& listener);
    inline void stopListening(const PropertyListenerBase::ptr&);
    inline void stopListening(const PropertyListenerBase*);
    template <typename ListenT = XVariant>
    inline void stopListening(const PropertyListenerBase::ptr&);
    template <typename ListenT = XVariant>
    const PropertyListenerBase::ptr listen(typename PropertyListener<ListenT>::WatchFunc watch);

    template <typename T>
    bool is() {
        return ctti::type_id<T>() == typeId();
    }
    template <typename T>
    T& get();
    template <typename T>
    const T& get() const;

protected:
    inline XPropertyBase(const ctti::type_id_t&type_id, Access access = Access::ReadWrite, Kind kind = Kind::Normal);
    ctti::type_id_t _typeId;
    Access _access;
    Kind _kind;
    vector<PropertyListenerBase::wptr> _listeners;
};

template <typename T, XPropertyBase::Access access = XPropertyBase::Access::ReadWrite>
struct property: XPropertyBase {
    property(): XPropertyBase(ctti::type_id<T>(), access) {
    }
    property(const T& value): property() { _value = value; }
    virtual void operator=(const XVariant&) override;
    virtual XVariant value() const override;

    T& operator*();
    const T& operator*() const;

    T& operator=(const T&);

    template <typename Lambda>
    const PropertyListenerBase::ptr listen(Lambda watcher) {
        return XPropertyBase::listen<T>(typename PropertyListener<T>::WatchFunc(watcher));
    }
    template <typename OtherT>
    bool operator==(const OtherT& other) const {
        return _value == other;
    }
    template <typename OtherT>
    bool operator!=(const OtherT& other) const {
        return _value != other;
    }

    const auto& as_const() const {
        return std::add_const_t<decltype(*this)>(*this);
    }
private:
    T _value;
};

} // xdev

#include <xdev/xdev-properties.inl>
