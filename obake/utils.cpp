// Copyright 2019-2020 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the obake.py library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <algorithm>
#include <string>
#include <utility>

#include <mp++/extra/pybind11.hpp>

#include <obake/math/safe_cast.hpp>
#include <obake/symbols.hpp>

#include <pybind11/pybind11.h>

#include "utils.hpp"

namespace obake_py
{

namespace py = ::pybind11;

// Throw a Python exception of type "type" with associated
// error message "msg".
void py_throw(::PyObject *type, const char *msg)
{
    ::PyErr_SetString(type, msg);
    throw py::error_already_set();
}

// Convert an arbitrary python object
// into a symbol_set.
::obake::symbol_set py_object_to_obake_ss(const py::object &o)
{
    ::obake::symbol_set ss;
    const auto len_h = py::len_hint(o);

    if (py::isinstance<py::set>(o)) {
        // If the input object is a set, we already
        // know it is sorted. Hence, we can keep
        // on inserting at the end and obtain
        // O(n) performance.
        ss.reserve(::obake::safe_cast<decltype(ss.size())>(len_h));

        for (const auto &str : o.cast<py::set>()) {
            ss.insert(ss.end(), str.cast<::std::string>());
        }
    } else {
        // Otherwise, we will just attempt
        // iterating over o, converting
        // its elements to strings and do
        // a final sort.
        ::obake::symbol_set::sequence_type seq;
        seq.reserve(::obake::safe_cast<decltype(seq.size())>(len_h));

        for (const auto &str : o) {
            seq.push_back(str.cast<::std::string>());
        }

        ::std::sort(seq.begin(), seq.end());
        ss.adopt_sequence(::std::move(seq));
    }

    return ss;
}

// Convert a symbol set into a python list.
py::list obake_ss_to_py_list(const ::obake::symbol_set &ss)
{
    py::list retval;

    for (const auto &s : ss) {
        retval.append(s);
    }

    return retval;
}

// Get the string representation of an object.
::std::string str(const py::object &o)
{
    return py::cast<std::string>(py::str(o));
}

} // namespace obake_py
