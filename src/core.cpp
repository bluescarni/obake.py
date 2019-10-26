#include <iostream>
#include <memory>

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

#include "type_system.hpp"

namespace py = ::pybind11;
namespace obpy = ::obake_py;

PYBIND11_MODULE(core, m)
{
    // Init the pybind11 integration for this module.
    ::mppp_pybind11::init();

    m.doc() = "The obake core module";

    // Create the types submodule.
    obpy::types_submodule_ptr = ::std::make_unique<py::module>(m.def_submodule("types", "The types submodule"));

    // Expose the type generator class.
    py::class_<obpy::type_generator> tg_class(m, "_type_generator");
    tg_class.def("__repr__", &obpy::type_generator::repr);
    // tg_class.def("__call__", &obpy::type_generator::operator());

    obpy::instantiate_type_generator<double>("double");
    obpy::instantiate_type_generator<::mppp::integer<1>>("integer");
    obpy::instantiate_type_generator<::mppp::rational<1>>("rational");
#if defined(MPPP_WITH_MPFR)
    obpy::instantiate_type_generator<::mppp::real>("real");
#endif
#if defined(MPPP_WITH_QUADMATH)
    obpy::instantiate_type_generator<::mppp::real128>("real128");
#endif
    obpy::instantiate_type_generator<::obake::packed_monomial<unsigned long long>>("packed_monomial");
    obpy::instantiate_type_generator<::obake::d_packed_monomial<unsigned long long, 8>>("d_packed_monomial");

    py::module::import("atexit").attr("register")(py::cpp_function([]() {
#if !defined(NDEBUG)
        ::std::cout << "Cleaning up obake.py data." << ::std::endl;
#endif
        obpy::types_submodule_ptr.reset();
    }));
}