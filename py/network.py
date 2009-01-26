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

# constants
STOPPED = 0
SERVER_RUNNING = 1
CLIENT_RUNNING = 2 

# globals
iperf_server = None
iperf_client = None
api = None
executable = None
state = STOPPED

# functions ---------------------------------------------
def start(subject):
    """
    Initial setup of the whole module.
    """
    global api, executable

    api = subject
    executable = commands.find_command("iperf", 
            "`iperf` command not found. Please see See https://svn.sat.qc.ca/trac/miville/wiki/NetworkTesting for installation instructions.")

class IperfServerProcessProtocol(protocol.ProcessProtocol):
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


def kill_process(process_transport, sig=15, verify_timeout=0.1):
    """
    Kills a process started using reactor.spawnProcess
    Double checks after 0.1 second
    """
    global state

    try:
        process_transport.signalProcess(sig)
    except Exception, e:
        print e
    if sig == 15:
        reactor.callLater(verify_timeout, kill_process, process_transport, 9)
    else:
        process_transport.loseConnection()
        state = STOPPED

def on_iperf_client_results(results, commands, extra_arg, caller):
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
                _parse_iperf_output(stdout.splitlines())
 
def _parse_iperf_output(lines):
    print "TODO: parse, change state and notify"
    for line in lines:
        print line
        if line.endswith("%)"):
            pass #percent = 

def start_iperf_client(caller, server_addr, bandwidth_megabits=1):
    """
    Starts  iperf -c localhost -u -b 1M
    """
    global executable
    global state

    if state == STOPPED:
        command = ["iperf", "-c", server_addr, "-u", "-b", "%dM" % (bandwidth_megabits)] 
        #deferred = commands._command_start(executable, command)
        extra_arg = None
        callback = on_iperf_client_results
        try:
            commands.single_command_start(command, callback, extra_arg, caller)
        except Exception, e:
            print e
        else:
            state = CLIENT_RUNNING

def start_iperf_server():
    """
    Starts `iperf -s -u`
    """
    global state, iperf_server

    if state == STOPPED:
        proto = IperfServerProcessProtocol()
        try:
            iperf_server = reactor.spawnProcess(proto, "/usr/bin/iperf", ["/usr/bin/iperf", "-s", "-u"], os.environ)
        except OSError, e:
            print "Error starting process: ", e 
        else:
            state = SERVER_RUNNING
            reactor.callLater(15.0, kill_process, iperf_server)

def debug_meanwhile():
    global state
    print "waiting...", state
    reactor.callLater(1.0, debug_meanwhile)

if __name__ == "__main__":
    if len(sys.argv) == 2:
        if sys.argv[1] == "client":
            print "starting client..."
            start_iperf_client(None, "localhost")
        else:
            print "usage: <file name> client"
    else:
        start_iperf_server()
    reactor.callLater(1.0, debug_meanwhile)
    reactor.run()

