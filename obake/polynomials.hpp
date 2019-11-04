// Copyright 2019 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the obake.py library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OBAKE_PY_POLYNOMIALS_HPP
#define OBAKE_PY_POLYNOMIALS_HPP

#include <iterator>
#include <string>
#include <utility>

#include <boost/hana/for_each.hpp>
#include <boost/hana/tuple.hpp>

#include <mp++/config.hpp>
#include <mp++/integer.hpp>
#include <mp++/rational.hpp>

#if defined(MPPP_WITH_MPFR)

#include <mp++/real.hpp>

#endif

#if defined(MPPP_WITH_QUADMATH)

#include <mp++/real128.hpp>

#endif

#include <obake/byte_size.hpp>
#include <obake/math/degree.hpp>
#include <obake/math/diff.hpp>
#include <obake/math/evaluate.hpp>
#include <obake/math/integrate.hpp>
#include <obake/math/pow.hpp>
#include <obake/math/subs.hpp>
#include <obake/math/trim.hpp>
#include <obake/polynomials/d_packed_monomial.hpp>
#include <obake/polynomials/packed_monomial.hpp>
#include <obake/polynomials/polynomial.hpp>
#include <obake/series.hpp>
#include <obake/type_name.hpp>

#include <pybind11/operators.h>
#include <pybind11/pybind11.h>

#include "docstrings.hpp"
#include "type_system.hpp"
#include "utils.hpp"

namespace obake_py
{

namespace hana = ::boost::hana;
namespace py = ::pybind11;

// The monomial types that will be exposed.
inline constexpr auto poly_key_types
    = hana::tuple_t<::obake::packed_monomial<long long>, ::obake::d_packed_monomial<long long, 8>>;

// The coefficient types that will be exposed.
inline constexpr auto poly_cf_types = hana::tuple_t<double, ::mppp::integer<1>, ::mppp::rational<1>
#if defined(MPPP_WITH_QUADMATH)
                                                    ,
                                                    ::mppp::real128
#endif
#if defined(MPPP_WITH_MPFR)
                                                    ,
                                                    ::mppp::real
#endif
                                                    >;

// The types with which we want polynomials to interoperate.
inline constexpr auto poly_interop_types = poly_cf_types;

#if defined(__clang__)

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-assign-overloaded"

#endif

// Polynomial exposition function.
template <typename K, typename C>
inline void expose_polynomial(py::module &m, type_getter &tg)
{
    using p_type = ::obake::polynomial<K, C>;

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
    class_inst.def("__deepcopy__", &generic_deepcopy_wrapper<p_type>);

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
    class_inst.def(py::self * py::self);
    class_inst.def(py::self *= py::self);

    // Comparison vs self.
    class_inst.def(py::self == py::self);
    class_inst.def(py::self != py::self);

    // Substitution with self.
    m.def("_subs", [](const p_type &, const p_type &x, const py::dict &d) {
        return ::obake::subs(x, py_dict_to_obake_sm<p_type>(d));
    });

    // Interact with the interoperable types.
    hana::for_each(poly_interop_types, [&class_inst, &m](auto t) {
        using cur_t = typename decltype(t)::type;

        // Constructor.
        class_inst.def(py::init<const cur_t &>());

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

    // Constructors from polynomials with different coefficients
    // (but same key).
    hana::for_each(poly_cf_types, [&class_inst](auto t) {
        using cur_cf_t = typename decltype(t)::type;

        if constexpr (!::std::is_same_v<cur_cf_t, C>) {
            // NOTE: skip the case cur_cf_t == C (that would be
            // a copy constructor).
            class_inst.def(py::init<const ::obake::polynomial<K, cur_cf_t> &>());
        }
    });

    // Byte size.
    m.def("byte_size", [](const p_type &p) { return ::obake::byte_size(p); });

    // Degree.
    m.def("degree", [](const p_type &p) { return ::obake::degree(p); });
    m.def("p_degree",
          [](const p_type &p, const py::iterable &s) { return ::obake::p_degree(p, py_object_to_obake_ss(s)); });

    // Trim.
    m.def("trim", [](const p_type &p) { return ::obake::trim(p); });

    // Polyomials factory function.
    m.def("_make_polynomials", [](const p_type &, py::args args) {
        py::list retval;

        // Special case: no input arguments.
        if (py::len(args) == 0u) {
            return retval;
        }

        auto args_it = ::std::begin(args);
        const auto args_end = ::std::end(args);

        if (py::isinstance<py::iterable>(*args_it) && !py::isinstance<py::str>(*args_it)) {
            // If the first argument is a non-string iterable, then
            // we interpret this as the symbol set argument for make_polynomials().
            const auto ss = py_object_to_obake_ss(args_it->cast<py::object>());
            for (++args_it; args_it != args_end; ++args_it) {
                auto [tmp] = ::obake::make_polynomials<p_type>(ss, args_it->cast<::std::string>());
                retval.append(::std::move(tmp));
            }
        } else {
            // Otherwise, we use the make_polynomials() overload with all strings.
            for (; args_it != args_end; ++args_it) {
                auto [tmp] = ::obake::make_polynomials<p_type>(args_it->cast<::std::string>());
                retval.append(::std::move(tmp));
            }
        }

        return retval;
    });

    // Diff/integrate.
    m.def("diff", [](const p_type &x, const ::std::string &s) { return ::obake::diff(x, s); });
    m.def("integrate", [](const p_type &x, const ::std::string &s) { return ::obake::integrate(x, s); });

    // Add the current polynomial
    // type to the type getter.
    tg.add<K, C>(class_inst);
}

#if defined(__clang__)

#pragma clang diagnostic pop

#endif

void expose_polynomials(py::module &);

void expose_polynomials_double(py::module &, type_getter &);
void expose_polynomials_integer(py::module &, type_getter &);
void expose_polynomials_rational(py::module &, type_getter &);
void expose_polynomials_real128(py::module &, type_getter &);
void expose_polynomials_real(py::module &, type_getter &);

} // namespace obake_py

#endif
