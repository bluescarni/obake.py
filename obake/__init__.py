# -*- coding: utf-8 -*-

# Copyright 2019 Francesco Biscani (bluescarni@gmail.com)
#
# This file is part of the obake.py library.
#
# This Source Code Form is subject to the terms of the Mozilla
# Public License v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

# for python 2.0 compatibility
from __future__ import absolute_import as _ai

# Version setup.
from ._version import __version__
# We import the sub-modules into the root namespace
from .core import *
# And we explicitly import the test submodule
from . import test


def make_polynomials(t, *args):
    from .core import _make_polynomials

    if not isinstance(t, type):
        raise TypeError(
            "the input parameter 't' is a {}, but it must be a type instead".format(type(t)))

    return _make_polynomials(t(), *args)
