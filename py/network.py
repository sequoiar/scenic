#!/usr/bin/env python 
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
import time

from twisted.internet import reactor
from twisted.internet import protocol
from twisted.internet import defer
from twisted.internet.error import ProcessExitedAlready
from twisted.python import failure
from twisted.protocols import basic

# App imports
from utils import log
from utils import commands

log = log.start('debug', 1, 0, 'network')

# -------------------- constants -----------------------------------
STATE_IDLE = 0
KIND_UNIDIRECTIONAL = 1
KIND_TRADEOFF = 2
KIND_DUALTEST_CLIENT = 3
KIND_DUALTEST_SERVER = 4
STATE_QUERIED_FOR_DUALTEST = 5
STATE_WAITING_REMOTE_ANSWER = 6
STATE_ANSWERED_OK = 7

# functions ------------------------------------------------------
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

class NetworkError(Exception):
    """
    Any error due to network testing (with iperf).
    """
    pass

class IperfServerProcessProtocol(protocol.ProcessProtocol):
    """
    Manages the process of the `iperf -s -u` command
    """
    def __init__(self):
        self.network_tester = None # which started this process
        self.verbose = False
        self.prefix = "iperf -s>"
        self.network_tester_warned = False
    
    def connectionMade(self):
        if self.verbose:
            print self.prefix, "process started"
    
    def outReceived(self, data):
        log.info("iperf server started")
        if self.verbose:
            for line in data.splitlines():
                print self.prefix, line

    def _warn_network_tester_that_i_died(self, was_intentional=False):
        if not self.network_tester_warned:
            self.network_tester_warned = True
            self.network_tester.on_iperf_server_process_done()

    def inConnectionLost(self):
        pass # print "inConnectionLost! stdin is closed! (we probably did it)"
        self._warn_network_tester_that_i_died()
    
    def errConnectionLost(self):
        # print "errConnectionLost! The child closed their stderr."
        self._warn_network_tester_that_i_died()

    def processEnded(self, status):
        print self.prefix, "Ended iperf server, status %d" % status.value.exitCode
        #print "STOP REACTOR"
        #reactor.stop()
        try:
            if status_object.value.exitCode == 1:
                pass # TODO
        except:
            pass
        try:
            print self.prefix, status.getErrorMessage()
        except:
            pass
        self._warn_network_tester_that_i_died()


# --------------- states : ------------
#STATE_IDLE = 0
#KIND_UNIDIRECTIONAL = 1
#KIND_TRADEOFF = 2
#KIND_DUALTEST_CLIENT = 3
#KIND_DUALTEST_SERVER = 4
#STATE_QUERIED_FOR_DUALTEST = 5
#STATE_WAITING_REMOTE_ANSWER = 6
#STATE_ANSWERED_OK = 7

class NetworkTester(object):
    """
    Network tester using `iperf`.
    
    The `iperf` command can be used either as a client or a server.
    It is the client that initiates the testing.
    """
    def __init__(self):
        # globals
        self.iperf_server = None
        self.iperf_client = None
        self.api = None
        self.state = STATE_IDLE #STOPPED
        self.iperf_server_is_running = True
        # ------ current stats and config
        self.current_remote_addr = None
        self.current_bandwidth = 10 # Mbits
        self.current_duration = 1 # seconds
        self.current_caller = None # instance
        self.current_stats_local = None # dict
        self.current_stats_remote = None # dict
        self.current_com_chan = None # com_chan
        self.current_kind = None
        self.current_latency = 0 # milliseconds
        self.current_ping_time = 0 # UNIX timestamp

        # -------- settings
        self.accept_timeout = 20 # timeout before auto rejecting dualtest query
        self.remote_results_timeout = 5 # how many extra seconds over the duration of a test to wait for remote stats before giving up.

    def kill_server_process(self, process_transport, sig=15, verify_timeout=1.0):
        """
        Kills a process started using reactor.spawnProcess
        Double checks after 0.1 second
        
        Used in this case to kill the iperf server.
        """
        process_transport = self.iperf_server
        try:
            process_transport.signalProcess(sig)
            self.iperf_server_is_running = False
        except ProcessExitedAlready, e:
            print "error : ProcessExitedAlready (may be ok)", e.message
            self.state = STATE_IDLE
        else:
            if sig == 15: # (first time)
                reactor.callLater(verify_timeout, self.kill_server_process, process_transport, 9)
            else: # sig = 9 (second time)
                process_transport.loseConnection()
                self.iperf_server_is_running = False
        #TODO: make sure we do not start again the process !!!!!!!!!!!

    def on_iperf_server_process_done(self, intentional=True):
        """
        Called when the iperf -s process is done.
        """
        self.iperf_server_is_running = False
        if not intentional:
            print "iperf server process died unintentionally. will start it again." # TODO: log
            reactor.callLater(1.0, self._start_iperf_server_process)

    def on_iperf_command_results(self, results, commands, extra_arg, caller):
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
                    try:
                        iperf_stats = _parse_iperf_output(stdout.splitlines())
                        if len(iperf_stats) == 0:
                            # TODO
                            raise NetworkError("iperf command gave unexpected results. Is remote iperf running ?")
                    except NetworkError, e:
                        self.notify_api(caller, 'error', e.message)
                    else:    
                        iperf_stats.update(extra_arg) # appends the infos to the results dict
                        self.state = STATE_IDLE 

                        self.notify_api(caller, 'info', "iperf stats for IP %s: %s" % (ip, str(iperf_stats)))
                        self.notify_api(caller, "network_test_done", iperf_stats)
                else:
                    self.notify_api(caller, "error", "network performance : Unknown error.") # TODO use error key

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

    def _on_dualtest_ok(self, caller, server_addr, bandwidth_megabits=1, duration=10):
        # TODO
        """
        Called when remote server has said it would be ok to start the iperf bidirectional test.
        """
        reactor.callLater(self.current_latency / 2.0, self._start_iperf_client)
        
    def _setup_com_chan(self):
        """
        Registers com chan callbacks
        """
        # TODO
        com = self.current_com_chan
        if self.current_kind == KIND_DUALTEST_CLIENT:
            pass
            # :*args: is a tuple of data, whose 0th element is the name of the remote procedure to call.
            # callRemote(self, *args):
        else: # SERVER
            pass
        


    def start_test(self, caller, server_addr, bandwidth_megabits=1, duration=10, kind=KIND_UNIDIRECTIONAL, com_chan=None): # start_client
        """
        Starts `iperf -c localhost -y c -u -t 10 -b 1M`
       
        Actually, the process is started in self._start_iperf_client()
        duration : int
        bandwidth_megabits : int
        server_addr: str
        """
        if self.state == STATE_IDLE:
            self.current_remote_addr = server_addr
            self.current_bandwidth = bandwidth_megabits # Mbits
            self.current_duration = duration # seconds
            self.current_caller = caller # instance
            self.current_com_chan = com_chan # only neededi for dualtests
            self.current_stats_local = None # dict
            self.current_stats_remote = None # dict
            self.current_kind = kind
            
            if kind in (KIND_UNIDIRECTIONAL, KIND_TRADEOFF):
                self._start_iperf_client()
            else:
                # KIND_DUALTEST
                # TODO
                pass
        else:
            self.notify_api(caller, "error", "Network test is already happening.")

    def _start_iperf_client(self):
        """
        this is where we actually start the iperf process as a client
        """
        command = ["iperf", 
            "-c", self.current_remote_addr, # ip
            "-t", "%d" % self.current_duration, # duration
            "-y", "c", # CSV output
            "-u", # UDP
            "-b", "%dM" % self.current_bandwidth # bandwidth (megabits)
        ] 
        if self.current_kind == KIND_TRADEOFF:
            command.append("-r") # tradeoff.
            # all other tests 
        #extra_arg = None
        extra_arg = {
            'ip': self.current_remote_addr, #server_addr, 
            'bandwidth': self.current_bandwidth, # bandwidth_megabits, 
            'duration': self.current_duration, 
            'kind': self.current_kind # NETPERF_TEST_UNIDIRECTIONAL # TODO
        }
        callback = self.on_iperf_command_results
        try:
            commands.single_command_start(command, callback, extra_arg, self.current_caller)
        except CommandNotFoundError, e: # TODO: 
            print "error: ", e.message
        else:
           self.state = self.current_kind # set the state to the current kind

    def _start_iperf_server_process(self):
        """
        Starts `iperf -s -u`
        """
        if self.state == STATE_IDLE: 
            proto = IperfServerProcessProtocol()
            proto.network_tester = self
            try:
                self.iperf_server = reactor.spawnProcess(proto,\
                        "/usr/bin/iperf", ["/usr/bin/iperf", "-s", "-u"],\
                        os.environ)
            except OSError, e:
                print "Error starting process: ", e 
            else:
                self.iperf_server_is_running = True
    
# functions ---------------------------------------------
def start(subject):
    """
    Initial setup of the whole module for miville's use.
    
    Notifies with 'error' key  if `iperf` command not found. (CommandNotFoundError)
    Returns a NetworkTester instance with the server started.
    """
    try:
        executable = commands.find_command("iperf", 
            "`iperf` command not found. Please see See https://svn.sat.qc.ca/trac/miville/wiki/NetworkTesting for installation instructions.")
    except CommandNotFoundError, e:
        if subject is not None:
            subject.notify(subject, e.message, "error")
        print e.message
    tester = NetworkTester()
    tester.api = subject
    tester._start_iperf_server_process()
    
    #client = NetworkTester()
    #client.api = subject
    return tester
    #return {'server':server, 'client':client}

    #    txt = """
    #    20090126182857,10.10.10.145,35875,10.10.10.65,5001,3,0.0-10.0,1252440,999997
    #    20090126182857,10.10.10.65,5001,10.10.10.145,35875,3,0.0-10.0,1252440,999970,0.008,0,852,0.000,0
    #    """
    #    
    #    lines = txt.strip().splitlines()
    #    pprint.pprint(_parse_iperf_output(lines))

