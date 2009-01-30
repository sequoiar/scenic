## -*- coding: utf-8 -*-

# Sropulpof
# Copyright (C) 2008 Société des arts technologiques (SAT)
# http://www.sat.qc.ca
# All rights reserved.
#
# This file is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# Sropulpof is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Sropulpof.  If not, see <http:#www.gnu.org/licenses/>.

"""
Network performance tests manager using iperf.

See https://svn.sat.qc.ca/trac/miville/wiki/NetworkTesting
"""

import os
import sys

from twisted.internet import protocol
from twisted.internet import reactor
from twisted.internet.error import ProcessExitedAlready
from twisted.python import failure

# App imports
from utils import log
from utils import commands

log = log.start('debug', 1, 0, 'network')

NETPERF_TEST_UNIDIRECTIONAL = "unidirectional from local to remote"

# functions
def _parse_iperf_output(lines):
    """
    Parses the output of iperf -c <host> -u

    Might raise NetworkError
    """
    ret = {}
    keys = ['datetime', 'local ip', 'local port', 'remote ip', 'remote port', 'test index', 
           'interval', 'transfer', 'bandwidth', 
           'jitter', 'datagrams lost', 'total datagrams', 'loss',  'datagrams out of order']
    #print "TODO: parse, change state and notify"
    line_index = 0
    for line in lines:
        if line.find("WARNING: did not receive ack of last datagram after 10 tries.") != -1:
            # error !!
            raise NetworkError("Impossible to reach remote iperf server.")
        if line.startswith("2"): # date
            #print line
            if line_index == 0:
                line_index += 1
            else: # 2nd line is the only one we need
                words = line.split(",")
                word_index = 0
                for word in words:# interval, transfert, jitter, packet loss (%)
                    k = keys[word_index]
                    if k == 'latency':  # TODO: is it correct?
                        ret[k] = float(word) # %
                    elif k == 'bandwidth': 
                        ret[k] = int(word) # Kbits/sec
                    elif k == 'jitter': 
                        ret[k] = float(word) # ms
                    elif k == 'loss': 
                        ret[k] = float(word) # %
                    word_index += 1
    return ret

# local  : datetime, local ip, local port, remote ip, remote port, test index, interval (0-10) sec, transfer (bytes), bandwidth (Mbit/sec)
# remote  : datetime, local ip, local port, remote ip, remote port, test index, interval (0-10) sec, transfer (bytes), bandwidth (Mbit/sec), 
#            jitter (ms), datagrams lost, total datagrams, % loss, datagrams out of order.
#
# WARNING: did not receive ack of last datagram after 10 tries.
# if client tells this, unable to reach server.
#
# aalex@plank:~$ iperf -c localhost -u -y c -t 1 -b 1M
# 20090130142704,127.0.0.1,35745,127.0.0.1,5001,3,0.0-1.0,127890,999957
# 20090130142704,127.0.0.1,5001,127.0.0.1,35745,3,0.0-1.0,127890,1000047,0.005,0,87,0.000,0

# actually in the stdout
# 20090130143525,10.10.10.145,57442,10.10.10.65,5001,3,0.0-1.0,127890,999964
# 20090130143525,10.10.10.65,5001,10.10.10.145,57442,3,0.0-1.0,127890,1000013,0.010,0,87,0.000,0

class IperfServerProcessProtocol(protocol.ProcessProtocol):
    """
    Manages the process of the `iperf -s -u` command
    """
    def __init__(self):
        pass

    def connectionMade(self):
        pass # print "connectionMade"

    def outReceived(self, data):
        print data

    def errReceived(self, data):
        pass # print "errReceived! with %d bytes!" % len(data)

    def inConnectionLost(self):
        pass # print "inConnectionLost! stdin is closed! (we probably did it)"

    def outConnectionLost(self):
        pass # print "outConnectionLost! The child closed their stdout!"

    def errConnectionLost(self):
        pass # print "errConnectionLost! The child closed their stderr."

    def processEnded(self, status_object):
        print "Ended, status %d" % status_object.value.exitCode
        #print "STOP REACTOR"
        #reactor.stop()

class NetworkError(Exception):
    """
    Any error due to network testing (with iperf).
    """
    pass

class NetworkTester(object):
    """
    Network tester using `iperf`.
    
    The `iperf` command can be used either as a client or a server.
    It is the client that initiates the testing.
    """
    # constants
    STOPPED = 0
    SERVER_RUNNING = 1
    CLIENT_RUNNING = 2 
    
    def __init__(self):
        # globals
        self.iperf_server = None
        self.iperf_client = None
        self.api = None
        self.state = self.STOPPED

    def kill_process(self, process_transport, sig=15, verify_timeout=1.0):
        """
        Kills a process started using reactor.spawnProcess
        Double checks after 0.1 second
        
        Used in this case to kill the iperf server.
        """
        try:
            process_transport.signalProcess(sig)
        except Exception, e:
            print e
        if sig == 15:
            reactor.callLater(verify_timeout, self.kill_process, process_transport, 9)
        else:
            process_transport.loseConnection()
            self.state = self.STOPPED

    def on_client_results(self, results, commands, extra_arg, caller):
        """
        Called once a bunch of child processes are done.
        
        See utils.commands
        """
        for i in range(len(results)): # should have a len() of 1
            result = results[i]
            success, results_infos = result
            command = commands[i]
            
            if isinstance(results_infos, failure.Failure):
                print ">>>> FAILURE : ", results_infos.getErrorMessage()  # if there is an error, the programmer should fix it.
            else:
                stdout, stderr, signal_or_code = results_infos
                if success:
                    # print "stdout:", stdout
                    ip = extra_arg['ip']
                    iperf_stats = _parse_iperf_output(stdout.splitlines())
                    if len(iperf_stats) == 0:
                        # TODO
                        raise Exception("iperf command gave unexpected results.")
                    
                    iperf_stats.update(extra_arg)
                    self.state = self.STOPPED

                    self.notify_api(caller, 'info', "iperf stats ifor IP %s: %s" % (ip, str(iperf_stats)))
                    self.notify_api(caller, "network_test_done", iperf_stats)
                else:
                    self.notify_api(caller, "error", "Unknown error.") # TODO

    def notify_api(self, caller, key, val):
        """
        little wrapper for api.notify in order to easily debug this module.
        
        calls api.notify
        
        ways that this class has got called : 
        - api.network_test_start(self, caller, bandwidth=1, duration=1)
        """
        if self.api is None:
            if key == "stats":
                print "STATS: ", val
                print "(caller is %s)" % (caller)
            elif key == "error":
                print "ERROR !", val
        else:
            self.api.notify(caller, val, key) #key, res)

    def start_client(self, caller, server_addr, bandwidth_megabits=1, duration=10):
        """
        Starts `iperf -c localhost -y c -u -t 10 -b 1M`
        
        duration : int
        bandwidth_megabits : int
        server_addr: str
        """
        if self.state == self.STOPPED:
            command = ["iperf", "-c", server_addr, "-t", "%d" % (duration), "-y", "c", "-u", "-b", "%dM" % (bandwidth_megabits)] 
            extra_arg = {
                'ip': server_addr, 
                'ip': server_addr, 
                'bandwidth': bandwidth_megabits, 
                'duration': duration, 
                'kind': NETPERF_TEST_UNIDIRECTIONAL # TODO
            }
            callback = self.on_client_results
            try:
                commands.single_command_start(command, callback, extra_arg, caller)
            except Exception, e:
                print e
            else:
               self.state = self.CLIENT_RUNNING

    def start_server(self):
        """
        Starts `iperf -s -u`
        """
        if self.state == self.STOPPED:
            proto = IperfServerProcessProtocol()
            try:
                self.iperf_server = reactor.spawnProcess(proto, "/usr/bin/iperf", ["/usr/bin/iperf", "-s", "-u"], os.environ)
            except OSError, e:
                print "Error starting process: ", e 
            else:
                self.state = self.SERVER_RUNNING
                reactor.callLater(15.0, self.kill_process, self.iperf_server)

# functions ---------------------------------------------
def start(subject):
    """
    Initial setup of the whole module for miville's use.
    
    Raises CommandNotFoundError if `iperf` command not found.

    Returns a dict whose keys are 'client' and 'server'
    """
    # will raise CommandNotFoundError if not found:
    executable = commands.find_command("iperf", 
            "`iperf` command not found. Please see See https://svn.sat.qc.ca/trac/miville/wiki/NetworkTesting for installation instructions.")
    
    server = NetworkTester()
    server.api = subject
    server.start_server()
    
    client = NetworkTester()
    client.api = subject
    
    return {'server':server, 'client':client}

if __name__ == "__main__":
    # SIMPLE TEST
    def debug_meanwhile():
        # TODO : remove
        print "waiting..."
        reactor.callLater(0.5, debug_meanwhile)

    iperf = NetworkTester()
    if len(sys.argv) == 2:
        if sys.argv[1] == "client":
            print "starting client..."
            iperf.start_client("Dummy caller", "10.10.10.65", 1, 1) # "localhost"
        else:
            print "usage: <file name> client"
    else:
        iperf.start_server()
    reactor.callLater(0.5, debug_meanwhile)
    reactor.callLater(2.0, lambda: reactor.stop())
    reactor.run()
    #    txt = """
    #    20090126182857,10.10.10.145,35875,10.10.10.65,5001,3,0.0-10.0,1252440,999997
    #    20090126182857,10.10.10.65,5001,10.10.10.145,35875,3,0.0-10.0,1252440,999970,0.008,0,852,0.000,0
    #    """
    #    
    #    lines = txt.strip().splitlines()
    #    pprint.pprint(_parse_iperf_output(lines))


