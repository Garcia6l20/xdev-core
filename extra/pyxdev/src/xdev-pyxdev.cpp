#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <spdlog/spdlog.h>

#include <xdev/xdev-library.hpp>
#include <xdev/xdev-xclass.hpp>

using namespace xdev;

namespace py = pybind11;

namespace pybind11::detail {

struct xvariant_to_python {

    return_value_policy _policy;
    handle _parent;

    py::handle operator()(const bool&item) {
        py::bool_ out{item};
        out.inc_ref();
        return out;
    }
    py::handle operator()(const double&item) {
        py::float_ out{item};
        out.inc_ref();
        return out.ptr();
    }
    py::handle operator()(const std::string&item) {
        py::str out{item.c_str()};
        out.inc_ref();
        return out.ptr();
    }
    py::handle operator()(const int&item) {
        py::int_ out{item};
        out.inc_ref();
        return out.ptr();
    }
    py::handle operator()(const XNone&) {
        return py::none{};
    }
    py::handle operator()(XDict& item) {
        return type_caster_base<XDict>::cast(item, _policy, _parent);
    }
    py::handle operator()(XArray&item) {
        return type_caster_base<XArray>::cast(item, _policy, _parent);
    }
    py::handle operator()(XObjectBase::ptr& item) {
        return type_caster_base<XObjectBase>::cast(*item, _policy, _parent);
    }
    py::handle operator()(const XFunction& item) {
        return type_caster_base<XFunction>::cast(item, _policy, _parent);
    }
    py::handle operator()(const XValue&) {
        throw std::runtime_error("might never append");
    }
};

template <> struct type_caster<XVariant>: public type_caster_base<XVariant> {
public:
    /**
     * This macro establishes the name 'inty' in
     * function signatures and declares a local variable
     * 'value' of type inty
     */
    // PYBIND11_TYPE_CASTER(XVariant, _("XVariant"));

    using base = type_caster_base<XVariant>;

    /**
     * Conversion part 1 (Python->C++): convert a PyObject into a inty
     * instance or return false upon failure. The second argument
     * indicates whether implicit conversions should be applied.
     */
    bool load(handle src, bool convert) {
        if (base::load(src, convert)) {
            return true;
        } else if(py::isinstance<py::none>(src)) {
            value = new XVariant(XNone{});
        } else if(py::isinstance<py::bool_>(src)) {
            value = new XVariant(py::cast<bool>(src));
        } else if(py::isinstance<py::int_>(src)) {
            value = new XVariant(py::cast<int>(src));
        } else if(py::isinstance<py::float_>(src)) {
            value = new XVariant(py::cast<double>(src));
        } else if(py::isinstance<py::str>(src)) {
            value = new XVariant(py::cast<std::string>(src));
        } else if(py::isinstance<py::dict>(src)) {
            XDict d;
            for(const auto& [k, v]: py::cast<py::dict>(src)) {
                d[py::cast<XVariant>(k)] = py::cast<XVariant>(v);
            }
            value = new XVariant(d);
        } else {
            return false;
        }
        return true;
    }

    /**
     * Conversion part 2 (C++ -> Python): convert an inty instance into
     * a Python object. The second and third arguments are used to
     * indicate the return value policy and parent object (for
     * ``return_value_policy::reference_internal``) and are generally
     * ignored by implicit casters.
     */
    static handle cast(XVariant&& src, return_value_policy policy, handle parent) {
        return src.visit(xvariant_to_python{policy, parent});
    }
};
}


PYBIND11_MODULE(pyxdev, m) {

    spdlog::set_level(spdlog::level::debug);

    m.doc() = "xdev python plugin"; // optional module docstring

    py::class_<XLibrary, std::shared_ptr<XLibrary>>(m, "Library")
      .def("path", [](const std::shared_ptr<XLibrary>& self) { return self->path().string(); } )
      .def("name", &XLibrary::name);

    py::class_<XVariant>(m, "Variant");

    py::class_<XDict>(m, "Dict")
    .def(py::init<>())
    .def(py::init([](const py::dict& in) {
        auto* self = new XDict();
        for (const auto& [k, v]: in) {
            (*self)[py::cast<XVariant>(k)] = py::cast<XVariant>(v);
        }
        return self;
    }))
    .def("__setitem__", [](XDict& self, py::handle k, py::handle v) {
        self[py::cast<XVariant>(k)] = py::cast<XVariant>(v);
    })
    .def("__getitem__", [](XDict& self, py::handle k) {
        return self[std::forward<XVariant>(py::cast<XVariant&>(k))];
    })
    .def_static("fromJson", [](const std::string& str){
        return XVariant::FromJSON(str).get<XDict>();
    })
    .def_static("fromYaml", [](const std::string& str){
        return XVariant::FromYAML(str).get<XDict>();
    })
    .def("__str__", &XDict::toString);

    py::class_<XArray>(m, "Array")
    .def(py::init<>())
    .def(py::init([](const py::tuple& in) {
        auto* self = new XArray();
        for (const auto& v: in) {
            (*self).push(py::cast<XVariant>(v));
        }
        return self;
    }))
    .def(py::init([](const py::args& in) {
        auto* self = new XArray();
        for (const auto& v: in) {
            (*self).push(py::cast<XVariant>(v));
        }
        return self;
    }))
    .def("__setitem__", [](XArray& self, py::int_ index, py::handle v) {
        self[py::cast<size_t>(index)] = py::cast<XVariant>(v);
    })
    .def("__getitem__", [](XArray& self, py::int_ index) {
        return self[py::cast<size_t>(index)];
    })
    .def("__str__", &XArray::toString);

    py::class_<XFunction>(m, "Function")
    .def("__call__", [](XFunction& self) {
        return self();
    })
    .def("__call__", [](XFunction& self, const py::args& args) {
        XArray a;
        for (const auto& v: args) {
            a.push(py::cast<XVariant>(v));
        }
        return self.apply(std::move(a));
    });

    py::implicitly_convertible<py::dict, XDict>();
    py::implicitly_convertible<py::args, XArray>();
    py::implicitly_convertible<py::tuple, XArray>();
    py::implicitly_convertible<py::list, XArray>();
    py::implicitly_convertible<XArray, XVariant>();
    py::implicitly_convertible<XDict, XVariant>();
    py::implicitly_convertible<XFunction, XVariant>();
    py::implicitly_convertible<XObjectBase::ptr, XVariant>();
    py::implicitly_convertible<XObjectBase, XVariant>();

    py::class_<XObjectBase, std::shared_ptr<XObjectBase>>(m, "Object")
    .def("__getattr__", [](std::shared_ptr<XObjectBase>& self, const std::string& attr)
            -> std::variant<XVariant, std::reference_wrapper<XDict>, XFunction> {
        if (self->has_prop(attr)) {
            auto& prop = self->prop(attr);
            if (prop.is<XDict>())
                return std::ref(prop.get<XDict>());
            return prop.value();
        } else {
            return self->method(attr);
        }
    })
    .def("__setattr__", [](const std::shared_ptr<XObjectBase>& self, const std::string& attr, const XObjectBase::ptr& var){
        return self->prop(attr) = var;
    })
    .def("__setattr__", [](const std::shared_ptr<XObjectBase>& self, const std::string& attr, const XDict& var){
        return self->prop(attr) = var;
    })
    .def("__setattr__", [](const std::shared_ptr<XObjectBase>& self, const std::string& attr, const XArray& var){
        return self->prop(attr) = var;
    })
    .def("__setattr__", [](const std::shared_ptr<XObjectBase>& self, const std::string& attr, const XVariant& var){
        return self->prop(attr) = var;
    });

    m.def("load", &XLibrary::Load, "Loads an xdev library");
    m.def("new", &XClass::Create<XObjectBase>, "Instanciates a new xdev object");
}
