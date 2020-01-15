// Copyright 2019-2020 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the obake.py library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <boost/hana/for_each.hpp>

#include <mp++/extra/pybind11.hpp>

#include <pybind11/pybind11.h>

#include "polynomials.hpp"
#include "type_system.hpp"

namespace obake_py
{

namespace py = ::pybind11;
namespace hana = ::boost::hana;

void expose_polynomials_double(py::module &m, type_getter &tg)
{
    hana::for_each(poly_key_types, [&m, &tg](auto t) { expose_polynomial<typename decltype(t)::type, double>(m, tg); });
}

} // namespace obake_py
