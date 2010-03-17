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
        print 'value is: \n'
        info = {}
        # parse output of jack-info into dict (stripping the ":" from the key name
        for line in stdout_value.splitlines():
            key = line.split()[0][0:-1]
            value = line.split()[-1]
            info[key] = value
            print "%s: %s" % (key, str(value))

        expected = ['backend', 'device', 'name', 'nperiods', 'period', 'pid', 'rate']
        for key in expected:
            assert(key in info.iterkeys())
        
        # check for correct number of lines
        assert(len(info) == len(expected))
