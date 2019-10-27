// Copyright 2019 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the obake.py library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <typeindex>
#include <vector>

#if defined(__GNUC__) || (defined(__clang__) && !defined(_MSC_VER))

// GCC demangle. This is available also for clang, both with libstdc++ and libc++.
#include <cstdlib>
#include <cxxabi.h>

#endif

#include <boost/functional/hash.hpp>

#include <mp++/extra/pybind11.hpp>

#include <pybind11/pybind11.h>

#include "type_system.hpp"

namespace obake_py
{

namespace py = ::pybind11;

// Global variables initialisation.
::std::unique_ptr<py::module> types_submodule_ptr;
::std::size_t exposed_types_counter = 0;
et_map_t et_map;
ti_map_t ti_map;

namespace
{

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

// Implementation of the type_generator class.
::std::string type_generator::repr() const
{
    return "Type generator for the C++ type '" + demangle_from_typeid(m_t_idx.name()) + "'";
}

py::object type_generator::operator()() const
{
    const auto it = et_map.find(m_t_idx);

    if (it == et_map.end()) {
        ::PyErr_SetString(::PyExc_TypeError,
                          ("the type '" + demangle_from_typeid(m_t_idx.name()) + "' has not been registered").c_str());

        throw py::error_already_set();
    }

    return it->second;
}

// Implementation of the hasher for ti_map_t.
::std::size_t v_idx_hasher::operator()(const ::std::vector<::std::type_index> &v) const
{
    ::std::size_t retval = 0;

    for (const auto &t_idx : v) {
        ::boost::hash_combine(retval, ::std::hash<::std::type_index>{}(t_idx));
    }

    return retval;
}

namespace
{

// Small utility to convert a vector of type indices to a string representation, only for error reporting purposes.
::std::string v_t_idx_to_str(const ::std::vector<::std::type_index> &v_t_idx)
{
    ::std::string tv_name = "[";

    for (decltype(v_t_idx.size()) i = 0; i < v_t_idx.size(); ++i) {
        tv_name += demangle_from_typeid(v_t_idx[i].name());

        if (i != v_t_idx.size() - 1u) {
            tv_name += ", ";
        }
    }

    tv_name += ']';

    return tv_name;
}

} // namespace

type_generator type_generator_template::getitem_t(const py::tuple &t) const
{
    if (ti_map.find(m_name) == ti_map.end()) {
        ::PyErr_SetString(::PyExc_TypeError,
                          ("no instance of the C++ class template '" + m_name + "' has been registered").c_str());

        throw py::error_already_set();
    }

    // Convert the tuple of generators to a vector of type idx objects.
    ::std::vector<::std::type_index> v_t_idx;
    for (const auto &o : t) {
        v_t_idx.push_back(o.cast<type_generator>().m_t_idx);
    }

    const auto it1 = ti_map[m_name].find(v_t_idx);
    if (it1 == ti_map[m_name].end()) {
        ::PyErr_SetString(::PyExc_TypeError, ("no instance of the C++ class template '" + m_name
                                              + "' has been registered with arguments " + v_t_idx_to_str(v_t_idx))
                                                 .c_str());
        throw py::error_already_set();
    }

    return type_generator{it1->second};
}

type_generator type_generator_template::getitem_o(const py::object &o) const
{
    return getitem_t(py::make_tuple(o));
}

::std::string type_generator_template::repr() const
{
    return "Type generator template for the C++ class template '" + m_name + "'";
}

} // namespace obake_py
