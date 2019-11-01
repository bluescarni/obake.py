// Copyright 2019 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the obake.py library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <string>

#include <mp++/extra/pybind11.hpp>

#include "docstrings.hpp"

namespace obake_py
{

::std::string symbol_set_docstring()
{
    return R"(Symbol set.

This read-only property returns a copy of the symbol set of this
series, represented as a list of strings.

)";
}

} // namespace obake_py
