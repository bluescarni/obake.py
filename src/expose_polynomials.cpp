// Copyright 2019 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the obake.py library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <mp++/extra/pybind11.hpp>

#include <obake/polynomials/polynomial.hpp>

#include <pybind11/pybind11.h>

#include "polynomials.hpp"
#include "type_system.hpp"

namespace obake_py
{

namespace py = ::pybind11;

void expose_polynomials(py::module &m)
{
    instantiate_type_generator_template<::obake::polynomial>("polynomial");

    expose_polynomials_double(m);
    expose_polynomials_integer(m);
    expose_polynomials_rational(m);
    expose_polynomials_real128(m);
    expose_polynomials_real(m);
}

} // namespace obake_py
