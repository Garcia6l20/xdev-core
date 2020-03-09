/**
 * @file xdev-app.hpp
 * @author Garcia Sylvain
 */
#pragma once

#include <xdev/xdev-core-export.hpp>

#include <xdev/xdev-core.hpp>
#include <xdev/xdev-exception.hpp>
#include <xdev/xdev-properties.hpp>
#include <xdev/xdev-concepts.hpp>
#include <xdev/xdev-variant-dict.hpp>

#include <ctti/type_id.hpp>
#include <spdlog/spdlog.h>

#include <typeinfo>
#include <typeindex>
#include <functional>
#include <memory>
#include <map>
#include <set>
#include <vector>
#include <mutex>

/**
 * Allows @p __item to be handled by XDEV moc.
 * Folowing items are valid:
 * - class: Creates a new XClass instanciable by the xdev runtime.
 * - types: int, double, string, XObject::ptr, ...
 *          Create a property with given @p __item as type.
 * - function<return_t(arg1_t, arg2_t, ...)>: Creates an new invokable method.
 * - event<arg1_t, arg2_t, ...>: Creates a new event.
 **/
#define X(__item, ...) \
    __item

/**
 * Macro used to define a static metadata.
 **/
#define XMETADATA(__name, ...)

/**
 * Binds a class method to a MetaFunction
 */
#define XINVOKABLE

namespace xdev {

using namespace std;

namespace variant {
    class Variant;
    class Dict;
    class List;
    struct Function;
} // namespace variant

    using xvar = variant::Variant;
    using xdict = variant::Dict;
    using xlist = variant::List;
    using xfn = variant::Function;


    template <typename T>
    static inline constexpr bool is_xvariant = is_same<T, xvar>::value;

    template <typename T>
    static inline constexpr bool is_xobject = is_base_of<XObjectBase, T>::value;


    class XObjectBase;

    struct XDEV_CORE_EXPORT MetaFunctionBase {
        MetaFunctionBase(const string&p_name, ctti::type_id_t ret, vector<ctti::type_id_t> args):
            name(p_name),
            returnType(ret),
            argumentsTypes(args) {
        }
        string name;
        ctti::type_id_t returnType;
        vector<ctti::type_id_t> argumentsTypes;
    };

    template <typename SignatureT>
    struct MetaFunction; // undefined

    template <typename RetT, typename...ArgsT>
    struct MetaFunction<RetT(ArgsT...)>: MetaFunctionBase {
        MetaFunction(const string&p_name): MetaFunctionBase(p_name, ctti::type_id<RetT>(), {ctti::type_id<ArgsT>()...}) {
        }
    };

    template <typename...ArgsT>
    struct MetaEvent: MetaFunctionBase {
        MetaEvent(const string&p_name): MetaFunctionBase(p_name, ctti::type_id<void>(), {ctti::type_id<ArgsT>()...}) {
        }
    };

    struct Event;

    struct Connection {
        using ptr = shared_ptr<Connection>;
        using wptr = weak_ptr<Connection>;
        inline Connection(xfn, Event*);
        inline ~Connection();
        inline void operator()(xlist&&lst);
        template <typename...ArgsT>
        inline void operator()(ArgsT&&...args);
    protected:
        Event * _event;
        xfn* _function;
        bool _valid = true;
        friend struct Event;
    };

    struct Event {
        Event() = default;
        inline ~Event();
        inline void operator()(const xlist&) const;
        template <typename...ArgsT>
        inline void operator()(ArgsT&&...) const;
        inline Connection::ptr connect(xfn);
        inline void disconnect(const Connection::ptr&);
    private:
        inline void disconnect(Connection*);
        vector<Connection::wptr> _connections;
        mutable mutex _mut;
        friend struct Connection;
    };

    /**
     * @brief End user event type
     */
    template <typename...ArgsT>
    struct event: Event {
        void operator()(ArgsT&&...);
        void operator()(const ArgsT&...);
    };

    struct ObjectMetadata {
        string objectName;
        map<string, xfn> functions = {};
        map<string, reference_wrapper<Event>> events = {};
        map<string, reference_wrapper<XPropertyBase>> properties = {};
        vector<PropertyListenerBase::ptr> bounded_props = {};
        vector<Connection::ptr> connections = {};
    };

    class XDEV_CORE_EXPORT XStaticClass
    {
    public:
        using ptr = shared_ptr<XStaticClass>;
        using wptr = weak_ptr<XStaticClass>;

        XStaticClass(const string& name):
            _name(name),
            _instanceCount(0)
        {
        }
        virtual ~XStaticClass() noexcept = default;

        virtual shared_ptr<XObjectBase> Create() const = 0;

        template<typename XObjectT, typename = enable_if_t<is_base_of_v<XObjectBase, XObjectT>>>
        typename XObjectT::ptr Create() const {
            return dynamic_pointer_cast<XObjectT>(Create());
        }

        const string& name() const { return _name; }


        const xdict& metadata() const
        {
            return _metadata;
        }

        const XMetaPropertyBase::map_t& properties() const
        {
            return _properties;
        }

        XMetaPropertyBase::list_t propertyList() const
        {
            XMetaPropertyBase::list_t lst;
            transform(_properties.begin(), _properties.end(), lst.begin(), [](auto&item) {return item.second;});
            return lst;
        }

        size_t operator++() {
            return ++_instanceCount;
        }

    private:
        string _name;

    protected:
        virtual void _init() {}

        inline ObjectMetadata& objectMetadata(XObjectBase* object) const;

        XMetaPropertyBase::map_t _properties;

        vector<MetaFunctionBase> _functions;
        vector<MetaFunctionBase> _events;
        xdict  _metadata;
        size_t _instanceCount;
        friend class XObjectBase;

        template<typename ObjT>
        static typename ObjT::ptr Make();
    };

    class XDEV_CORE_EXPORT XObjectBase: public enable_shared_from_this<XObjectBase>
    {
    protected:
        XObjectBase();
        size_t _init();

        using BaseStaticClass = XStaticClass;
    public:
        virtual ~XObjectBase() noexcept = default;

        virtual void initialize() {}
        virtual void destroy() {}

        using ptr = shared_ptr<XObjectBase>;

        template<typename T, PropertyAccess access = ReadWrite>
        using property = xdev::property<T, access>;
        template<typename Signature>
        using function = xdev::function<Signature>;
        template<typename Signature>
        using event = xdev::event<Signature>;

        struct NoSuchProperty: XException {
            using XException::XException;
        };

        template <class ObjectClass>
        static ObjectClass* Cast(XObjectBase* object)
        {
            return dynamic_cast<ObjectClass*>(object);
        }

        template <class ObjectClass>
        typename ObjectClass::ptr cast()
        {
            return dynamic_pointer_cast<ObjectClass>(shared_from_this());
        }

        template <class ObjectClass>
        const typename ObjectClass::ptr cast() const
        {
            return dynamic_pointer_cast<ObjectClass>(shared_from_this());
        }

        template <class ObjectClass>
        static typename ObjectClass::ptr Cast(XObjectBase::ptr object)
        {
            return dynamic_pointer_cast<ObjectClass>(object);
        }

        template <class ObjectClass>
        static typename ObjectClass::ptr Create()
        {
            return ObjectClass::StaticClass().Create()->template cast<ObjectClass>();
        }

        inline bool has_prop(const std::string& name) const;

        inline XPropertyBase& prop(const string& name);
        inline const XPropertyBase& prop(const string& name) const;

        template <typename T>
        inline property<T>& prop(const string& name);

        template <typename T>
        inline const property<T>& prop(const string& name) const;

        virtual string toString() const
        {
            return objectName();
        }

        template<typename StaticClassT>
        const StaticClassT& staticClass() const {
            const XStaticClass& clazz = staticClass();
            return dynamic_cast<const StaticClassT&>(clazz);
        }

        virtual const XStaticClass& staticClass() const = 0;

        inline const string& objectName() const;

        inline void setObjectName(const string&);

        static constexpr const char* ctti_nameof()
        {
            return "XObject";
        }

        /**
         * @brief Bind object property to another.
         *
         * The given @p prop_name propery will be automaticaly updated with @p other value.
         *
         * @param prop_name     The property name to bind.
         * @param source        The source of the property value.
         * @param source_pname  The property name of source object to use.
         */
        inline void bind(const string& prop_name, const XObjectBase::ptr& source, string source_pname = "");

        /**
         * @brief Bind all properties.
         * @param source    The source of the properties value.
         * @see bind(string, XObject::ptr, string)
         */
        inline void bind(const XObjectBase::ptr& source);

        template <typename ListenerT>
        inline PropertyListenerBase::ptr listen(const string& prop_name, ListenerT listener);

        template <typename...ArgsT>
        auto apply(string&& method, tuple<ArgsT...>&& args);

        template <typename ResultT = xvar, typename...ArgsT>
        ResultT call(string&& method, ArgsT&&...args);

        template <typename ResultT = xvar, typename...ArgsT>
        ResultT call(const string& method, ArgsT&&...args);

        inline xvar call(string&& method);
        inline xvar call(string&& method, xlist&&args);
        inline xvar call(const string& method, const xlist&args);
        inline xvar apply(const string& method, const xlist&args);

        inline xfn method(const std::string& name) const;

        inline void connect(string&& event_name, const XObjectBase::ptr& target, string&& function_name);

    private:
        ObjectMetadata _metaData;

        virtual XStaticClass& _get_static_class() = 0;
        template<typename StaticClassT>
        const StaticClassT& _get_static_class() {
            XStaticClass& clazz = _get_static_class();
            return dynamic_cast<StaticClassT&>(clazz);
        }

        friend class XStaticClass;
    };

#define _XOBJECT_BASE_EXTEND(ObjectClassT, StaticClassT) \
    public: \
        using ClassType = ObjectClassT; \
        using BaseStaticClass = StaticClassT; \
        using ptr = std::shared_ptr<ClassType>; \
        static auto Create() { \
            return xdev::XObjectBase::Create<ClassType>(); \
        } \
        static xdev::XObjectBase::ptr CreateObject() { \
            return xdev::XObjectBase::Create<ClassType>(); \
        } \
        static const BaseStaticClass& StaticClass() { return _XStaticClass; } \
        virtual const xdev::XStaticClass& staticClass() const override { \
            return dynamic_cast<const xdev::XStaticClass&>(_XStaticClass); \
        } \
    private: \
        static BaseStaticClass& _XStaticClass; \
        virtual xdev::XStaticClass& _get_static_class() override { \
            return dynamic_cast<xdev::XStaticClass&>(_XStaticClass); \
        } \
        friend BaseStaticClass;

    template <class ObjectClassT, class StaticClassT = XStaticClass>
    struct xobj: XObjectBase {
        _XOBJECT_BASE_EXTEND(ObjectClassT, StaticClassT)
    };
}

#include <xdev/xdev-object.inl>
