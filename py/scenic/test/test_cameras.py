#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
Unit tests for the camera listing.
"""
from twisted.trial import unittest
from scenic.devices import cameras

#TODO: this test could test more

class Test_Cameras(unittest.TestCase):
    def test_list_cameras(self):
        def _cb(result):
            #print result
            return result
        deferred = cameras.list_cameras()
        deferred.addCallback(_cb)
        return deferred
