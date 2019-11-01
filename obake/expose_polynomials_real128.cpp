// Copyright 2019 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the obake.py library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <boost/hana/for_each.hpp>

#include <mp++/config.hpp>
#include <mp++/extra/pybind11.hpp>

#if defined(MPPP_WITH_QUADMATH)
#include <mp++/real128.hpp>
#endif

#include <pybind11/pybind11.h>

#include "polynomials.hpp"
#include "type_system.hpp"

namespace obake_py
{

namespace py = ::pybind11;
namespace hana = ::boost::hana;

void expose_polynomials_real128([[maybe_unused]] py::module &m, [[maybe_unused]] type_getter &tg)
{
#if defined(MPPP_WITH_QUADMATH)
    hana::for_each(poly_key_types,
                   [&m, &tg](auto t) { expose_polynomial<typename decltype(t)::type, ::mppp::real128>(m, tg); });
#endif
}

} // namespace obake_py
