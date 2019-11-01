# -*- coding: utf-8 -*-

# Copyright 2019 Francesco Biscani (bluescarni@gmail.com)
#
# This file is part of the obake.py library.
#
# This Source Code Form is subject to the terms of the Mozilla
# Public License v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

from __future__ import absolute_import as _ai

import unittest as _ut


class core_test_case(_ut.TestCase):
    """Test case for core obake.py functionality.

    """

    def runTest(self):
        pass


class polynomials_test_case(_ut.TestCase):
    """Test case for polynomials.

    """

    def __init__(self):
        from .core import with_mpfr, with_quadmath
        from .core import types

        _ut.TestCase.__init__(self)

        self.cf_types = [types.double, types.integer, types.rational]
        if with_mpfr:
            self.cf_types.append(types.real)
        if with_quadmath:
            self.cf_types.append(types.real128)

        self.key_types = [types.packed_monomial, types.d_packed_monomial]

    def runTest(self):
        self.run_basic_tests()
        self.run_arithmetic_tests()
        self.run_degree_tests()
        self.run_trim_tests()

    def run_basic_tests(self):
        from itertools import product
        from fractions import Fraction as F
        from copy import copy, deepcopy
        from . import polynomial, make_polynomials

        key_cf_list = list(product(self.key_types, self.cf_types))

        for t in key_cf_list:
            pt = polynomial[t[0], t[1]]

            # Default ctor.
            self.assertTrue(len(pt()) == 0)
            self.assertTrue(pt() == 0)
            self.assertTrue(pt().symbol_set == [])

            # Ctor from scalars.
            self.assertTrue(len(pt(42)) == 1)
            self.assertTrue(pt(42.) == 42.)
            self.assertTrue(pt(F(4, 2)) == 2)

            # cpp_name class property.
            self.assertFalse(pt.cpp_name == "")

            # repr.
            self.assertTrue("Key type" in repr(pt(42)))

            # Copy/deepcopy.
            self.assertEqual(pt(42), copy(pt(42)))
            self.assertEqual(pt(42), deepcopy(pt(42)))

            # Ctors from other poly types with same
            # key type.
            for t2 in key_cf_list:
                if t == t2 or t[0] != t[1]:
                    continue

                self.assertEqual(pt(42), pt(pt2(42)))

            # The make_polynomials() factory.
            self.assertTrue(len(make_polynomials(pt)) == 0)

            x, y, z = make_polynomials(pt, 'x', 'y', 'z')
            self.assertTrue(len(x) == 1)
            self.assertEqual(x.symbol_set, ['x'])
            self.assertTrue(len(y) == 1)
            self.assertEqual(y.symbol_set, ['y'])
            self.assertTrue(len(z) == 1)
            self.assertEqual(z.symbol_set, ['z'])
            self.assertTrue(len(x + y + z) == 3)

            x, y, z = make_polynomials(pt, ['x', 'y', 'z'], 'x', 'y', 'z')
            self.assertTrue(len(x) == 1)
            self.assertEqual(x.symbol_set, ['x', 'y', 'z'])
            self.assertTrue(len(y) == 1)
            self.assertEqual(y.symbol_set, ['x', 'y', 'z'])
            self.assertTrue(len(z) == 1)
            self.assertEqual(z.symbol_set, ['x', 'y', 'z'])
            self.assertTrue(len(x + y + z) == 3)

            # Try with the symbol set passed in as a Python set.
            x, y, z = make_polynomials(pt, set(['x', 'y', 'z']), 'x', 'y', 'z')
            self.assertTrue(len(x) == 1)
            self.assertEqual(x.symbol_set, ['x', 'y', 'z'])
            self.assertTrue(len(y) == 1)
            self.assertEqual(y.symbol_set, ['x', 'y', 'z'])
            self.assertTrue(len(z) == 1)
            self.assertEqual(z.symbol_set, ['x', 'y', 'z'])
            self.assertTrue(len(x + y + z) == 3)

            # Error handling.
            with self.assertRaises(TypeError) as cm:
                make_polynomials(3, set(['x', 'y', 'z']), 'x', 'y', 'z')
            err = cm.exception
            self.assertTrue(
                "the input parameter 't' is a {}, but it must be a type instead".format(type(3)))

    def run_arithmetic_tests(self):
        from itertools import product
        from copy import copy
        from . import polynomial, make_polynomials, types

        key_cf_list = list(product(self.key_types, self.cf_types))

        for t in key_cf_list:
            pt = polynomial[t[0], t[1]]

            x, y, z = make_polynomials(pt, 'x', 'y', 'z')

            # Arithmetics with self.
            self.assertEqual(+x, x)
            self.assertEqual(x+y, y+x)
            x2 = copy(x)
            x2 += y
            self.assertEqual(-x, x*-1)
            self.assertEqual(x2, y+x)
            self.assertEqual(x-y, -(y-x))
            y2 = copy(y)
            y2 -= x
            self.assertEqual(y2, y-x)
            self.assertEqual(x*x, x**2)
            z2 = copy(z)
            z2 *= z
            self.assertEqual(z2, z**2)

            # Arithmetics with some interoperable types.
            self.assertEqual(1+x, x+1)
            self.assertEqual(1+x, x+pt(1))
            x2 = copy(x)
            x2 += 1
            self.assertEqual(x2, x+1)

            self.assertEqual(1-x, -(x-1))
            self.assertEqual(1-x, -(x-pt(1)))
            x2 = copy(x)
            x2 -= 1
            self.assertEqual(x2, x-1)

            self.assertEqual(2*x, x*2)
            self.assertEqual(2*x, pt(2)*x)
            x2 = copy(x)
            x2 *= 2
            self.assertEqual(x2, x*2)

            self.assertEqual((2*x)/2, x)
            x2 = copy(x)
            x2 *= 2
            x2 /= 2
            self.assertEqual(x2, x)

            # Exponentiation.
            self.assertEqual((2*x)**3, 8*x*x*x)
            if t[1] == types.double:
                self.assertEqual(pt(2)**(1./3), 2**(1./3))

    def run_degree_tests(self):
        from itertools import product
        from . import polynomial, make_polynomials, degree, p_degree

        key_cf_list = list(product(self.key_types, self.cf_types))

        for t in key_cf_list:
            pt = polynomial[t[0], t[1]]

            x, y, z = make_polynomials(pt, 'x', 'y', 'z')

            self.assertEqual(degree(pt()), 0)
            self.assertEqual(degree(pt(3)), 0)
            self.assertEqual(degree(x), 1)
            self.assertEqual(degree(x**-1), -1)
            self.assertEqual(degree(x**-1*y), 0)
            self.assertEqual(degree(x*y*z), 3)
            self.assertEqual(degree((x+y+z)**10), 10)

            self.assertEqual(p_degree(pt(), []), 0)
            self.assertEqual(p_degree(pt(), ['x', 'y']), 0)
            self.assertEqual(p_degree(pt(3), ['x', 'y']), 0)
            self.assertEqual(p_degree(x, []), 0)
            self.assertEqual(p_degree(x, ['x']), 1)
            self.assertEqual(p_degree(x, ['x', 'y']), 1)
            self.assertEqual(p_degree(x, ['z']), 0)
            self.assertEqual(p_degree(x**-1, ['x', 'y']), -1)
            self.assertEqual(p_degree(x**-1, ['z']), 0)
            self.assertEqual(p_degree(x**-1*y, ['x']), -1)
            self.assertEqual(p_degree(x**-1*y, ['x', 'y']), 0)
            self.assertEqual(p_degree(x*y*z, ['x', 'y']), 2)
            self.assertEqual(p_degree(x*y*z, ['x', 'y', 'z']), 3)
            self.assertEqual(p_degree(x*y*z, []), 0)
            self.assertEqual(p_degree((x+y+z)**10, []), 0)
            self.assertEqual(p_degree((x+y+z)**10, ['x']), 10)
            self.assertEqual(p_degree((x+y+z)**10, ['y']), 10)
            self.assertEqual(p_degree((x+y+z)**10, ['z']), 10)

    def run_trim_tests(self):
        from itertools import product
        from . import polynomial, make_polynomials, trim

        key_cf_list = list(product(self.key_types, self.cf_types))

        for t in key_cf_list:
            pt = polynomial[t[0], t[1]]

            x, y, z = make_polynomials(pt, ['x', 'y', 'z'], 'x', 'y', 'z')
            self.assertEqual(x.symbol_set, ['x', 'y', 'z'])
            self.assertEqual(trim(x).symbol_set, ['x'])
            self.assertEqual(trim((x+y)**10).symbol_set, ['x', 'y'])
            self.assertEqual(trim((x+y+z)**10).symbol_set, ['x', 'y', 'z'])


def run_test_suite():
    """Run the full test suite.

    This function will raise an exception if at least one test fails.

    """

    suite = _ut.TestLoader().loadTestsFromTestCase(core_test_case)
    suite.addTest(polynomials_test_case())

    test_result = _ut.TextTestRunner(verbosity=2).run(suite)

    if len(test_result.failures) > 0 or len(test_result.errors) > 0:
        raise RuntimeError('One or more tests failed.')
