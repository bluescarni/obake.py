// Copyright 2019-2020 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the obake.py library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OBAKE_PY_UTILS_HPP
#define OBAKE_PY_UTILS_HPP

#include <algorithm>
#include <sstream>
#include <string>
#include <utility>

#include <boost/container/container_fwd.hpp>

#include <obake/math/safe_cast.hpp>
#include <obake/symbols.hpp>
#include <obake/tex_stream_insert.hpp>

#include <pybind11/pybind11.h>

namespace obake_py
{

namespace py = ::pybind11;

// Throw a Python exception.
[[noreturn]] void py_throw(::PyObject *, const char *);

// Convert a generic object into a symbol set.
::obake::symbol_set py_object_to_obake_ss(const py::object &);

// Convert a symbol set into a python list.
py::list obake_ss_to_py_list(const ::obake::symbol_set &);

// Convert a dictionary into a symbol map of type T.
template <typename T>
inline ::obake::symbol_map<T> py_dict_to_obake_sm(const py::dict &d)
{
    typename ::obake::symbol_map<T>::sequence_type seq;
    seq.reserve(::obake::safe_cast<decltype(seq.size())>(py::len_hint(d)));

    for (const auto &[k, v] : d) {
        seq.emplace_back(k.template cast<::std::string>(), v.template cast<T>());
    }

    ::std::sort(seq.begin(), seq.end(), [](const auto &p1, const auto &p2) { return p1.first < p2.first; });

    ::obake::symbol_map<T> retval;
    retval.adopt_sequence(::boost::container::ordered_unique_range_t{}, ::std::move(seq));

    return retval;
}

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

// Get the string representation of an object.
::std::string str(const py::object &);

} // namespace obake_py

#endif
