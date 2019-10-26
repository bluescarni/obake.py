#include <boost/hana/for_each.hpp>

#include <mp++/config.hpp>
#include <mp++/extra/pybind11.hpp>

#if defined(MPPP_WITH_MPFR)
#include <mp++/real.hpp>
#endif

#include <pybind11/pybind11.h>

#include "polynomials.hpp"

namespace obake_py
{

namespace py = ::pybind11;
namespace hana = ::boost::hana;

void expose_polynomials_real([[maybe_unused]] py::module &m)
{
#if defined(MPPP_WITH_MPFR)
    hana::for_each(poly_key_types, [&m](auto t) { expose_polynomial<typename decltype(t)::type, ::mppp::real>(m); });
#endif
}

} // namespace obake_py
