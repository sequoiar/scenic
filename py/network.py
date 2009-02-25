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
import warnings 

from twisted.internet import reactor
from twisted.internet import protocol
from twisted.internet import defer
from twisted.internet.error import ProcessExitedAlready
from twisted.python import failure
from twisted.protocols import basic
try:
    from twisted.internet.error import PotentialZombieWarning
    warnings.simplefilter("ignore", PotentialZombieWarning)
    # this is just to be able to use imiville without an error message
    # Emitted when IReactorProcess.spawnProcess is called in a way which may result in termination of the created child process not being reported.  
except ImportError:
    pass # needs twisted version higher than the one shipped with ubuntu 8.04
# App imports
from utils import log
from utils import commands
import connectors
from errors import CommandNotFoundError
log = log.start('debug', 1, 0, 'network')

# -------------------- constants -----------------------------------
STATE_IDLE = 0
KIND_UNIDIRECTIONAL = 1
KIND_TRADEOFF = 2
KIND_DUALTEST = 3 # CLIENT
# KIND_DUALTEST_CLIENT = 3
KIND_DUALTEST_SERVER = 4
STATE_QUERIED = 5
STATE_WAITING_REMOTE_ANSWER = 6
STATE_ANSWERED_OK = 7

# functions ------------------------------------------------------
def _parse_iperf_output(lines):
    """
    Parses the output of iperf -c <host> -u

    Might raise NetworkError
    """
    #  iperf results: 20090225111131,10.10.10.65,35428,10.10.10.68,5001,3,0.0-10.0,64425739815,51440124145    ??????
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

# from wajig source iperf: src/ReportCSV.c :
# // UDP Reporting
#        printf( reportCSV_bw_jitter_loss_format,
#                timestamp,
#                (stats->reserved_delay == NULL ? ",,," : stats->reserved_delay),
#                stats->transferID,
#                stats->startTime,
#                stats->endTime,
#                stats->TotalLen,
#                speed,
#                stats->jitter*1000.0,
#                stats->cntError,
#                stats->cntDatagrams,
#                (100.0 * stats->cntError) / stats->cntDatagrams, stats->cntOutofOrder );
#

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
        log.info("Started the iperf server process.")
    
    def outReceived(self, data):
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
        #print self.prefix, "Ended iperf server, status %d" % status.value.exitCode
        log.info("Success ! The iperf server process is done. exit code : %r %s" % (status.value.exitCode, status.getErrorMessage()))
        #print "STOP REACTOR"
        #reactor.stop()
        try:
            if status_object.value.exitCode == 1:
                pass # TODO
        except:
            pass
        self._warn_network_tester_that_i_died()

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
        self.local_addr = None
        self.current_bandwidth = 10 # Mbits
        self.current_duration = 1 # seconds
        self.current_caller = None # instance
        self.current_stats_local = None # dict
        self.current_stats_remote = None # dict
        self.current_com_chan = None # com_chan
        self.current_kind = None
        self.current_latency = 0 # milliseconds
        self.current_ping_started_time  = 0 # UNIX timestamp
        
        # -------- settings
        self.accept_timeout = 20 # timeout before auto rejecting dualtest query
        self.remote_results_timeout = 5 # how many extra seconds over the duration of a test to wait for remote stats before giving up.

    def kill_server_process(self, process_transport, sig=15, verify_timeout=1.0):
        """
        Kills a process started using reactor.spawnProcess
        Double checks after 0.1 second
        
        Used in this case to kill the iperf server.
        """
        #process_transport = self.iperf_server
        try:
            process_transport.signalProcess(sig)
            self.iperf_server_is_running = False
        except AttributeError, e:
            log.error("Error in kill_server_process: %s" % e.message)
        except ProcessExitedAlready, e:
            log.debug("Successfuly killed the iperf server. (%s)" % e.message)
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
                        log.debug("iperf results: %s" % stdout)
                        if len(iperf_stats) == 0:
                            # TODO
                            raise NetworkError("iperf command gave empty results - or only 1 line. Is remote iperf server running ?")
                    except NetworkError, e:
                        self.notify_api(caller, 'error', e.message)
                        log.error(e.message)
                    else:    
                        # TODO : send updated dict to remote A
                        iperf_stats.update(extra_arg) # appends the infos to the dict we used for arguments to start it.
                        self.state = STATE_IDLE 
                        # SUCCESS !!!!!!!!!!!!!!!!!!!
                        #self.notify_api(caller, 'info', "iperf stats for IP %s: %s" % (ip, str(iperf_stats)))
                        self.notify_api(caller, "network_test_done", iperf_stats)
                        # XXX add an if
                        self._send_message("stop")
                else:
                    self.notify_api(caller, "error", "Network performance : Unknown error.") # TODO use error key

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
    def _get_remote_addr(self):
        """
        Returns the IP of the current remote contact.

        Useful for dualtest case.
        """
        # TODO : make more sturdy.
        ret = None
        contact = self.api.get_contact()
        try:
            ret = contact.address
        except:
            pass
        return ret
    #def _setup_com_chan(self):
    #    """
    #    Registers com chan callbacks
    #    """
    #    # TODO
    #    com = self.current_com_chan
    #    if self.current_kind == KIND_DUALTEST_CLIENT:
    #        pass
    #        # :*args: is a tuple of data, whose 0th element is the name of the remote procedure to call.
    #        # callRemote(self, *args):
    #    else: # SERVER
    #        pass
        


    def start_test(self, caller, server_addr, bandwidth_megabits=1, duration=10, kind=KIND_UNIDIRECTIONAL, com_chan=None): # start_client
        """
        Method called from the core API to try to start a network test.
        
        Starts `iperf -c localhost -y c -u -t 10 -b 1M`
        Actually, the process is started in self._start_iperf_client()
        
        :param duration: int
        :param bandwidth_megabits: int
        :param server_addr: str
        :param kind: int Must match one of the constants in this module. 
        """
        # TODO: remove com_chan
        if self.state != STATE_IDLE:
            self.notify_api(caller, "error", "A network test is already being done.")
        else:
            self.current_stats_local = None # dict
            self.current_stats_remote = None # dict
            
            self.current_remote_addr = server_addr
            self.current_bandwidth = bandwidth_megabits # Mbits
            self.current_duration = duration # seconds
            self.current_caller = caller # instance
            # XXX
            if com_chan is not None:
                log.debug("Using comm channel %s." % (com_chan))
            #    self.current_com_chan
                self.current_com_chan = com_chan # only neededi for dualtests
                # XXX We register again our callbacks !!
                log.debug("registering our callback once again.")
                self._register_my_callbacks_to_com_chan(com_chan)
            else:
                log.error("Invalid com_chan. It is None")
            self.current_kind = kind
            
            self.current_ping_started_time = self._get_time_now()
            self.state = STATE_WAITING_REMOTE_ANSWER 
            #if kind == KIND_DUALTEST:
            log.info("starting iperf server process")
            self._start_iperf_server_process()
            self._send_message("ping")

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
            # all other test kinds are using the default unidirectional iperf client.
        extra_arg = {
            'ip': self.current_remote_addr, #server_addr, 
            'bandwidth': self.current_bandwidth, # bandwidth_megabits, 
            'duration': self.current_duration, 
            'kind': self.current_kind 
        }
        # make sure all our agrs are string !!
        for i in range(len(command)):
            if not isinstance(command[i], str):
                command[i] = str(command[i])
        log.debug("Calling %r." % (command))
        callback = self.on_iperf_command_results
        try:
            commands.single_command_start(command, callback, extra_arg, self.current_caller)
        except CommandNotFoundError, e: 
            log.error("CommandNotFoundError %s" % e.message)
        else:
           self.state = self.current_kind # set the state to the current kind

    def _stop_iperf_server_process(self):
        """
        Wrapper to simplify calling kill_server_process
        """
        if self.iperf_server_is_running:
            self.kill_server_process(self.iperf_server)
        else:
            log.error("Called _stop_iperf_server_process while not running.")
    
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
                log.error("Error starting process: "+ e.message) 
            else:
                self.iperf_server_is_running = True

    def _get_time_now(self):
        """
        Returns current UNIT time in seconds.
        
        (it is a float)
        """
        return time.time() # * 1000.0 #  in ms

    def on_remote_message(self, key, args):
        """
        Called by the com_chan whenever a message is received from the remote contact through the com_chan with the "network_test" key.
        
        :param key: string defining the action/message
        :param *args: list of arguments
        
        The key and *args can be one of the following...
        
        In the example/sequence diagram below messages from A are sent by the one that initiates the test. 
        Messages from B represent the one that responds to the first one.

        * A: ping (used to measure latency)
        * B: pong
        * A: start <int kind> <dict {bandwidth:<int>, duration:<float>, latency:<float>}>
        * B: ok (it means the remote iperf server is started, and the remote client too if dualtest)
        * A: stop (asks the remote to stop its iperf server and give us the results)
        * B: results <int kind> <dict stats>
        """
        # TODO: we should check from which contact this message is from !!!!!!!!!!

        log.debug("received network_test %s:(%s)" % (key, str(args)))
        if key == "ping": # from A
            self._start_iperf_server_process()
            if self.state != STATE_IDLE:
                log.error("Received a ping while being busy doing some network test. (state = %d)" % self.state)
            else:
                log.debug("received ping")
                self._send_message("pong")
        
        elif key == "pong": # from B 
            if self.state != STATE_WAITING_REMOTE_ANSWER:
                log.error("Received pong while not being wainting for an answer. (state = %d)" % self.state)
            else:
                log.debug("received pong")
                # measure latency
                self.current_latency = self._get_time_now() - self.current_ping_started_time # seconds
                params = {
                    'duration':self.current_duration,
                    'bandwidth':self.current_bandwidth,
                    'latency':self.current_latency #, 
                    #'remote_addr':self.current_remote_addr
                }
                self._send_message("start", [self.current_kind, params])
                reactor.callLater(self.current_latency / 2.0, self._start_iperf_client)
        
        elif key == "start": # from A
            log.debug("received start")
            kind = args[0]
            params = args[1]
            self.current_kind = kind
            if kind == KIND_DUALTEST:
                # TODO
                self.current_kind = KIND_DUALTEST_SERVER
                log.debug("Starting iperf client, since it is a dualtest.")
                self.current_duration = params['duration']
                self.current_bandwidth = params['bandwidth']
                self.current_latency = params['latency']
                #self.current_remote_addr = params['remote_addr'] # IMPORTANT 
                self.current_remote_addr = self._get_remote_addr() # TODO XXX Make more sturdy
                reactor.callLater(self.current_latency / 2.0, self._start_iperf_client)
            self._send_message("ok")
        
        elif key == "ok": # from B
            log.debug("Received ok. Test should be happening.")
        
        elif key == "results": # from B
            kind = args[0]
            stats = args[1]
            log.debug("received remote iperf client results : %r" % stats)
            #if self.current_kind == KIND_DUALTEST:
            self._stop_iperf_server_process()
            self._send_message("stop")
        
        elif key == "stop": # from A
            log.debug("Received stop")
            self._stop_iperf_server_process()
        
        else:
            log.error("Unhandled com_chan message. %s" % key)
        
    def _send_message(self, key, args_list=[]):
        """
        Sends a message to the current remote contact through the com_chan with key "network_test".
        :param args: list
        """
        #args.insert(0, key)
        #args.insert(0, "network_test")
        #args.insert(0, "")
        try:
            self.current_com_chan.callRemote("network_test", key, args_list) # list with key as 0th element
        except AttributeError, e:
            log.error("Could not send message to remote. ComChan is None" + e.message)
        log.debug("Sent %s." % key)
    
    def _register_my_callbacks_to_com_chan(self, com_channel):
        """
        this is where we actually registers the callbacks
        """
        if com_channel is None:
            log.error("network.py: The provided com_channel is None !")
        else:
            # calls ComChannel.add(callback, key) 
            com_channel.add(self.on_remote_message, "network_test")
            log.info("Registered the com_chan callback with a new com_channel for the network_test instance.")

    def on_com_chan_connected(self, com_channel, role="client"):
        """
        Called from the Connector class in its com_chan_started_client or com_chan_started_server method.

        registers the com_chan callback for network_test
        
        :param role: string "client" or "server"
        We actually do not care if this miville is a com_chan client or server.
        """
        self.current_com_chan = com_channel # Maybe not necessary
        self._register_my_callbacks_to_com_chan(com_channel)

    def on_com_chan_disconnected(self, com_chan_instance):
        pass

# functions ---------------------------------------------
def start(subject):
    """
    Initial setup of the whole module for miville's use.
    
    Notifies with 'error' key  if `iperf` command not found. (CommandNotFoundError)
    Returns a NetworkTester instance with the server started.
    """
    try:
        executable = commands.find_command("iperf", 
            "`iperf` command not found. Please see https://svn.sat.qc.ca/trac/miville/wiki/NetworkTesting for installation instructions.")
    except CommandNotFoundError, e:
        if subject is not None:
            subject.notify(subject, e.message, "error")
        print e.message
    tester = NetworkTester()
    tester.api = subject
    
    connectors.register_callback("networktest_on_connect", tester.on_com_chan_connected, event="connect")
    connectors.register_callback("networktest_on_disconnect", tester.on_com_chan_disconnected, event="disconnect")
    return tester

    #    txt = """
    #    20090126182857,10.10.10.145,35875,10.10.10.65,5001,3,0.0-10.0,1252440,999997
    #    20090126182857,10.10.10.65,5001,10.10.10.145,35875,3,0.0-10.0,1252440,999970,0.008,0,852,0.000,0
    #    """
    #    
    #    lines = txt.strip().splitlines()
    #    pprint.pprint(_parse_iperf_output(lines))

