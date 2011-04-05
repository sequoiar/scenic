import subprocess
import unittest

class TestMilhouse(unittest.TestCase):
    def test_milhouse_help(self):
        """
        Looks for an expected string in the output of milhouse --help
        """
        # Make sure TESTS_ENVIRONMENT has milhouse in path (shouldn't need to be installed), this is done in Makefile.am
        # redirect stderr to stdout
        proc = subprocess.Popen(['milhouse --help', '"to stdout"'], 
                shell=True, 
                stderr=subprocess.STDOUT,
                stdout=subprocess.PIPE)

        stdout_value = proc.communicate()[0]

        
        expected_string_in_output = "is a receiver"
        if stdout_value.find(expected_string_in_output) == -1:
            self.fail("Did not find expected string %s in milhouse --help output" % (expected_string_in_output))
              
