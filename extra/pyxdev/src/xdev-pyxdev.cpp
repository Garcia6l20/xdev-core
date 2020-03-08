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
    py::handle operator()(const xnone&) {
        return py::none{};
    }
    py::handle operator()(xdict& item) {
        return type_caster_base<xdict>::cast(item, _policy, _parent);
    }
    py::handle operator()(xlist&item) {
        return type_caster_base<xlist>::cast(item, _policy, _parent);
    }
    py::handle operator()(XObjectBase::ptr& item) {
        return type_caster_base<XObjectBase>::cast(*item, _policy, _parent);
    }
    py::handle operator()(const xfn& item) {
        return type_caster_base<xfn>::cast(item, _policy, _parent);
    }
    py::handle operator()(const xval&) {
        throw std::runtime_error("might never append");
    }
};

template <> struct type_caster<xvar>: public type_caster_base<xvar> {
public:
    /**
     * This macro establishes the name 'inty' in
     * function signatures and declares a local variable
     * 'value' of type inty
     */
    // PYBIND11_TYPE_CASTER(XVariant, _("XVariant"));

    using base = type_caster_base<xvar>;

    /**
     * Conversion part 1 (Python->C++): convert a PyObject into a inty
     * instance or return false upon failure. The second argument
     * indicates whether implicit conversions should be applied.
     */
    bool load(handle src, bool convert) {
        if (base::load(src, convert)) {
            return true;
        } else if(py::isinstance<py::none>(src)) {
            value = new xvar(xnone{});
        } else if(py::isinstance<py::bool_>(src)) {
            value = new xvar(py::cast<bool>(src));
        } else if(py::isinstance<py::int_>(src)) {
            value = new xvar(py::cast<int>(src));
        } else if(py::isinstance<py::float_>(src)) {
            value = new xvar(py::cast<double>(src));
        } else if(py::isinstance<py::str>(src)) {
            value = new xvar(py::cast<std::string>(src));
        } else if(py::isinstance<py::dict>(src)) {
            xdict d;
            for(const auto& [k, v]: py::cast<py::dict>(src)) {
                d[py::cast<xvar>(k).get<xval>()] = py::cast<xvar>(v);
            }
            value = new xvar(d);
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
    static handle cast(xvar&& src, return_value_policy policy, handle parent) {
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

    py::class_<xvar>(m, "Variant");

    py::class_<xdict>(m, "Dict")
    .def(py::init<>())
    .def(py::init([](const py::dict& in) {
        auto* self = new xdict();
        for (const auto& [k, v]: in) {
            (*self)[py::cast<xvar>(k).get<xval>()] = py::cast<xvar>(v);
        }
        return self;
    }))
    .def("__setitem__", [](xdict& self, py::handle k, py::handle v) {
        self[py::cast<xvar>(k).get<xval>()] = py::cast<xvar>(v);
    })
    .def("__getitem__", [](xdict& self, py::handle k) {
        return self[std::forward<xvar>(py::cast<xvar&>(k)).get<xval>()];
    })
    .def_static("fromJson", [](const std::string& str){
        return xvar::FromJSON(str).get<xdict>();
    })
    .def_static("fromYaml", [](const std::string& str){
        return xvar::FromYAML(str).get<xdict>();
    })
    .def("__str__", &xdict::toString);

    py::class_<xlist>(m, "Array")
    .def(py::init<>())
    .def(py::init([](const py::tuple& in) {
        auto* self = new xlist();
        for (const auto& v: in) {
            (*self).push(py::cast<xvar>(v));
        }
        return self;
    }))
    .def(py::init([](const py::args& in) {
        auto* self = new xlist();
        for (const auto& v: in) {
            (*self).push(py::cast<xvar>(v));
        }
        return self;
    }))
    .def("__setitem__", [](xlist& self, py::int_ index, py::handle v) {
        self[py::cast<size_t>(index)] = py::cast<xvar>(v);
    })
    .def("__getitem__", [](xlist& self, py::int_ index) {
        return self[py::cast<size_t>(index)];
    })
    .def("__str__", &xlist::toString);

    py::class_<xfn>(m, "Function")
    .def("__call__", [](xfn& self) {
        return self();
    })
    .def("__call__", [](xfn& self, const py::args& args) {
        xlist a;
        for (const auto& v: args) {
            a.push(py::cast<xvar>(v));
        }
        return self.apply(std::move(a));
    });

    py::implicitly_convertible<py::dict, xdict>();
    py::implicitly_convertible<py::args, xlist>();
    py::implicitly_convertible<py::tuple, xlist>();
    py::implicitly_convertible<py::list, xlist>();
    py::implicitly_convertible<xlist, xvar>();
    py::implicitly_convertible<xdict, xvar>();
    py::implicitly_convertible<xfn, xvar>();
    py::implicitly_convertible<XObjectBase::ptr, xvar>();
    py::implicitly_convertible<XObjectBase, xvar>();

    py::class_<XObjectBase, std::shared_ptr<XObjectBase>>(m, "Object")
    .def("__getattr__", [](std::shared_ptr<XObjectBase>& self, const std::string& attr)
            -> std::variant<xvar, std::reference_wrapper<xdict>, xfn> {
        if (self->has_prop(attr)) {
            auto& prop = self->prop(attr);
            if (prop.is<xdict>())
                return std::ref(prop.get<xdict>());
            return prop.value();
        } else {
            return self->method(attr);
        }
    })
    .def("__setattr__", [](const std::shared_ptr<XObjectBase>& self, const std::string& attr, const XObjectBase::ptr& var){
        return self->prop(attr) = var;
    })
    .def("__setattr__", [](const std::shared_ptr<XObjectBase>& self, const std::string& attr, const xdict& var){
        return self->prop(attr) = var;
    })
    .def("__setattr__", [](const std::shared_ptr<XObjectBase>& self, const std::string& attr, const xlist& var){
        return self->prop(attr) = var;
    })
    .def("__setattr__", [](const std::shared_ptr<XObjectBase>& self, const std::string& attr, const xvar& var){
        return self->prop(attr) = var;
    });

    m.def("load", &XLibrary::Load, "Loads an xdev library");
    m.def("new", &XClass::Create<XObjectBase>, "Instanciates a new xdev object");
}
