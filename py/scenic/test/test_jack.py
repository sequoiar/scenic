#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
Unit tests for the jackd listing.
""" 
#FIXME: this must be run from the py directory. Like this:
# cd py/
# trial scenic/test/test_jack.py

from twisted.trial import unittest
from scenic.devices import jackd
import os

os.environ["PATH"] = os.environ["PATH"] + ":" + os.path.join(__file__, "..", "..", "..", "utils", "jack-info")

#TODO: this test could test more
VERBOSE = True

class Test_Jack(unittest.TestCase):
    def test_list_jackd(self):
        def _cb(result):
            if len(result) > 0:
                rate = result[0]["rate"]
                if rate not in [22050, 32000, 44100, 48000, 88200, 96000, 192000]:
                    self.fail("Sampling has an unknown value: " + str(rate))
            if VERBOSE:
                print result
            return result
        deferred = jackd.jackd_get_infos()
        deferred.addCallback(_cb)
        return deferred

