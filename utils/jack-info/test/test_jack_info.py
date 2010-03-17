#import subprocess
from twisted.trial import unittest

class TestJackInfo(unittest.TestCase):
    def test_jack_info_help(self):
        pass
        # Make sure TESTS_ENVIRONMENT has dc-ctl in path (shouldn't need to be installed)
        # redirect stderr to stdout
       #proc = subprocess.Popen(['jack-info', '"to stdout"'], 
       #        shell=True, 
       #        stderr=subprocess.STDOUT,
       #        stdout=subprocess.PIPE)

       #stdout_value = proc.communicate()[0]
       #print "value is: \n"
       #print stdout_value
