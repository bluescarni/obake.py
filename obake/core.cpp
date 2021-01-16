// Copyright 2019-2020 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the obake.py library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <cstdint>

#include <mp++/config.hpp>
#include <mp++/extra/pybind11.hpp>
#include <mp++/integer.hpp>
#include <mp++/rational.hpp>

#if defined(MPPP_WITH_MPFR)

#include <mp++/real.hpp>

#endif

#if defined(MPPP_WITH_QUADMATH)

#include <mp++/real128.hpp>

#endif

#include <obake/polynomials/d_packed_monomial.hpp>
#include <obake/polynomials/packed_monomial.hpp>

#include <pybind11/pybind11.h>

#include "polynomials.hpp"
#include "type_system.hpp"

namespace py = ::pybind11;
namespace obpy = ::obake_py;

PYBIND11_MODULE(core, m)
{
    // Init the pybind11 integration for this module.
    ::mppp_pybind11::init();

    m.doc() = "The core obake module";

    // Flag the presence of MPFR/quadmath.
    m.attr("with_mpfr") =
#if defined(MPPP_WITH_MPFR)
        true
#else
        false
#endif
        ;

    m.attr("with_quadmath") =
#if defined(MPPP_WITH_QUADMATH)
        true
#else
        false
#endif
        ;

    // Export the obake version.
    m.attr("_obake_cpp_version_major") = OBAKE_VERSION_MAJOR;
    m.attr("_obake_cpp_version_minor") = OBAKE_VERSION_MINOR;
    m.attr("_obake_cpp_version_patch") = OBAKE_VERSION_PATCH;

    // Create the types submodule.
    auto types_submodule = m.def_submodule("types", "The types submodule");

    // Expose the type tag class.
    py::class_<obpy::type_tag> ttag_class(m, "_type_tag");
    ttag_class.def("__repr__", &obpy::type_tag::repr);

    // Expose the type getter class.
    py::class_<obpy::type_getter> tg_class(m, "_type_getter");
    tg_class.def("__getitem__", &obpy::type_getter::getitem_t);
    tg_class.def("__getitem__", &obpy::type_getter::getitem_o);
    tg_class.def("__repr__", &obpy::type_getter::repr);

    // Instantiate the type tags.
    obpy::instantiate_type_tag<double>(types_submodule, "double");
    obpy::instantiate_type_tag<::mppp::integer<1>>(types_submodule, "integer");
    obpy::instantiate_type_tag<::mppp::rational<1>>(types_submodule, "rational");
#if defined(MPPP_WITH_QUADMATH)
    obpy::instantiate_type_tag<::mppp::real128>(types_submodule, "real128");
#endif
#if defined(MPPP_WITH_MPFR)
    obpy::instantiate_type_tag<::mppp::real>(types_submodule, "real");
#endif
    obpy::instantiate_type_tag<::obake::packed_monomial<
#if defined(OBAKE_PACKABLE_INT64)
        ::std::int64_t
#else
        ::std::int32_t
#endif
        >>(types_submodule, "packed_monomial");
    obpy::instantiate_type_tag<::obake::d_packed_monomial<
#if defined(OBAKE_PACKABLE_INT64)
        ::std::int64_t
#else
        ::std::int32_t
#endif
        ,
        8>>(types_submodule, "d_packed_monomial");

    // Expose the polynomials.
    obpy::expose_polynomials(m);
}
