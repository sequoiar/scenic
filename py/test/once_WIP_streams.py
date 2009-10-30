#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
Starts two miville and tests the protocols/pinger module.
"""
import unittest
import time
from test import lib_clientserver as clientserver

class Test_Ping(unittest.TestCase):
    def setUp(self):
        clientserver.kill_all_zombie_processes()
        self.alice = clientserver.TelnetMivilleTester(port_offset=2, verbose=False, use_tmp_home=True, name='alice')
        self.alice.setup(self)
        self.bob = clientserver.TelnetMivilleTester(port_offset=3, verbose=False, use_tmp_home=True, name='bob')
        self.bob.setup(self)
        self.alice.sendline("c -l")
        self.alice.send_expect("c -a Bob 127.0.0.1 2225", "added")
        self.bob.sendline("c -l")
        self.bob.send_expect("c -a Alice 127.0.0.1 2224", "added")
        self.alice.send_expect("c -s Bob", "selected")
        self.alice.sendline("j -s")
        self.bob.expect_test("Do you accept")
        self.bob.sendline("Y")
        self.alice.expect_test('accepted', 'Connection not successful.')

    def test_01_start_stop_streams(self):
        self.alice.send_expect("z -s Bob", "Successfully")
        end = time.time() + 10.0
        while time.time() < end:
            self.alice.flush_output()
            self.bob.flush_output()
        self.alice.send_expect("z -i Bob", "Successfully")

    def tearDown(self):
        self.alice.sendline("c -e Bob")
        self.bob.sendline("c -e Alice")
        self.alice.sendline("quit")
        self.bob.sendline("quit")
        self.alice.kill_children()
        self.bob.kill_children()

