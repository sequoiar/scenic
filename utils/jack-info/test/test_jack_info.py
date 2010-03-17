import subprocess
from twisted.trial import unittest

# Make sure TESTS_ENVIRONMENT has jack-info in path (shouldn't need to be installed)

class TestJackInfo(unittest.TestCase):
    def test_jack_info_help(self):
        """
            Check that jack-info prints backend, device, name, nperiods, period, pid, rate
        """
        # redirect stderr to stdout
        proc = subprocess.Popen(['jack-info', '"to stdout"'], 
                shell=True, 
                stderr=subprocess.STDOUT,
                stdout=subprocess.PIPE)

        stdout_value = proc.communicate()[0]
        print "value is: \n"
        print stdout_value
