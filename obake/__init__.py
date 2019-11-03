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


def _remove_hash():
    # Helper to remove the hashing method
    # from exposed series, which are mutable
    # objects hence non-hashable:
    # https://docs.python.org/3/reference/datamodel.html#object.__hash__
    # NOTE: not sure if we can do this
    # from pybind11?
    from . import core
    for s in dir(core):
        if s.startswith('_exposed_type_'):
            setattr(getattr(core, s), '__hash__', None)


_remove_hash()


def _check_subs_map(d):
    if not isinstance(d, dict):
        raise TypeError(
            "the substitution map must be a dictionary, but it is of type {} instead".format(type(d)))

    if len(d) == 0:
        raise ValueError(
            "the substitution map cannot have a size of zero".format(type(d)))

    ctype = None
    for k in d:
        if not isinstance(k, str):
            raise TypeError(
                "the keys in a substitution map must be strings, but a key of type {} was encountered instead".format(type(k)))

        tdk = type(d[k])
        if ctype is None:
            ctype = tdk
        elif ctype != tdk:
            raise TypeError(
                "the values in a substitution map must be all of the same type, but values of type {} and {} were encountered instead".format(
                    ctype, tdk))

    return ctype


def subs(x, d):
    from .core import _subs

    t = _check_subs_map(d)
    return _subs(t(), x, d)
