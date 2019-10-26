#include <mp++/extra/pybind11.hpp>

#include <pybind11/pybind11.h>

#include "polynomials.hpp"

namespace obake_py
{

namespace py = ::pybind11;

void expose_polynomials(py::module &m)
{
    expose_polynomials_double(m);
    expose_polynomials_integer(m);
    expose_polynomials_rational(m);
    expose_polynomials_real128(m);
    expose_polynomials_real(m);
}

} // namespace obake_py
