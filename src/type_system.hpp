// Copyright 2019 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the obake.py library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OBAKE_PY_TYPE_SYSTEM_HPP
#define OBAKE_PY_TYPE_SYSTEM_HPP

#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <memory>
#include <string>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <utility>
#include <vector>

#include <obake/type_name.hpp>

#include <pybind11/pybind11.h>

namespace obake_py
{

namespace py = ::pybind11;

// The types submodule.
extern ::std::unique_ptr<py::module> types_submodule_ptr;

// Counter of exposed types, used for naming them.
extern ::std::size_t exposed_types_counter;

// Map of exposed types: it connects a C++ type
// (represented as a std::type_index) to a concrete
// python type (contained into a py::object.)
using et_map_t = ::std::unordered_map<::std::type_index, py::object>;
extern et_map_t et_map;

// Type generator structure. It establishes
// the connection between a C++ type (the m_t_idx member)
// and its exposed Python counterpart via the call
// operator, which will query the et_map archive.
// Type generators are called from Python to retrieve
// the exposed Python type corresponding to the C++
// type t_idx.
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

// Register into et_map a C++ type that was exposed to Python,
// recording the corresponding Python type.
// This will error out if the type has already been registered.
template <typename T, typename... Args>
inline void register_exposed_type(const py::class_<T, Args...> &c)
{
    ::std::type_index t_idx(typeid(T));

    if (et_map.find(t_idx) != et_map.end()) {
        ::PyErr_SetString(::PyExc_TypeError, ("the C++ type '" + ::obake::type_name<T>()
                                              + "' has already been "
                                                "registered in the type system")
                                                 .c_str());

        throw py::error_already_set();
    }

    et_map[t_idx] = c;
}

struct no_t_name {
};

// Machinery to establish a 1-to-1 mapping between
// a class template and a string. The string will be
// used as a surrogate identifier for the class template,
// as we don't have objects in the standard to represent at
// runtime the "type" of class templates. This needs to be
// specialised in order to be used, otherwise a compile-time
// error will be generated because the default
// implementation is not a string.
template <template <typename...> typename>
inline constexpr auto t_name = no_t_name{};

// Hasher for a vector of type indices.
struct v_idx_hasher {
    ::std::size_t operator()(const ::std::vector<::std::type_index> &) const;
};

// A dictionary that records template instances. The string
// is a surrogate for a class template (as we cannot
// extract a type index from a class template), whose various
// instances are memorized in terms of the types
// defining the instance and a type_index representing the
// instance itself.
// Example: the instances std::map<int,double> and
// std::map<std::string,float> would be encoded as following:
// {"map" : {[int,double] : std::map<int,double>,
//  [std::string,float] : std::map<std::string,float>}}
using ti_map_t
    = ::std::unordered_map<::std::string,
                           ::std::unordered_map<::std::vector<::std::type_index>, ::std::type_index, v_idx_hasher>>;
extern ti_map_t ti_map;

// Register a template instance into ti_map.
// The string identifying the class template is
// taken from the specialisation
// of t_name for TT.
template <template <typename...> typename TT, typename... Args>
inline void register_template_instance()
{
    static_assert(::std::is_same_v<decltype(t_name<TT>), const ::std::string>);
    const ::std::string &name = t_name<TT>;

    // Convert the variadic args to a vector of type indices.
    ::std::vector<::std::type_index> v_t_idx{::std::type_index(typeid(Args))...};

    // Create the type index for the concrete type.
    ::std::type_index tidx(typeid(TT<Args...>));

    // NOTE: the new key in ti_map, if needed, will be created by the first call
    // to ti_map[name].
    if (ti_map[name].find(v_t_idx) != ti_map[name].end()) {
        ::PyErr_SetString(
            ::PyExc_TypeError,
            ("the template instance '" + ::obake::type_name<TT<Args...>>() + "' has already been registered").c_str());

        throw py::error_already_set();
    }

    ti_map[name].emplace(::std::move(v_t_idx), ::std::move(tidx));
}

// The purpose of this structure is to go look into
// ti_map for a template instance and, if found,
// return a type generator corresponding to that instance.
// The template instance  will be constructed from:
// - the class template connected to the string m_name (via t_name),
// - one or more type generators passed in as arguments to the getitem() method, representing
//   the parameters of the template instance.
struct type_generator_template {
    type_generator getitem_t(const py::tuple &) const;
    type_generator getitem_o(const py::object &) const;
    ::std::string repr() const;

    ::std::string m_name;
};

// Instantiate a type generator template for the class
// template TT into the types submodule. If an attribute
// with the same name already exists, it will error out.
template <template <typename...> typename TT>
inline void instantiate_type_generator_template(const ::std::string &name)
{
    assert(types_submodule_ptr);
    auto &ts = *types_submodule_ptr;

    if (py::hasattr(ts, name.c_str())) {
        ::PyErr_SetString(::PyExc_AttributeError,
                          ("error while trying to instantiate a type generator for the C++ class template '"
                           + t_name<TT> + "': an attribute called '" + name + "' already exists in the types submodule")
                              .c_str());

        throw py::error_already_set();
    }

    ts.attr(name.c_str()) = type_generator_template{t_name<TT>};
}

} // namespace obake_py

#endif
