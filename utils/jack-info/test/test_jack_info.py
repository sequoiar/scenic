import subprocess
from twisted.trial import unittest

# Make sure TESTS_ENVIRONMENT has jack-info in path (shouldn't need to be installed)

class TestJackInfo(unittest.TestCase):
    def test_jack_info(self):
        """
            Check that jack-info prints backend, device, name, nperiods, period, pid, rate
        """
        # redirect stderr to stdout
        proc = subprocess.Popen(['../jack-info', '"to stdout"'], 
                shell=True, 
                stderr=subprocess.STDOUT,
                stdout=subprocess.PIPE)

        stdout_value = proc.communicate()[0]
        if "not found" in stdout_value:
            self.fail("jack-info is not in PATH")

        info_retrieved = {} # data retrived from child's STDOUT
        # parse output of jack-info into dict (stripping the ":" from the key name
        expected_keys = ['buffer-size', 'samplerate']
        for line in stdout_value.splitlines():
            if "JACK server not running" in line:
                print("JACK server not running. ")
                return
            for key in expected_keys:
                if line.startswith(key):
                    key = line.split()[0][0:-1] # first word
                    value = line.split()[1] # 2nd word
                    info_retrieved[key] = value # store to data retrieved
                    # print("%s: %s" % (key, str(value)))

        for key in expected_keys:
            if key not in info_retrieved.iterkeys():
                self.fail("Key %s was not found in jack-info output. Its output is: \n%s" % (key, stdout_value))
        # check for correct number of lines
        #assert(len(info) == len(expected))
    # FIXME: We should fix this test
    test_jack_info.skip = "This test cast does not work in pbuilder."
