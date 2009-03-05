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

4 kinds of network tests and their commands :

 * from local to remote (KIND_UNIDIRECTIONAL)
    local:  'iperf -c 10.10.10.66 -t 1 -y c -u -b 1M'

 * from remote to local (KIND_REMOTETOLOCAL)
    remote: 'iperf -c 10.10.10.68 -t 1 -y c -u -b 1M'

 * bidirectional sequential (KIND_TRADEOFF)
    local: 'iperf -c 10.10.10.66 -t 1 -y c -u -b 1M -r'

 * bidirectional simetrical (KIND_DUALTEST)
    local:  'iperf -c 10.10.10.66 -t 1 -y c -u -b 1M'
    remote: 'iperf -c 10.10.10.68 -t 1 -y c -u -b 1M'
"""

import os
import sys
import time
import warnings 

from twisted.internet import reactor
from twisted.internet import protocol
from twisted.internet import defer
from twisted.internet.error import ProcessExitedAlready, AlreadyCancelled
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
log = log.start('info', 1, True, 'network') # LOG TO FILE = True
# CHANGE IT TO debug LEVEL TO GET MORE OUTPUT

# -------------------- constants -----------------------------------
STATE_IDLE = 0
KIND_UNIDIRECTIONAL = 1
KIND_REMOTETOLOCAL = 8
KIND_REMOTETOLOCAL_SERVER = 9
KIND_TRADEOFF = 2
KIND_DUALTEST = 3 # CLIENT
# KIND_DUALTEST_CLIENT = 3
KIND_DUALTEST_SERVER = 4
STATE_QUERIED = 5
STATE_WAITING_REMOTE_ANSWER = 6
STATE_ANSWERED_OK = 7

#---------------------- module variables ---------------------------
# IMPORTANT MODULE VARIABLES
# there can be only one network test at a time.
_is_currently_busy = False
# dict of NetworkTester instances. One for each contact to which we are connected.
# keys are unicode strings.
_testers = {}
_api = None
_TIMEOUT = 30.0 # timeout in seconds max to wait before calling _timeout.

"""
This dict explains the 2nd line of the output of `iperf -y c -u -c <host>`
"""
iperf_results_indices = {
    0:["datetime_YmdHis", "str"], # 0 str
    1:["remote_ip", "str"], # 1 str                         # XXX local is actually remote... since it is the local remote server... 
    2:["remote_port", "int"], # 2 int
    3:["local_ip", "str"], # 3 str
    4:["local_port", "int"], # 4 int
    5:["transfert_ID", "int"], # 5 int 
    6:["start_and_end_time", "str"], # 6 str #%.1f-%.1f
    # ------------------------ end of first line.
    #                          2nd line has more:
    7:["total_length", "int"], # 8 long int
    8:["speed", "int"], # 9 long int 
    9:["jitter", "float"], # 10 float 
    10:["count_errors", "int"], # 11 int
    11:["count_datagrams", "int"], # 12 int 
    12:["percent_errors", "float"], # 13 float
    13:["count_out_of_order", "int"] # 14 int
}

# functions ------------------------------------------------------
def _parse_iperf_output(lines):
    """
    Parses the output of iperf -c <host> -u

    Might raise NetworkError
    """
    ret = {}
#     keys = ['datetime', 'local ip', 'local port', 'remote ip', 'remote port', 'test index', 
#            'interval', 'transfer', 'bandwidth', 
#            'jitter', 'datagrams lost', 'total datagrams', 'loss',  'datagrams out of order']
    #print "TODO: parse, change state and notify"
    line_index = 0
    was_ok = False
    for line in lines:
        if line.find("WARNING: did not receive ack of last datagram after 10 tries.") != -1:
            # error !!
            raise NetworkError("Impossible to reach remote iperf server.")
        elif line.startswith("2"): # date
            #print line
            if line_index == 0:
                line_index += 1
            else: # 2nd line is the only one we need
                words = line.split(",")
                word_index = 0
                for str_value in words:
                    key_name = iperf_results_indices[word_index][0]
                    cast_type = iperf_results_indices[word_index][1]
                    to_exec = "tmp = %s(\"%s\")" % (cast_type, str_value)
                    try:
                        # print to_exec
                        exec(to_exec)
                    except ValueError:
                        log.error("Could not cast %s string to %s for index %s." % (str_value, cast_type, key_name))
                    ret[key_name] = tmp
                    word_index += 1
                was_ok = True
    if not was_ok:
        raise NetworkError("Could not get satisfying iperf output.")
    return ret
# XXX former algo : 
#                 for word in words:# interval, transfert, jitter, packet loss (%)
#                     k = keys[word_index]
#                     if k == 'latency':  # TODO: is it correct?
#                         ret[k] = float(word) # %
#                     elif k == 'bandwidth': 
#                         ret[k] = int(word) # Kbits/sec
#                     elif k == 'jitter': 
#                         ret[k] = float(word) # ms
#                     elif k == 'loss': 
#                         ret[k] = float(word) # %
#                     word_index += 1
# TODO : in the process to parse the iperf output better...
#------------------- SAT rules --------------------
# %s,%s,%d,%.1f-%.1f,%lld,%lld,%.3f,%d,%d,%.3f,%d
#20090226155534,
# 10.10.10.65,5001,    10.10.10.68,43627,
# 3, 0.0-10.0, 1252440, 999983, 0.006, 0,852, 0.000,0

# NOTES ----------------------------------------------------------
# example outputs: 
# iperf -c 10.10.10.68 -t 10 -y c -u -b 1M
# 20090226144430,10.10.10.65,47632,10.10.10.68,5001,3,0.0-10.0,64425739815,51440119011
# --------------
# iperf -c 10.10.10.65 -t 10 -y c -u -b 1M
# 20090226144426,10.10.10.68,48338,10.10.10.65,5001,3,0.0-10.0,1252440,999996
# 20090226144426,10.10.10.65,5001,10.10.10.68,48338,3,0.0-10.0,1252440,999986,0.004,0,852,0.000,0
# ----------------------------------------------------------------
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
# -----------------------------------------------------------------
# from wajig source iperf: src/Locale.c :
#   #ifdef HAVE_PRINTF_QD
#   XXX TCP
#   const char reportCSV_bw_format[] =
#   "%s,%s,%d,%.1f-%.1f,%qd,%qd\n";
#  XXX UDP: 
#   const char reportCSV_bw_jitter_loss_format[] =
#   "%s,%s,%d,%.1f-%.1f,%qd,%qd,%.3f,%d,%d,%.3f,%d\n";
#   #else // HAVE_PRINTF_QD
#   const char reportCSV_bw_format[] =
#  "%s,%s,%d,%.1f-%.1f,%lld,%lld\n";
#   
#   const char reportCSV_bw_jitter_loss_format[] =
#   "%s,%s,%d,%.1f-%.1f,%lld,%lld,%.3f,%d,%d,%.3f,%d\n";
#   #endif // HAVE_PRINTF_QD
# -----------------------------------------------------------------

# XXX timestamp format : 
# "%Y%m%d%H%M%S"


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
        self.current_contact = None
        self.iperf_server = None
        self.iperf_client = None
        self.api = None
        self.state = STATE_IDLE #STOPPED
        self.iperf_server_is_running = False
        # ------ current stats and config
        self.current_remote_addr = None
        #self.local_addr = None
        self.current_bandwidth = 10 # Mbits
        self.current_duration = 1 # seconds
        self.current_caller = None # instance
        self.current_stats_local = None # dict
        self.current_stats_remote = None # dict
        self.current_com_chan = None # com_chan
        self.current_kind = None
        self.current_latency = 0 # milliseconds
        self.current_ping_started_time  = 0 # UNIX timestamp
        self.current_results_sent = False
        # -------- settings
        # TODO use those 2 vars:
        self.accept_timeout = 20 # timeout before auto rejecting dualtest query
        self.remote_results_timeout = 5 # how many extra seconds over the duration of a test to wait for remote stats before giving up.
        self.timeout_call_later_id = None

    def kill_server_process(self, process_transport, sig=15, verify_timeout=1.0):
        """
        Kills a process started using reactor.spawnProcess
        Double checks after 0.1 second
        
        Used in this case to kill the iperf server.
        """
        #process_transport = self.iperf_server
        try:
            process_transport.signalProcess(sig)
        except AttributeError, e:
            log.error("Error in kill_server_process: %s" % e.message)
        except ProcessExitedAlready, e:
            log.debug("Successfuly killed the iperf server. (%s)" % e.message)
            #self.state = STATE_IDLE
            self.iperf_server_is_running = False
        else:
            if sig == 15: # (first time)
                reactor.callLater(verify_timeout, self.kill_server_process, process_transport, 9)
                self.iperf_server_is_running = False
            else: # sig = 9 (second time)
                process_transport.loseConnection()
                self.iperf_server_is_running = False
        #TODO: make sure we do not start again the process !!!!!!!!!!!

    def on_iperf_server_process_done(self, intentional=True):
        """
        Called when the iperf -s process is done.
        """
        log.debug("iperf server is done.")
        self.iperf_server_is_running = False
        #if not intentional:
        #    print "iperf server process died unintentionally. will start it again." # TODO: log
        #    reactor.callLater(1.0, self._start_iperf_server_process)


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
        return self.current_contact.address
        # TODO : make more sturdy.
        ret = None
        contact = self.api.get_contact()
        try:
            ret = contact.address
        except:
            pass
        return ret

    def start_test(self, caller, bandwidth_megabits=1, duration=10, kind=KIND_UNIDIRECTIONAL): # start_client
        """
        Method called from the core API to try to start a network test.
        
        Starts `iperf -c localhost -y c -u -t 10 -b 1M`
        Actually, the process is started in self._start_iperf_client()
        
        :param duration: int
        :param bandwidth_megabits: int
        :param server_addr: str DEPRECATED PARAM
        :param kind: int Must match one of the constants in this module. 
        :return success: boolean
        """
        global _is_currently_busy 

        # TODO: remove com_chan argument
        # TODO: remove address argument
        if self.state != STATE_IDLE or _is_currently_busy :
            self.notify_api(caller, "error", "A network test is already in process.")
            return False
        else:
            self.current_stats_local = None # dict
            self.current_stats_remote = None # dict
            self.current_bandwidth = bandwidth_megabits # Mbits
            self.current_duration = duration # seconds
            self.current_caller = caller # instance
            self.current_results_sent = False
            # XXX
            # if com_chan is not None:
            #     log.debug("Using comm channel %s." % (com_chan))
            # #    self.current_com_chan
            #     self.current_com_chan = com_chan # only neededi for dualtests
            #     # XXX We register again our callbacks !!
            #     log.debug("registering our callback once again.")
            #     self._register_my_callbacks_to_com_chan(com_chan)
            # else:
            #     log.error("Invalid com_chan. It is None")
            self.current_kind = kind
            self.current_ping_started_time = self._get_time_now()
            self._start_iperf_server_process()
            self._send_message("ping")
            self.state = STATE_WAITING_REMOTE_ANSWER 
            _is_currently_busy = True
            self.timeout_call_later_id = reactor.callLater(_TIMEOUT + duration, self._timeout)
            return True

    def _timeout(self):
        """
        After a little while, if nothing happened, makes sure we are available for a test.
        """
        if _is_currently_busy or self.state != STATE_IDLE:
            self._when_done()
            if self.iperf_server_is_running: # XXX
                self._stop_iperf_server_process()

    def _when_done(self):
        """
        Call this when you are done to change out state to available.
        """
        global _is_currently_busy
        _is_currently_busy = False
        self.state = STATE_IDLE

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
        #log.debug("Calling %r." % (command))
        log.debug("Calling %r." % (" ".join(command)))
        callback = self.on_iperf_command_results
        try:
            commands.single_command_start(command, callback, extra_arg, self.current_caller)
        except CommandNotFoundError, e: 
            log.error("CommandNotFoundError %s" % e.message)
            self.notify_api(self.current_caller, "error", e.message)
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
        #if self.state != STATE_IDLE: 
        log.info("starting iperf server process")
        if self.iperf_server_is_running:
            log.error("Could not start iperf server, since it is already running.")
        else:
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
    
    def _cancel_timeout(self):
        """
        Cancels a call later to the method that cancels the iperf test in case of timeout.

        see twisted.internet.interfaces.IDelayedCall
        """
        try:
            if self.timeout_call_later_id.active():
                self.timeout_call_later_id.cancel()
            #reactor.cancelCallLater(self.timeout_call_later_id)
        #except AlreadyCancelled, e:
        #    pass
        except AttributeError, e:
            pass

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
        global _is_currently_busy
        # TODO: we should check from which contact this message is from !!!!!!!!!!

        #log.debug("received %s:(%s)" % (key, str(args)))
        if key == "ping": # from A
            self._start_iperf_server_process()
            if self.state != STATE_IDLE:
                log.error("Received a ping while being busy doing some network test. (state = %d)" % self.state)
                self._send_message("busy")
            if _is_currently_busy:
                log.error("Received a ping while being busy doing some network test. (_is_currently_busy)")
                self._send_message("busy")
            else:
                # TODO: use 
                # self.accept_timeout = 20 # timeout before auto rejecting dualtest query
                self._send_message("pong")
                _is_currently_busy = True
                self._cancel_timeout()
                self.timeout_call_later_id = reactor.callLater(_TIMEOUT, self._timeout) # XXX FIXME
        
        elif key == "busy": # from B
            self.notify_api(self.current_caller, "error", "Network test not possible. Remote peer is busy.")
            self._when_done()
            self._stop_iperf_server_process()

        elif key == "pong": # from B 
            if self.state != STATE_WAITING_REMOTE_ANSWER:
                log.error("Received pong while not being wainting for an answer. (state = %d)" % self.state)
            else:
                self.current_latency = self._get_time_now() - self.current_ping_started_time # seconds
                params = {
                    'duration':self.current_duration,
                    'bandwidth':self.current_bandwidth,
                    'latency':self.current_latency #, 
                    #'remote_addr':self.current_remote_addr
                }
                self._send_message("start", [self.current_kind, params])
                wait_for = self.current_latency
                log.debug("Will start iperf client in %f seconds" % wait_for)
                if self.current_kind != KIND_REMOTETOLOCAL: # the only case where we do not send from local to remote
                    reactor.callLater(wait_for, self._start_iperf_client)
                self._cancel_timeout()
                self.timeout_call_later_id = reactor.callLater(_TIMEOUT + self.current_duration, self._timeout) # XXX FIXME
        
        elif key == "start": # from A
            kind = args[0]
            params = args[1]
            self.current_kind = kind
            # the two cases where B starts an iperf client.
            if kind == KIND_DUALTEST or kind == KIND_REMOTETOLOCAL:
                # TODO
                if kind == kind == KIND_DUALTEST:
                    self.current_kind = KIND_DUALTEST_SERVER
                elif kind == KIND_REMOTETOLOCAL:
                    self.current_kind = KIND_REMOTETOLOCAL_SERVER
                log.debug("Starting iperf client.")
                self.current_duration = params['duration']
                self.current_bandwidth = params['bandwidth']
                self.current_latency = params['latency']
                #self.current_remote_addr = params['remote_addr'] # IMPORTANT 
                #self.current_remote_addr = self._get_remote_addr() # TODO XXX Make more sturdy
                wait_for = self.current_latency / 2.0
                log.debug("Will start iperf client in %f seconds" % wait_for)
                reactor.callLater(wait_for, self._start_iperf_client)
            else:
                log.debug("No test to start from this side.")
            self._send_message("ok")
        
        elif key == "ok": # from B
            log.debug("Test should be happening.")
        
        elif key == "results": # from B
            # [int kind, dict stats]
            kind = args[0]
            stats = args[1]
            self.current_stats_remote = stats
            #log.debug("received remote iperf client results : %r" % stats)
            self._send_results_if_ready()
            reactor.callLater(0.5, self._stop_iperf_server_process)
            self._send_message("stop")
        
        elif key == "stop": # from A
            #log.debug("Received stop")
            reactor.callLater(0.5, self._stop_iperf_server_process)
            self.state = STATE_IDLE 
            self._when_done()
        else:
            log.error("Unhandled com_chan message. %s" % key)
        log.debug("state is %d" % self.state)
    
    def _send_results_if_ready(self):
        """
        For each kind of test, check if it has gathered all the infos
        and notify the UI if the data in complete. 
        
        The resulting dict has keys 'local' and 'remote'
         * 'local' :  stats for local to remote
         * 'remote' : stats for remote to local
         * 'contact' : the name of the contact the test has been done with.
        """
        # TODO: use
        #self.remote_results_timeout = 5 # how many extra seconds over the duration of a test to wait for remote stats before giving up.
        if not self.current_results_sent:
            must_have_local = True
            must_have_remote = False
            ok = True
            results = {}

            if self.current_kind == KIND_DUALTEST: 
                must_have_remote = True
            if self.current_kind == KIND_REMOTETOLOCAL:
                must_have_remote = True
                must_have_local = False
            if must_have_remote:
                if self.current_stats_remote is None:
                    ok = False
                else:
                    results['remote'] = self.current_stats_remote
            if must_have_local:
                if self.current_stats_local is None:
                    ok = False
                else:
                    results['local'] = self.current_stats_local
            if ok:
                results['contact'] = self.current_contact
                self.notify_api(self.current_caller, "network_test_done", results)
                self.current_results_sent = True
                self._when_done()
        else:
            log.debug("results were already sent to observers.")

    def on_iperf_command_results(self, results, commands, extra_arg, caller):
        """
        Called once the iperf client child process is done.
        
        See utils.commands
        """
        for i in range(len(results)): # should have a len() of 1
            result = results[i]
            success, results_infos = result
            command = commands[i]
            
            if isinstance(results_infos, failure.Failure):
                log.error("FAILURE in on_iperf_command_results: " + str( results_infos.getErrorMessage()))  # if there is an error, the programmer should fix it.
            else:
                stdout, stderr, signal_or_code = results_infos
                if success:
                    # print "stdout:", stdout
                    ip = extra_arg['ip']
                    try:
                        iperf_stats = _parse_iperf_output(stdout.splitlines())
                        log.debug("iperf results: %s" % stdout)
                    except NetworkError, e:
                        self.notify_api(caller, 'error', e.message)
                        log.error("NetworkError in on_iperf_command_results" + e.message) # XXX
                    else:    
                        kind = extra_arg['kind']
                        iperf_stats['test_kind'] = kind
                        log.debug("on_iperf_commands_results : %r" % iperf_stats)
                        if kind == KIND_DUALTEST_SERVER or kind == KIND_REMOTETOLOCAL_SERVER:
                            #log.debug("KIND_DUALTEST_SERVER")
                            #log.debug('Sending iperf stats %r' % iperf_stats)
                            try:
                                self._send_message('results', [kind, iperf_stats]) 
                                # [int kind, dict stats]
                            except Exception, e:
                                log.error("Error sending results to remote peer : %s" % e.message)
                        else:
                            self.current_stats_local = iperf_stats
                            self._send_results_if_ready()
                            self._send_message("stop")
                        self.state = STATE_IDLE 
                else:
                    self.notify_api(caller, "error", "Network performance : Unknown error.") 
    
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
        log.debug("Sent %s. %r" % (key, args_list))
    
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

def on_com_chan_connected(connection_handle, role="client"):
    """
    Called from the Connector class in its com_chan_started_client or com_chan_started_server method.

    registers the com_chan callback for network_test
    
    :param connection_handle: connectors.Connection object.
    :param role: string "client" or "server"
    We actually do not care if this miville is a com_chan client or server.
    """
    global _testers

    # creates a new tester object
    tester = NetworkTester()
    contact = connection_handle.contact
    tester.current_contact = contact
    name = contact.name
    tester.current_com_chan = connection_handle.com_chan
    tester._register_my_callbacks_to_com_chan(tester.current_com_chan)
    tester.api = _api
    tester.current_remote_addr = contact.address
    _testers[tester.current_contact.name] = tester
    log.debug("testers: " + str(_testers))


def on_com_chan_disconnected(connection_handle):
    """
    Called when a connection is stopped
    """
    global _testers
    try:
        del _testers[connection_handle.contact.name]
        log.debug("testers: " + str(_testers))
    except Exception, e:
        log.error("error in on_com_chan_disconnected" + e.message)

def get_tester_for_contact(contact_name=None):
    """
    Returns a NetworkTester instance for contact name, or raises a  NetworkError.

    :param contact_name: unicode string
    """
    global _testers
    ret = None
    try:
        ret = _testers[contact_name]
    except KeyError:
        raise KeyError("In get_tester_for_contact: No Network Tester for contact.") # TODO: use NetworkError
    else:
        return ret




# functions ---------------------------------------------
def start(subject):
    """
    Initial setup of the whole module for miville's use.
    
    Notifies with 'error' key  if `iperf` command not found. (CommandNotFoundError)
    Returns a NetworkTester instance with the server started.
    """
    global _api

    try:
        executable = commands.find_command("iperf", 
            "`iperf` command not found. Please see https://svn.sat.qc.ca/trac/miville/wiki/NetworkTesting for installation instructions.")
    except CommandNotFoundError, e:
        if subject is not None:
            subject.notify(subject, e.message, "error") # notifies the user. But therer are no users at startup
        print e.message
    # tester = NetworkTester()
    _api = subject
    
    connectors.register_callback("networktest_on_connect", on_com_chan_connected, event="connect")
    connectors.register_callback("networktest_on_disconnect", on_com_chan_disconnected, event="disconnect")
    #return tester

