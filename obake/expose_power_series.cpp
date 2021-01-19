// Copyright 2019-2020 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the obake.py library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <mp++/extra/pybind11.hpp>

#include <pybind11/pybind11.h>

#include "power_series.hpp"
#include "type_system.hpp"

namespace obake_py
{

namespace py = ::pybind11;

void expose_power_series(py::module &m)
{
    // Create the power series type getter.
    type_getter tg("p_series");

    // Invoke the exposition functions
    // for the various cf types.
    expose_power_series_double(m, tg);
    expose_power_series_rational(m, tg);
    expose_power_series_real128(m, tg);
    expose_power_series_real(m, tg);

    // Add the power series type getter to the
    // python module.
    m.attr("p_series") = tg;
}

} // namespace obake_py
