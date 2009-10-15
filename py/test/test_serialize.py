#!/usr/bin/env python 
# -*- coding: utf-8 -*-
"""
test for miville.utils.serialize
"""
from twisted.trial import unittest
from miville.utils import serialize
import tempfile

#VERBOSE = True
VERBOSE = False
FILE_NAME = tempfile.mktemp()

class SubDummy(serialize.Serializable):
    def __init__(self):
        self.x = None

class DummyData(serialize.Serializable):
    def __init__(self):
        self.number = None
        self.text = None
        self.dic = None
        self.sub = None

class Test_01_Serializable(unittest.TestCase):
    def test_01_serialize(self):
        if VERBOSE:
            print("File name : %s" % (FILE_NAME))
        dummy = DummyData()
        dummy.number = 5
        dummy.text = "hello"
        dummy.dic = {'asdf':3}
        dummy.sub = SubDummy()
        dummy.sub.x = ["some", "list"]
        serialize.save(FILE_NAME, dummy)

    def test_02_unserialize(self):
        dummy = serialize.load(FILE_NAME)
        if not isinstance(dummy, DummyData):
            self.fail("object should be an instance of the expected class.")
        if dummy.text != "hello":
            self.fail("wrong attribute value for hello")
        if dummy.dic["asdf"] != 3:
            self.fail("wrong attribute value for dict[\"asdf\"]")
        if dummy.number != 5:
            self.fail("wrong attribute value for number")
        sub = dummy.sub
        if not isinstance(sub, SubDummy):
            self.fail("object sub should be an instance of the expected class.")
        if dummy.sub.x[1] != "list":
            self.fail("sub dummy opbject should contain a list.")

