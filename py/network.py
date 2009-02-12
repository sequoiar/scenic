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
# --------------------------------- server side ---------------------
class TestServerProtocol(basic.LineReceiver):
    """
    server side communication protocol

    usage :
    * quit
    * test <float ms> args
    * ping
    """
    def connectionMade(self):
        print "Connection from", self.transport.getPeer().host

    def lineReceived(self, line):
        if line == 'quit':
            self.sendLine("Goodbye.")
            self.transport.loseConnection()
        elif line.startswith("test"):
            args = line.split(" ")[1:]
            print "server received test with args : ", args
            self.sendLine("starting test")
        elif line == "ping":
            print "server received ping"
            self.sendLine("pong")
        else:
            self.sendLine("You said: " + line)

class TestServerFactory(protocol.ServerFactory):
    protocol  = TestServerProtocol
# ------------------------------------- client side ----------------
class TestClientProtocol(basic.LineReceiver):
    """
    client side protocol for network tests

    usage : 
    * starting test
    * pong 
    """
    def __init__(self):
        self.ping_started = 0.0000 # time.time()

    def lineReceived(self, line):
        #log.debug('Line received from %s:%s: %s' % (self.transport.realAddress[0], self.transport.realAddress[1], line))
        #log.info('Bad command receive from remote')
        #print "client received ", line

        if line == "starting test":
            print "client received starting test"
            self.sendLine("quit")
            #self.transport.loseConnection()

        elif line == "pong":
            print "client received pong"
            latency = (time.time() - self.ping_started) * 1000 # roughly estimated in ms
            print "latency : %f ms" % latency 
            latency = latency / 2.0 # unidirectional
            self.sendLine("test %f" % latency)

    def connectionMade(self):
        self.factory.deferred.callback("Connected !")
        self.ping_started = time.time()
        self.sendLine("ping")

    def connectionLost(self, reason):
        """
        The reason Failure wraps a twisted.internet.error.ConnectionDone or twisted.internet.error.ConnectionLost instance
        """
        print "client : connectionLost"
        try:
            print reason.message
        except AttributeError:
            pass
        #reactor.stop()

class TestClientFactory(protocol.ClientFactory):
    protocol = TestClientProtocol
    def __init__(self):
        self.deferred = defer.Deferred()

    def clientConnectionFailed(self, connector, reason):
        self.deferred.errBack(reason)

class TestClient:
    """
    The actual client for network tests
    
    This is the class that does the job.
    """
    # constants 
    DISCONNECTED = 0
    CONNECTED = 1

    def __init__(self): #, contact, api):
        self._state = self.DISCONNECTED

    def connect_to(self, host, port):
        """
        Connects to a remote TestServerFactory

        :return: a defer.Deferred instance.
        """
        test_factory = TestClientFactory()
        reactor.connectTCP(host, port, test_factory)
        test_factory.deferred.addCallback(self.handle_success, host, port)
        test_factory.deferred.addErrback(self.handle_failure, host, port)
        return test_factory.deferred

    def handle_success(self, result, host, port):
        """
        Callback for successful connection to a remote test server
        """
        print "connected to port %i" % port
        self.state = self.CONNECTED

    def handle_failure(self, failure, host, port):
        """
        Errback for a failure when trying to connect to a remore test server.
        """
        print "error connecting to port %i" % port
        print failure.getErrorMessage()
        # reactor.stop()


# -------------------------------- iperf --------------------------------------
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
# --------------------- constants --------------------------------
NETPERF_TEST_UNIDIRECTIONAL = "unidirectional from local to remote"

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
        print "Ended iperf server, status %d" % status_object.value.exitCode
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
        except ProcessExitedAlready, e:
            print "error : ProcessExitedAlready", e.message
        else:
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
                    try:
                        iperf_stats = _parse_iperf_output(stdout.splitlines())
                        if len(iperf_stats) == 0:
                            # TODO
                            raise NetworkError("iperf command gave unexpected results. Is remote iperf running ?")
                    except NetworkError, e:
                        self.notify_api(caller, 'info', e.message)
                    else:    
                        iperf_stats.update(extra_arg)
                        self.state = self.STOPPED

                        self.notify_api(caller, 'info', "iperf stats ifor IP %s: %s" % (ip, str(iperf_stats)))
                        self.notify_api(caller, "network_test_done", iperf_stats)
                else:
                    self.notify_api(caller, "info", "network performance : Unknown error.") # TODO use error key

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

    def start_bidirectional_as_client(self, caller, server_addr, bandwidth_megabits=1, duration=10):
        # TODO
        if self.state == self.STOPPED:
            
           self._on_bidirectional_ok(caller, server_addr, bandwidth_megabits, duration)
        else:
            self.notify_api(caller, "error", "Network test is already happening.")

    def _on_bidirectional_ok(self, caller, server_addr, bandwidth_megabits=1, duration=10):
        # TODO
        """
        Called when remote server has said it would be ok to start the iperf bidirectional test.
        """
        self.start_client(caller, server_addr, bandwidth_megabits, duration)

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
            except CommandNotFoundError, e: # TODO: 
                print "error: ", e.message
            else:
               self.state = self.CLIENT_RUNNING
        else:
            self.notify_api(caller, "error", "Network test is already happening.")

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
    try:
        executable = commands.find_command("iperf", 
            "`iperf` command not found. Please see See https://svn.sat.qc.ca/trac/miville/wiki/NetworkTesting for installation instructions.")
    except CommandNotFoundError, e:
        if subject is not None:
            subject.notify(subject, e.message, "error")
        print e.message
    tester = NetworkTester()
    tester.api = subject
    tester.start_server()
    
    client = NetworkTester()
    client.api = subject
    
    return {'server':server, 'client':client}

    #    txt = """
    #    20090126182857,10.10.10.145,35875,10.10.10.65,5001,3,0.0-10.0,1252440,999997
    #    20090126182857,10.10.10.65,5001,10.10.10.145,35875,3,0.0-10.0,1252440,999970,0.008,0,852,0.000,0
    #    """
    #    
    #    lines = txt.strip().splitlines()
    #    pprint.pprint(_parse_iperf_output(lines))

# -------------------------- MAIN ---------------------------------
if __name__ == "__main__":
    if True: #False: # ------------------- test iperf
        # SIMPLE TEST
        def debug_meanwhile():
            # TODO : remove
            print "waiting..."
            reactor.callLater(0.5, debug_meanwhile)
        
        # default : client
        
        iperf = NetworkTester()
        if len(sys.argv) == 2:
            if sys.argv[1] == "server":
                iperf.start_server()
            else:
                print "usage: <file name> [server]"
        else:
            print "starting client..."
            iperf.start_client("Dummy caller", "10.10.10.66", 1, 1) # "localhost" | tzing !
        reactor.callLater(0.5, debug_meanwhile)
        reactor.callLater(2.0, lambda: reactor.stop())
    #    try:
        reactor.run()
    #    except KeyboardInterrupt:
    #        print "you pressed ctrl-c"
    #        reactor.stop() 
        print "-----------------------------------"
        print "reactor has stopped"
    else: # ---------- test network protocol
        # ----------------------------------------------------------------
        # and now, the test for the network test protocol.
        port = 15432
        host = "localhost"
        reactor.listenTCP(port, TestServerFactory())
        print "Server running on port %d, press ctrl-C to stop." % (port)
        print "starting client"
        client = TestClient()
        client.connect_to(host, port)
        #reactor.callLater(2.0, lambda: reactor.stop())
        #try: 
        reactor.run()
        #except KeyboardInterrupt:
        #    print "you pressed ctrl-c"
        #    reactor.stop() 
        print "reactor has stopped"

