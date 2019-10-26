#ifndef OBAKE_PY_TYPE_SYSTEM_HPP
#define OBAKE_PY_TYPE_SYSTEM_HPP

#include <cassert>
#include <memory>
#include <string>
#include <typeindex>
#include <typeinfo>

#include <obake/type_name.hpp>

#include <pybind11/pybind11.h>

namespace obake_py
{

namespace py = ::pybind11;

extern ::std::unique_ptr<py::module> types_submodule_ptr;

struct type_generator {
    py::object operator()() const;
    ::std::string repr() const;
    ::std::type_index m_t_idx;
};

// Instantiate a type generator for type T in the types
// submodule. If an attribute with the same name already
// exists, it will error out.
template <typename T>
inline void instantiate_type_generator(const ::std::string &name)
{
    assert(types_submodule_ptr);
    auto &ts = *types_submodule_ptr;

    if (py::hasattr(ts, name.c_str())) {
        ::PyErr_SetString(::PyExc_AttributeError,
                          ("error while trying to instantiate a type generator for the C++ type '"
                           + ::obake::type_name<T>() + "': an attribute called '" + name
                           + "' already exists in the types submodule")
                              .c_str());

        throw py::error_already_set();
    }

    ts.attr(name.c_str()) = type_generator{::std::type_index(typeid(T))};
}

} // namespace obake_py

#endif
