#!/usr/bin/env python
"""
Makes sure we can detect the presence of Gstreamer elements.
"""
from twisted.trial import unittest
from scenic import gstreamer

class Test_Find_Elements(unittest.TestCase):
    def test_non_existing(self):
        assert(gstreamer.is_gstreamer_element_found("i_dont_exist") is False)

    def test_existing(self):
        assert(gstreamer.is_gstreamer_element_found("fakesink"))

