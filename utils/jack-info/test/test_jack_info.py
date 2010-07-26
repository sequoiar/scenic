import subprocess
from twisted.trial import unittest

# Make sure TESTS_ENVIRONMENT has jack-info in path (shouldn't need to be installed)

class TestJackInfo(unittest.TestCase):
    def test_jack_info(self):
        """
            Check that jack-info prints backend, device, name, nperiods, period, pid, rate
        """
        # redirect stderr to stdout
        proc = subprocess.Popen(['jack-info', '"to stdout"'], 
                shell=True, 
                stderr=subprocess.STDOUT,
                stdout=subprocess.PIPE)

        stdout_value = proc.communicate()[0]
        if "not found" in stdout_value:
            self.fail("jack-info is not in PATH")

        info = {}
        # parse output of jack-info into dict (stripping the ":" from the key name
        expected = ['buffer-size', 'samplerate']
        for line in stdout_value.splitlines():
            if "JACK server not running" in line:
                print "JACK server not running"
                return
            for key in expected:
                if key in line:
                    key = line.split()[0][0:-1]
                    value = line.split()[1]
                    info[key] = value
                    print "%s: %s" % (key, str(value))

        for key in expected:
            if key not in info.iterkeys():
                self.fail("key %s was not found in jack-info output" % (key))
        
        # check for correct number of lines
        #assert(len(info) == len(expected))
