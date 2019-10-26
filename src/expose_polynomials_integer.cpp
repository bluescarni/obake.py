#include <boost/hana/for_each.hpp>

#include <mp++/extra/pybind11.hpp>
#include <mp++/integer.hpp>

#include <pybind11/pybind11.h>

#include "polynomials.hpp"

namespace obake_py
{

namespace py = ::pybind11;
namespace hana = ::boost::hana;

void expose_polynomials_integer(py::module &m)
{
    hana::for_each(poly_key_types,
                   [&m](auto t) { expose_polynomial<typename decltype(t)::type, ::mppp::integer<1>>(m); });
}

} // namespace obake_py
