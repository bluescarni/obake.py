// Copyright 2019 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the obake.py library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <cstddef>
#include <functional>
#include <string>
#include <typeindex>
#include <utility>
#include <vector>

#if defined(__GNUC__) || (defined(__clang__) && !defined(_MSC_VER))

// GCC demangle. This is available also for clang, both with libstdc++ and libc++.
#include <cstdlib>
#include <cxxabi.h>
#include <memory>

#endif

#include <boost/functional/hash.hpp>

#include <mp++/extra/pybind11.hpp>

#include <pybind11/pybind11.h>

#include "type_system.hpp"
#include "utils.hpp"

namespace obake_py
{

namespace py = ::pybind11;

::std::size_t exposed_types_counter = 0;

namespace
{

// Wrapper to demangle a name fetched from typeid.
::std::string demangle_from_typeid(const char *s)
{
#if defined(__GNUC__) || (defined(__clang__) && !defined(_MSC_VER))
    // NOTE: wrap std::free() in a local lambda, so we avoid
    // potential ambiguities when taking the address of std::free().
    // See:
    // https://stackoverflow.com/questions/27440953/stdunique-ptr-for-c-functions-that-need-free
    auto deleter = [](void *ptr) { ::std::free(ptr); };

    // NOTE: abi::__cxa_demangle will return a pointer allocated by std::malloc, which we will delete via std::free().
    ::std::unique_ptr<char, decltype(deleter)> res{::abi::__cxa_demangle(s, nullptr, nullptr, nullptr), deleter};

    // NOTE: return the original string if demangling fails.
    return res ? ::std::string(res.get()) : ::std::string(s);
#else
    // If no demangling is available, just return the mangled name.
    // NOTE: MSVC already returns the demangled name from typeid.
    return ::std::string(s);
#endif
}

} // namespace

// Implementation of the type_tag class.
::std::string type_tag::repr() const
{
    return "Type tag for the C++ type '" + demangle_from_typeid(m_t_idx.name()) + "'";
}

bool operator==(const type_tag &t1, const type_tag &t2)
{
    return t1.m_t_idx == t2.m_t_idx;
}

// Implementation of the type_getter class.
::std::size_t type_getter::vtt_hasher::operator()(const ::std::vector<type_tag> &v) const
{
    ::std::size_t retval = 0;

    for (const auto &ttag : v) {
        ::boost::hash_combine(retval, ::std::hash<::std::type_index>{}(ttag.m_t_idx));
    }

    return retval;
}

type_getter::type_getter(const char *name) : m_name(name) {}

::std::string type_getter::repr() const
{
    return "Type getter for the C++ class template '" + m_name + "'";
}

namespace
{

// Small utility to convert a vector of type tags
// to a string representation, only for error
// reporting purposes.
::std::string v_ttag_to_str(const ::std::vector<type_tag> &v)
{
    ::std::string tv_name = "[";

    for (decltype(v.size()) i = 0; i < v.size(); ++i) {
        tv_name += demangle_from_typeid(v[i].m_t_idx.name());

        if (i != v.size() - 1u) {
            tv_name += ", ";
        }
    }

    tv_name += ']';

    return tv_name;
}

} // namespace

void type_getter::add_impl(::std::vector<type_tag> &&v, const py::object &o)
{
    if (m_map.find(v) != m_map.end()) {
        py_throw(::PyExc_TypeError, ("an instance of the C++ class template '" + m_name
                                     + "' has already been registered with arguments " + v_ttag_to_str(v))
                                        .c_str());
    }

    m_map.emplace(::std::move(v), o);
}

// The two overloads for the [] operator.
py::object type_getter::getitem_t(const py::tuple &t) const
{
    // Convert t to a vector of type tags.
    ::std::vector<type_tag> v;
    for (const auto &o : t) {
        v.push_back(o.cast<type_tag>());
    }

    const auto it = m_map.find(v);
    if (it == m_map.end()) {
        py_throw(::PyExc_TypeError, ("no instance of the C++ class template '" + m_name
                                     + "' has been registered with arguments " + v_ttag_to_str(v))
                                        .c_str());
    }

    return it->second;
}

py::object type_getter::getitem_o(const py::object &o) const
{
    return getitem_t(py::make_tuple(o));
}

} // namespace obake_py
