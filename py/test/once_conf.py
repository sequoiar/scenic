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
from miville.utils import conf

# data is persistent in database between each test.

class Test_01_Conf(unittest.TestCase):
    def setUp(self):
        self.client = conf.Client() # using the "default" profile
    def tearDown(self):
        pass
    def test_01_create_field(self):
        return self.client.field_add("/egg", type="int", desc="Egg")
    def test_02_create_field(self):
        return self.client.field_add("/ham", type="int", desc="Ham")
    def test_03_add_entry(self):
        return self.client.entry_add("/egg", 2)
    def test_04_add_entry(self):
        return self.client.entry_add("/ham", 2)
    def test_05_list_entries(self):
        return self.client.entry_list()
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
        return self.client.entry_list()

