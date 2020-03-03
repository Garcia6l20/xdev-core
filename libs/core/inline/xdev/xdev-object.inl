#include <xdev/xdev-variant.hpp>

#include <xdev/xdev-object.hpp>
#include <xdev/xdev-exception.hpp>

namespace std {

template <xdev::XObjectPointer XObjectPointerT>
ostream& operator<<(ostream& stream, const XObjectPointerT& obj) {
    if (!obj) {
        stream << "null[" << obj->staticClass().name() << "]";
    } else {
        stream << obj->objectName() << "[" << obj->staticClass().name() << "]";
    }
    return stream;
}

}

namespace xdev {


template<typename ObjT>
typename ObjT::ptr XStaticClass::Make() {
    XObjectBase::ptr instance = { new ObjT(), [](ObjT* ptr){        
        ptr->destroy();
        delete ptr;
    }};
    spdlog::debug("XObjectBase: creating {}", instance->staticClass().name());
    instance->_init();
    return std::dynamic_pointer_cast<ObjT>(instance);
}

//
// Event
//

Connection::Connection(XFunction func, Event* event): _event(event), _function(new XFunction(func)) {
}

Connection::~Connection() {
    if (_valid)
        _event->disconnect(this);
    delete _function;
}

void Connection::operator()(XArray&&array) {
    (*_function)(xfwd(array));
}

template <typename...ArgsT>
void Connection::operator()(ArgsT&&...args) {
    (*_function)({xfwd(args)...});
}

Event::~Event() {
    for (auto conn: _connections) {
        if (auto c = conn.lock(); c != nullptr)
            c->_valid = false;
    }
}

void Event::operator()(const XArray&arg_list) const {
    lock_guard lock(_mut);
    for (auto conn: _connections) {
        if (auto c = conn.lock(); c != nullptr)
            (*c)(arg_list);
    }
}

template <typename...ArgsT>
void Event::operator()(ArgsT&&...args) const {
    for (auto conn: _connections) {
        if (auto c = conn.lock(); c != nullptr)
            (*c)(xfwd(args)...);
    }
}

Connection::ptr Event::connect(XFunction target) {
    lock_guard lock(_mut);
    auto conn = make_shared<Connection>(target, this);
    _connections.push_back(conn);
    return conn;
}


void Event::disconnect(const Connection::ptr &conn) {
    lock_guard lock(_mut);
    _connections.erase(remove_if(_connections.begin(), _connections.end(), [&conn] (auto&&c) {
        return c.lock() == nullptr || c.lock() == conn;
    }));
}

void Event::disconnect(Connection* conn) {
    lock_guard lock(_mut);
    _connections.erase(remove_if(_connections.begin(), _connections.end(), [&conn] (auto&&c) {
        return c.lock() == nullptr || c.lock().get() == conn;
    }));
}
//
// event
//

template <typename...ArgsT>
void event<ArgsT...>::operator()(ArgsT&&...args) {
    Event::operator()(xfwd(args)...);
}

template <typename...ArgsT>
void event<ArgsT...>::operator()(const ArgsT&...args) {
    Event::operator()(args...);
}

//
// XObject::XStaticClass::XPropertyBase
//

//
// PropertyListener
//
PropertyListenerBase::PropertyListenerBase(const reference_wrapper<xdev::XPropertyBase> &prop):
    _property(prop) {
}
PropertyListenerBase::~PropertyListenerBase() {
    if (_valid)
        _property.get().stopListening(this);
}

template <typename T>
PropertyListener<T>::PropertyListener(WatchFunc watcher, const reference_wrapper<XPropertyBase>& prop):
    PropertyListenerBase(prop),
    _watcher(watcher) {
}

template <typename T>
PropertyListener<T>::~PropertyListener() {

}

template <typename T>
void PropertyListener<T>::notify(const XVariant& value) const {
    if constexpr (is_same_v<XVariant, T>)
        _watcher(value);
    else _watcher(value.get<T>()); // convert ???
}

//
// XProperty
//

XPropertyBase::XPropertyBase(const ctti::type_id_t&type_id, Access access, Kind kind):
    _typeId(type_id), _access(access), _kind(kind) {
}

XPropertyBase::~XPropertyBase() {
}

void XPropertyBase::listen(const PropertyListenerBase::ptr &listener) {
    listener->notify(value());
    _listeners.push_back(listener);
}

void XPropertyBase::stopListening(const xdev::PropertyListenerBase::ptr &listener) {
    stopListening(listener.get());
    listener->_valid = false;
}

void XPropertyBase::stopListening(const xdev::PropertyListenerBase* listener) {
    _listeners.erase(remove_if(_listeners.begin(), _listeners.end(), [&listener](PropertyListenerBase::wptr&ref) {
        if (ref.expired())
            return true;
        else return ref.lock().get() == listener;
    }));
}

template <typename ListenT>
const PropertyListenerBase::ptr XPropertyBase::listen(typename PropertyListener<ListenT>::WatchFunc watch) {
    auto listener = make_shared<PropertyListener<ListenT>>(watch, *this);
    listen(listener);
    return listener;
}

void XPropertyBase::operator=(const XVariant&) {
    throw IllegalAccess("invalid usage of XPropertyBase");
}

XVariant XPropertyBase::value() const {
    throw IllegalAccess("invalid usage of XPropertyBase");
}

template <typename T>
bool XPropertyBase::operator==(T&& other) const {
    if constexpr (is_base_of_v<XPropertyBase, T>)
        return value() == other.value();
    else return value() == other;
}

template <typename T>
bool XPropertyBase::operator==(const T& other) const {
    if constexpr (is_base_of_v<XPropertyBase, T>)
        return value() == other.value();
    else return value() == other;
}

template <typename T>
T& XPropertyBase::get() {
    // TODO(me) handle access
    if constexpr (is_xobject<T>)
        return (*dynamic_cast<property<XObjectBase::ptr>&>(*this))->cast<T>();
    else return *dynamic_cast<property<T>&>(*this);
}

template <typename T>
const T& XPropertyBase::get() const {
    // TODO(me) handle access
    if constexpr (is_xobject<T>)
        return (*dynamic_cast<property<XObjectBase::ptr>&>(*this))->cast<T>();
    else return *dynamic_cast<property<T>&>(*this);
}

template <typename T, XPropertyBase::Access access>
void property<T, access>::operator=(const XVariant& value) {
    if constexpr (access > Access::ReadWrite) {
        throw IllegalAccess("property is readonly");
    } else {
        _value = value.convert<T>(); // make sure is convertible to T
    }
    for (auto wlit = _listeners.begin(); wlit != _listeners.end(); ++wlit) {
        if (auto listener = wlit->lock(); listener != nullptr) {
            listener->notify(value);
        } else {
            _listeners.erase(wlit--);
        }
    }
}

template <typename T, XPropertyBase::Access access>
T& property<T, access>::operator=(const T& value) {
    if constexpr (access > Access::ReadWrite) {
        static_assert (always_false<T>::value, "property is readonly");
        throw IllegalAccess("property is readonly");
    } else {
        _value = value;
        for (auto wlit = _listeners.begin(); wlit != _listeners.end(); ++wlit) {
            if (auto listener = wlit->lock(); listener != nullptr) {
                listener->notify(value);
            } else {
                _listeners.erase(wlit--);
            }
        }
        return _value;
    }
}

template <typename T, XPropertyBase::Access access>
XVariant property<T, access>::value() const {
    return _value;
}

template <typename T, XPropertyBase::Access access>
T& property<T, access>::operator*() {
    if constexpr (access > Access::ReadWrite) {
        static_assert (always_false<T>::value, "property is readonly");
        throw IllegalAccess("property is readonly");
    } else {
        return _value;
    }
}

template <typename T, XPropertyBase::Access access>
const T& property<T, access>::operator*() const {
    return _value;
}

//
// XObject::XStaticClass::XMetaData
//

ObjectMetadata& XStaticClass::objectMetadata(XObjectBase* object) const {
    return object->_metaData;
}

//
// XObject
//

const string& XObjectBase::objectName() const {
    return _metaData.objectName;
}

void XObjectBase::setObjectName(const string& name) {
    _metaData.objectName = name;
}

bool XObjectBase::has_prop(const std::string& name) const {
    return _metaData.properties.contains(name);
}

template <typename T>
inline T& XObjectBase::prop(const string& name) {
    return prop(name).get<T>();
}

template <typename T>
inline const T& XObjectBase::prop(const string& name) const {
    return prop(name).get<T>();
}

XPropertyBase& XObjectBase::prop(const string& name) try {
    return _metaData.properties.at(name).get();
} catch (const out_of_range& e) {
    throw NoSuchProperty("cannot access property " + name, e);
}

const XPropertyBase& XObjectBase::prop(const string& name) const try {
    return _metaData.properties.at(name).get();
} catch (const out_of_range& e) {
    throw NoSuchProperty("cannot access property " + name, e);
}

void XObjectBase::bind(const string& prop_name, const XObjectBase::ptr& source, string source_pname) {
    if (source_pname.empty())
        source_pname = prop_name;
    auto listener = source->listen(source_pname, [&](auto value) {
        this->prop(prop_name) = value;
    });
    _metaData.bounded_props.push_back(listener);
}

void XObjectBase::bind(const XObjectBase::ptr& source) {
    for (auto& [name, prop]: _metaData.properties) {
        try {
            auto listener = source->listen(name, [&](auto value) {
                prop.get() = value;
            });
            _metaData.bounded_props.push_back(listener);
        } catch (const NoSuchProperty&) {}
    }
}

template <typename ListenerT>
PropertyListenerBase::ptr XObjectBase::listen(const string& prop_name, ListenerT handler) {
    for (auto& [name, prop]: _metaData.properties) {
        if (name == prop_name) {
            return prop.get().listen(handler);
        }
    }
    throw NoSuchProperty("Property not found: " + prop_name);
}

template <typename...ArgsT>
auto XObjectBase::apply(string&& method, tuple<ArgsT...>&& args) {
    return std::apply(_metaData.functions.at(forward<string>(method)), forward<tuple<ArgsT...>>(args));
}

template <typename ResultT, typename...ArgsT>
ResultT XObjectBase::call(string&& method, ArgsT&&...args) {
    return _metaData.functions.at(forward<string>(method)).operator()<ResultT>(forward<ArgsT>(args)...);
}

template <typename ResultT, typename...ArgsT>
ResultT XObjectBase::call(const string& method, ArgsT&&...args) {
    return _metaData.functions.at(method).operator()<ResultT>(forward<ArgsT>(args)...);
}

XVariant XObjectBase::call(string&& method) {
    return _metaData.functions.at(forward<string>(method))({});
}

XVariant XObjectBase::call(string&& method, XArray&&args) {
    return _metaData.functions.at(forward<string>(method))(forward<XArray>(args));
}

XVariant XObjectBase::call(const string& method, const XArray&args) {
    return _metaData.functions.at(method)(args);
}

XVariant XObjectBase::apply(const string& method, const XArray&args) {
    return _metaData.functions.at(method).apply(args);
}

void XObjectBase::connect(string&& event_name, const XObjectBase::ptr& target, string&& function_name) {
    auto& src = _metaData.events.at(xfwd(event_name)).get();
    auto& dst = target->_metaData.functions.at(xfwd(function_name));
    target->_metaData.connections.push_back(src.connect(dst));
}

XFunction XObjectBase::method(const std::string& name) const {
    return _metaData.functions.at(name);
}

} // namespace xdev


#pragma once

#include <xdev/xdev-variant-fmt.hpp>

//
// fmt formatter
//

template <>
struct fmt::formatter<xdev::XPropertyBase>: fmt::formatter<xdev::XVariant> {
    using base = fmt::formatter<xdev::XVariant>;

    // Formats the point p using the parsed format specification (presentation)
    // stored in this formatter.
    template <typename FormatContext>
    auto format(const xdev::XPropertyBase& v, FormatContext& ctx) {
        return base::format(v.value(), ctx);
    }
};

