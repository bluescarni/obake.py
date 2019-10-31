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


def run_test_suite():
    """Run the full test suite.

    This function will raise an exception if at least one test fails.

    """

    suite = _ut.TestLoader().loadTestsFromTestCase(core_test_case)
    suite.addTest(polynomials_test_case())

    test_result = _ut.TextTestRunner(verbosity=2).run(suite)

    if len(test_result.failures) > 0 or len(test_result.errors) > 0:
        raise RuntimeError('One or more tests failed.')
