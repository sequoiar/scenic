from selenium import selenium
import time
import unittest
import re
import os
import commands

"""
Expects:
The invitation has been accepted
Streaming (Medium-Bandwidth 2-Channel raw)
Streaming (Medium-Bandwidth 2-Channel raw)
Stopped streaming

selenium setup.py script::
from distutils.core import setup

setup(
    name="selenium",
    version="1.0.1",
    py_modules=["selenium"],
    )
"""

print("""
Running distributed web tests.
This command must be ran from the same dir.
We assume that :
 - Miville is running on local host (alice) and remote host. (bob)
 - The remote host (bob) has a contact for alice with auto-answer enabled.
 - There are no contact named bob in the local addressbook.
 - Selenium server is running
 - The path to selenium.py in is PYTHONPATH : %s
 - The local miville is neither connected nor streaming to any remote agent.
""" % (os.getenv("PYTHONPATH")))


print("Deleting contact bob using telnet with netcat.")
actions = [
    "c -l",
    "c -s bob",
    "j =i", 
    "c -e"
    ]
for action in actions:
    comm = """echo %s | nc localhost 14444 -q 1 """ % (action)
    ret = commands.getoutput(comm)
    print(ret)

class test_selenium_streams(unittest.TestCase):
    def setUp(self):
        self.verificationErrors = []
        self.selenium = selenium("localhost", 4444, "*chrome", "http://localhost:8080/")
        self.selenium.start()
    
    def _expect(self, txt):
        """
        Checks in the addressbook status HTML element for a string.
        """
        client = self.selenium
        got = client.get_text("adb_status")
        if got.find(txt) == -1:
            self.fail("Expected '%s' but got '%s'." % (txt, got))
        else:
            pass #print("Got '%s' in '%s' as expected." % (txt, got))

    def test_test_selenium_streams(self):
        client = self.selenium
        client.open("http://localhost:8080")
        time.sleep(0.1)
        client.click("adb_add")
        time.sleep(0.1)
        client.type("adb_name", "bob")
        time.sleep(0.1)
        client.type("adb_address", "10.10.10.72")
        time.sleep(0.1)
        client.click("adb_edit")
        time.sleep(0.1)
        client.click("adb_join") # connect
        time.sleep(5.0)
        self._expect("The invitation has been accepted")
        client.select("strm_global_setts", "label=Medium-Bandwidth 2-Channel raw")
        time.sleep(0.1)
        client.click("//option[@value='18']")
        time.sleep(0.1)
        client.click("strm_start") # start streams
        time.sleep(10.0)
        self._expect("Streaming")
        client.click("strm_start") # stop streams
        time.sleep(5.0)
        self._expect("Stopped streaming")
        client.click("adb_join") # disconnect
        time.sleep(5.0)
        self._expect("Connection stopped")
        client.click("adb_remove")
        time.sleep(0.5)
    
    def tearDown(self):
        self.selenium.stop()
        self.assertEqual([], self.verificationErrors)

if __name__ == "__main__":
    unittest.main()
