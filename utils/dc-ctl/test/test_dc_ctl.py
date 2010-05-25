import subprocess
from twisted.trial import unittest

class TestDcCtl(unittest.TestCase):
    def test_dc_ctl_help(self):
        # Make sure TESTS_ENVIRONMENT has dc-ctl in path (shouldn't need to be installed)
        # redirect stderr to stdout
        proc = subprocess.Popen(['dc-ctl --help', '"to stdout"'], 
                shell=True, 
                stderr=subprocess.STDOUT,
                stdout=subprocess.PIPE)

        stdout_value = proc.communicate()[0]
        print "value is: \n"
        print stdout_value

        # either camera is not plugged in and we error out correctly, or it is and 
        # we set the brightness on it
        assert(stdout_value.find('print current settings'))
