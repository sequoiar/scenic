#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
Unit tests for our pure Python implementation of the signal-slot pattern.
"""

from twisted.trial import unittest
from scenic import sig
from twisted.internet import reactor
from twisted.internet import defer

class Test_01_Method_Signals(unittest.TestCase):
    def _slot(self):
        self.called = True
    def test_methods(self):
        self.called = False
        new_signal = sig.Signal()
        new_signal.connect(self._slot)
        new_signal()
        if not self.called:
            self.fail("Slot hasn't been called.")

    def test_args(self):
        pass
    test_args.skip = "to do"

    def test_kwargs(self):
        pass
    test_kwargs.skip = "to do"

class Test_02_Function_Signals(unittest.TestCase):
    def test_function(self):
        pass
    test_function.skip = "Our Pure-Python signals don't work with functions yet."
