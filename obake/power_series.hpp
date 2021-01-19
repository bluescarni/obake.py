// Copyright 2019-2020 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the obake.py library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OBAKE_PY_POWER_SERIES_HPP
#define OBAKE_PY_POWER_SERIES_HPP

#include <cstdint>
#include <string>

#include <boost/hana/concat.hpp>
#include <boost/hana/for_each.hpp>
#include <boost/hana/tuple.hpp>

#include <pybind11/operators.h>
#include <pybind11/pybind11.h>

#include <mp++/config.hpp>
#include <mp++/integer.hpp>
#include <mp++/rational.hpp>

#if defined(MPPP_WITH_MPFR)

#include <mp++/real.hpp>

#endif

#if defined(MPPP_WITH_QUADMATH)

#include <mp++/real128.hpp>

#endif

#include <obake/config.hpp>
#include <obake/math/evaluate.hpp>
#include <obake/math/pow.hpp>
#include <obake/math/subs.hpp>
#include <obake/polynomials/d_packed_monomial.hpp>
#include <obake/power_series/power_series.hpp>
#include <obake/type_name.hpp>

#include "docstrings.hpp"
#include "type_system.hpp"
#include "utils.hpp"

namespace obake_py
{

namespace hana = ::boost::hana;
namespace py = ::pybind11;

// The monomial types that will be exposed.
inline constexpr auto p_series_key_types = hana::tuple_t<::obake::d_packed_monomial<
#if defined(OBAKE_PACKABLE_INT64)
    ::std::int64_t
#else
    ::std::int32_t
#endif
    ,
    8>>;

// The coefficient types that will be exposed.
inline constexpr auto p_series_cf_types = hana::tuple_t<double, ::mppp::rational<1>
#if defined(MPPP_WITH_QUADMATH)
                                                        ,
                                                        ::mppp::real128
#endif
#if defined(MPPP_WITH_MPFR)
                                                        ,
                                                        ::mppp::real
#endif
                                                        >;

// The types with which we want power series to interoperate.
// NOTE: we add integer so that we can interoperate with Python ints.
inline constexpr auto p_series_interop_types = hana::concat(hana::tuple_t<::mppp::integer<1>>, p_series_cf_types);

#if defined(__clang__)

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-assign-overloaded"

#endif

// Power series exposition function.
template <typename K, typename C>
inline void expose_power_series(py::module &m, type_getter &tg)
{
    using p_type = ::obake::p_series<K, C>;
    using namespace py::literals;

    py::class_<p_type> class_inst(m, ("_exposed_type_" + ::std::to_string(exposed_types_counter++)).c_str());

    // Default constructor.
    class_inst.def(py::init<>());
    // Add a static readonly string to the class type
    // which represents the corresponding C++ type.
    class_inst.def_property_readonly_static("cpp_name", [](py::object) { return ::obake::type_name<p_type>(); });
    // Special methods.
    class_inst.def("__repr__", &repr_ostr<p_type>);
    class_inst.def("__len__", &p_type::size);
    class_inst.def("__copy__", &generic_copy_wrapper<p_type>);
    class_inst.def(
        "__deepcopy__", [](const p_type &p, py::dict) { return p; }, "memo"_a);

    // Latex repr.
    class_inst.def("_repr_latex_", &repr_latex_ostr<p_type>);

    // Table stats.
    class_inst.def("table_stats", &p_type::table_stats);

    // Symbol set getter.
    class_inst.def_property_readonly(
        "symbol_set", [](const p_type &p) { return obake_ss_to_py_list(p.get_symbol_set()); },
        symbol_set_docstring().c_str());

    // Arithmetics vs self.
    class_inst.def(+py::self);
    class_inst.def(py::self + py::self);
    class_inst.def(py::self += py::self);
    class_inst.def(-py::self);
    class_inst.def(py::self - py::self);
    class_inst.def(py::self -= py::self);
    // TODO
    // class_inst.def(py::self * py::self);
    // class_inst.def(py::self *= py::self);

    // Comparison vs self.
    class_inst.def(py::self == py::self);
    class_inst.def(py::self != py::self);

    // Substitution with self.
    // TODO
    // m.def("_subs", [](const p_type &, const p_type &x, const py::dict &d) {
    //     return ::obake::subs(x, py_dict_to_obake_sm<p_type>(d));
    // });

    // Interact with the interoperable types.
    hana::for_each(p_series_interop_types, [&class_inst, &m](auto t) {
        using cur_t = typename decltype(t)::type;

        // Constructor.
        class_inst.def(py::init<const cur_t &>());
        // Constructor with symbol set.
        class_inst.def(
            py::init([](const cur_t &c, const py::iterable &s) { return p_type(c, py_object_to_obake_ss(s)); }));

        // Arithmetics.
        class_inst.def(py::self + cur_t{});
        class_inst.def(cur_t{} + py::self);
        class_inst.def(py::self += cur_t{});

        class_inst.def(py::self - cur_t{});
        class_inst.def(cur_t{} - py::self);
        class_inst.def(py::self -= cur_t{});

        class_inst.def(py::self * cur_t{});
        class_inst.def(cur_t{} * py::self);
        class_inst.def(py::self *= cur_t{});

        class_inst.def(py::self / cur_t{});
        class_inst.def(py::self /= cur_t{});

        // Comparisons.
        class_inst.def(py::self == cur_t{});
        class_inst.def(cur_t{} == py::self);
        class_inst.def(py::self != cur_t{});
        class_inst.def(cur_t{} != py::self);

        // Exponentiation.
        class_inst.def("__pow__", [](const p_type &p, const cur_t &x) { return ::obake::pow(p, x); });

        // Subs.
        m.def("_subs", [](const cur_t &, const p_type &x, const py::dict &d) {
            return ::obake::subs(x, py_dict_to_obake_sm<cur_t>(d));
        });

        // Evaluate.
        m.def("_evaluate", [](const cur_t &, const p_type &x, const py::dict &d) {
            return ::obake::evaluate(x, py_dict_to_obake_sm<cur_t>(d));
        });
    });

    // Add the current power series
    // type to the type getter.
    tg.add<K, C>(class_inst);
}

#if defined(__clang__)

#pragma clang diagnostic pop

#endif

void expose_power_series(py::module &);

void expose_power_series_double(py::module &, type_getter &);
void expose_power_series_rational(py::module &, type_getter &);
void expose_power_series_real128(py::module &, type_getter &);
void expose_power_series_real(py::module &, type_getter &);

} // namespace obake_py

#endif
