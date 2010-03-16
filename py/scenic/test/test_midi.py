#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
Unit tests for listing MIDI devices.
""" 
from twisted.trial import unittest
from scenic.devices import midi

#TODO: this test could test more

class Test_Midi(unittest.TestCase):
    def test_list_midi_devices(self):
        def _cb(result):
            print result
            return result
        deferred = midi.list_midi_devices()
        deferred.addCallback(_cb)
        return deferred
