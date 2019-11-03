// Copyright 2019 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the obake.py library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OBAKE_PY_UTILS_HPP
#define OBAKE_PY_UTILS_HPP

#include <sstream>
#include <string>

#include <obake/symbols.hpp>
#include <obake/tex_stream_insert.hpp>

#include <pybind11/pybind11.h>

namespace obake_py
{

namespace py = ::pybind11;

::obake::symbol_set py_object_to_obake_ss(const py::object &);

py::list obake_ss_to_py_list(const ::obake::symbol_set &);

// repr() via std::ostringstream.
template <typename T>
inline ::std::string repr_ostr(const T &x)
{
    ::std::ostringstream oss;
    oss << x;
    return oss.str();
}

// latex repr.
template <typename T>
inline ::std::string repr_latex_ostr(const T &x)
{
    ::std::ostringstream oss;
    oss << '$';
    ::obake::tex_stream_insert(oss, x);
    oss << '$';
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
