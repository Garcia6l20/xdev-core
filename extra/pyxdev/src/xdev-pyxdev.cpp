#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <spdlog/spdlog.h>

#include <xdev/xdev-library.hpp>
#include <xdev/xdev-xclass.hpp>

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
    py::handle operator()(const xdev::XDict& item) {
        py::dict d;
        for(const auto& [k, v]: item) {
            d[k.visit(xvariant_to_python{})] = v.visit(xvariant_to_python{});
        }
        d.inc_ref();
        return d.ptr();
    }
    py::handle operator()(const xdev::XArray&item) {
        py::list l;
        for (const auto& ii : item) {
            l.append(ii.visit(xvariant_to_python{}));
        }
        l.inc_ref();
        return l.ptr();
    }
    py::handle operator()(const xdev::XObjectBase::ptr&) {
        return PyUnicode_FromString(ni);
    }
    py::handle operator()(const xdev::XValue&) {
        throw std::runtime_error("might never append");
        return PyUnicode_FromString(ni);
    }
    py::handle operator()(const xdev::XNone&) {
        return py::none{};
    }
    py::handle operator()(const xdev::XFunction&) {
        throw std::runtime_error("functions not handled here");
    }
};

namespace pybind11::detail {
template <> struct type_caster<xdev::XVariant>: public type_caster_base<xdev::XVariant> {
public:
    /**
     * This macro establishes the name 'inty' in
     * function signatures and declares a local variable
     * 'value' of type inty
     */
    // PYBIND11_TYPE_CASTER(xdev::XVariant, _("xdev::XVariant"));

    using base = type_caster_base<xdev::XVariant>;

    /**
     * Conversion part 1 (Python->C++): convert a PyObject into a inty
     * instance or return false upon failure. The second argument
     * indicates whether implicit conversions should be applied.
     */
    bool load(handle src, bool convert) {
        if (base::load(src, convert)) {
        } else if(py::isinstance<py::none>(src)) {
            value = new xdev::XVariant(xdev::XNone{});
        } else if(py::isinstance<py::bool_>(src)) {
            value = new xdev::XVariant(py::cast<bool>(src));
        } else if(py::isinstance<py::int_>(src)) {
            value = new xdev::XVariant(py::cast<int>(src));
        } else if(py::isinstance<py::float_>(src)) {
            value = new xdev::XVariant(py::cast<double>(src));
        } else if(py::isinstance<py::str>(src)) {
            value = new xdev::XVariant(py::cast<std::string>(src));
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
    static handle cast(xdev::XVariant src, return_value_policy /* policy */, handle /* parent */) {
        return src.visit(xvariant_to_python{});
    }
};
}

PYBIND11_MODULE(pyxdev, m) {

    spdlog::set_level(spdlog::level::debug);

    m.doc() = "xdev python plugin"; // optional module docstring

    py::class_<xdev::XLibrary, std::shared_ptr<xdev::XLibrary>>(m, "Library")
      .def("path", [](const std::shared_ptr<xdev::XLibrary>& self) { return self->path().string(); } )
      .def("name", &xdev::XLibrary::name);

    py::class_<xdev::XVariant>(m, "Variant");

    py::class_<xdev::XObjectBase, std::shared_ptr<xdev::XObjectBase>>(m, "Object")
      .def("__getattr__", [](const std::shared_ptr<xdev::XObjectBase>& self, const std::string& attr){        
        return self->getProperty(attr);
      })
      .def("__setattr__", [](const std::shared_ptr<xdev::XObjectBase>& self, const std::string& attr, const xdev::XVariant& var){
        return self->setProperty(attr, var);
      });

    m.def("load", &xdev::XLibrary::Load, "Loads an xdev library");
    m.def("new", &xdev::XClass::Create<xdev::XObjectBase>, "Instanciates a new xdev object");
}
