#ifndef OBAKE_PY_UTILS_HPP
#define OBAKE_PY_UTILS_HPP

#include <sstream>
#include <string>

#include <obake/symbols.hpp>

#include <pybind11/pybind11.h>

namespace obake_py
{

namespace py = ::pybind11;

::obake::symbol_set py_object_to_obake_ss(const py::object &);

// repr() via std::ostringstream.
template <typename T>
inline ::std::string repr_ostr(const T &x)
{
    ::std::ostringstream oss;
    oss << x;
    return oss.str();
}

// Generic copy wrappers.
template <typename T>
inline T generic_copy_wrapper(const T &x)
{
    return x;
}

template <typename T>
inline T generic_deepcopy_wrapper(const T &x, const py::dict &)
{
    return x;
}

} // namespace obake_py

#endif
