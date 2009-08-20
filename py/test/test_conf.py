#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Miville
# Copyright (C) 2008 Société des arts technologiques (SAT)
# http://www.sat.qc.ca
# All rights reserved.
#
# This file is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# Miville is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Miville.  If not, see <http://www.gnu.org/licenses/>.
"""
Tests for the conf module.
"""
from twisted.trial import unittest
from twisted.internet import defer
from miville.utils import conf
import tempfile
from twisted.python import failure

conf.DEFAULT_FILE_NAME = tempfile.mktemp()
# data is persistent in database between each test.

class TestError(Exception):
    """
    Error raised by a test case.
    """
    pass

class Test_01_Conf(unittest.TestCase):
    def setUp(self):
        self.client = conf.Client() # using the "default" profile
    def tearDown(self):
        pass
    def test_01_create_field(self):
        return self.client.field_add("/egg", type="int", desc="Egg")

    def test_02_validate_duplicate_fields(self):
        """
        This test makes sure the Deferred triggers a Failure.
        It was not easy to figure out, so please copy and paste.
        """
        def _callback(result, test_case):
            return failure.Failure(TestError("Calling field_add() should have raised a failure."))
        def _errback(result, test_case):
            if isinstance(result.value, TestError):
                return result # a failure
            else:
                return True # instead of return result
        deferred = self.client.field_add("/egg", type="int", desc="Egg")
        deferred.addCallback(_callback, self)
        deferred.addErrback(_errback, self)
        return deferred
    
    def test_02_create_field(self):
        return self.client.field_add("/ham", type="int", desc="Ham")

    def test_03_add_entry(self):
        return self.client.entry_add("/egg", 2)

    def test_04_add_entry(self):
        return self.client.entry_add("/ham", 2)

    def test_05_list_entries(self):
        def _check(result, test_case):
            result = result.value
            for key in ["/egg", "/ham"]:
                if not result.has_key(key):
                    test_case.fail("Key not found: %s" % (key))
            return result
        return self.client.entry_list().addCallback(_check, self)

    def test_06_entry_to_default(self):
        return self.client.entry_default("/ham")

    def test_07_duplicate_profile(self):
        return self.client.profile_duplicate("other") # using the "other" profile

    def test_08_add_entry(self):
        return self.client.entry_add("/egg", 3)

    def test_09_save(self):
        return self.client.file_save()

    def test_10_load(self):
        return self.client.file_load()

    def test_11_list_entries(self):
        def _check(result, test_case):
            result = result.value
            for key in ["/egg", "/ham"]:
                if not result.has_key(key):
                    test_case.fail("Entry not found, but should be there: %s" % (key))
            return result
        return self.client.entry_list().addCallback(_check, self)

    def test_12_new_profile(self):
        return self.client.profile_add("one more")

    def test_13_list_profiles(self):
        def _check(result, test_case):
            result = result.value
            for key in ["default", "other", "one more"]:
                if not result.has_key(key):
                    test_case.fail("Profile not found, but should be there: %s" % (key))
            return result
        return self.client.profile_list()

    def test_14_remove_field(self):
        return self.client.field_remove("/egg")

    def test_15_list_fields(self):
        def _check(result, test_case):
            result = result.value
            for there in ["/ham"]:
                if not result.has_key(there):
                    test_case.fail("Field not found, but should be there: %s" % (there))
            for not_there in ["/egg"]:
                if result.has_key(not_there):
                    test_case.fail("Field found, but should not be there: %s" % (not_there))
            return result
        return self.client.field_list().addCallback(_check, self)

    def test_16_remove_profile(self):
        return self.client.profile_remove("other")

    def test_17_trial(self):
        def _callback(result, test_case):
            pass #print(result)
            # test_case.fail()
        return defer.Deferred().addCallback(_callback, self).callback("It works")
    
