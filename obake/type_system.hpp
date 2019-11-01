// Copyright 2019 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the obake.py library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OBAKE_PY_TYPE_SYSTEM_HPP
#define OBAKE_PY_TYPE_SYSTEM_HPP

#include <cstddef>
#include <initializer_list>
#include <string>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <vector>

#include <pybind11/pybind11.h>

namespace obake_py
{

namespace py = ::pybind11;

// Counter of exposed types, used for naming them.
extern ::std::size_t exposed_types_counter;

// A small wrapper around type_index. Used to
// represent a C++ type at runtime.
struct type_tag {
    ::std::string repr() const;

    ::std::type_index m_t_idx;
};

bool operator==(const type_tag &, const type_tag &);

// Small helper to create a tag for type T in
// the module m.
template <typename T>
inline void instantiate_type_tag(py::module &m, const char *name)
{
    m.attr(name) = type_tag{::std::type_index(typeid(T))};
}

// This class is used to fetch, in python, a type
// which depends on other types (i.e., this is
// essentially a class template). Internally,
// it maps sequences of C++ types (represented as
// type_tag instances) to a python type object.
// Template instances must be registered on
// the C++ side via the add() member function.
// The corresponding python type can be fetched
// (from python) via the getitem_* overloads.
struct type_getter {
    struct vtt_hasher {
        ::std::size_t operator()(const ::std::vector<type_tag> &) const;
    };

    explicit type_getter(const char *);

    ::std::string repr() const;

    template <typename... Args>
    void add(const py::object &t)
    {
        add_impl(::std::vector<type_tag>{type_tag{::std::type_index(typeid(Args))}...}, t);
    }
    void add_impl(::std::vector<type_tag> &&, const py::object &);

    py::object getitem_t(const py::tuple &) const;
    py::object getitem_o(const py::object &) const;

    ::std::string m_name;
    ::std::unordered_map<::std::vector<type_tag>, py::object, vtt_hasher> m_map;
};

} // namespace obake_py

#endif
