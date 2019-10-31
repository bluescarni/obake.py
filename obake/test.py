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

def run_test_suite():
    """Run the full test suite.

    This function will raise an exception if at least one test fails.

    """

    suite = _ut.TestLoader().loadTestsFromTestCase(core_test_case)

    test_result = _ut.TextTestRunner(verbosity=2).run(suite)

    if len(test_result.failures) > 0 or len(test_result.errors) > 0:
        raise RuntimeError('One or more tests failed.')
