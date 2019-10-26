// Copyright 2019 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the obake.py library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <cstddef>
#include <memory>
#include <string>

#if defined(__GNUC__) || (defined(__clang__) && !defined(_MSC_VER))

// GCC demangle. This is available also for clang, both with libstdc++ and libc++.
#include <cstdlib>
#include <cxxabi.h>

#endif

#include <mp++/extra/pybind11.hpp>

#include <pybind11/pybind11.h>

#include "type_system.hpp"

namespace obake_py
{

namespace py = ::pybind11;

// Global variables initialisation.
::std::unique_ptr<py::module> types_submodule_ptr;
::std::size_t exposed_types_counter = 0;

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

::std::string type_generator::repr() const
{
    return "Type generator for the C++ type '" + demangle_from_typeid(m_t_idx.name()) + "'";
}

} // namespace obake_py
