#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <spdlog/spdlog.h>

#include <xdev/xdev-library.hpp>
#include <xdev/xdev-xclass.hpp>

using namespace xdev;

namespace py = pybind11;

struct xvariant_to_python {
    static const constexpr char* ni = "not implemented";
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
    py::handle operator()(const XDict& item) {
        return nullptr;
    }
    py::handle operator()(const XArray&item) {
        return nullptr;
    }
    py::handle operator()(const XObjectBase::ptr&) {
        return PyUnicode_FromString(ni);
    }
    py::handle operator()(const XValue&) {
        throw std::runtime_error("might never append");
    }
    py::handle operator()(const XNone&) {
        return py::none{};
    }
    py::handle operator()(const XFunction&) {
        throw std::runtime_error("functions not handled here");
    }
};

namespace pybind11::detail {
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
    static handle cast(XVariant src, return_value_policy policy, handle parent) {
//        handle h = type_caster_base<XVariant>::cast(src, policy, parent);
//        if (h)
//            return h;
        return src.visit(xvariant_to_python{});
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
        return self[py::cast<XVariant>(k)];
    })
    .def("__str__", &XDict::toString);
    //py::implicitly_convertible<py::dict, XDict>();


    py::class_<XArray>(m, "Array");

    py::class_<XObjectBase, std::shared_ptr<XObjectBase>>(m, "Object")
    .def("__getattr__", [](std::shared_ptr<XObjectBase>& self, const std::string& attr) -> std::variant<XVariant, std::reference_wrapper<XDict>> {
        auto& prop = self->prop(attr);
        if (prop.is<XDict>())
            return std::ref(prop.get<XDict>());
        return prop.value();
    })
    .def("__setattr__", [](const std::shared_ptr<XObjectBase>& self, const std::string& attr, const XDict& var){
        return self->prop(attr) = var;
    })
    .def("__setattr__", [](const std::shared_ptr<XObjectBase>& self, const std::string& attr, const XVariant& var){
        return self->prop(attr) = var;
    });

    m.def("load", &XLibrary::Load, "Loads an xdev library");
    m.def("new", &XClass::Create<XObjectBase>, "Instanciates a new xdev object");
}
