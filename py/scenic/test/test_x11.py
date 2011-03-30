#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
Unit tests for listing X11 displays.
"""
from twisted.trial import unittest
from scenic.devices import x11
import os

class Test_X11(unittest.TestCase):
    def test_list_displays(self):
        def _cb(result):
            if len(result) < 1:
                self.fail("There should be at least one X11 display listed.")
            return result
        deferred = x11.list_x11_displays(verbose=False)
        deferred.addCallback(_cb)
        return deferred

    def test_xvideo_is_present(self):
        return x11.xvideo_extension_is_present()

    if "DISPLAY" not in os.environ:
        test_xvideo_is_present.skip = "DISPLAY is not set, cannot run test"
