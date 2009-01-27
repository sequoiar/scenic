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


def kill_process(process_transport, sig=15, verify_timeout=1.0):
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
    global state
    for i in range(len(results)): # should have a len() of 1
        result = results[i]
        success, results_infos = result
        command = commands[i]
        
        if isinstance(results_infos, failure.Failure):
            print ">>>> FAILURE : ", results_infos.getErrorMessage()  # if there is an error, the programmer should fix it.
        else:
            stdout, stderr, signal_or_code = results_infos
            if success:
                print "stdout:", stdout
                iperf_stats = _parse_iperf_output(stdout.splitlines())
                print iperf_stats
                state = STOPPED
 
class NetworkError(Exception):
    """Any error due to iperf."""
    pass

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
            print line
            if line_index == 0:
                line_index += 1
            else: # 2nd line is the only one we need
                words = line.split(",")
                word_index = 0
                for word in words:# interval, transfert, jitter, packet loss (%)
                    k = keys[word_index]
                    if k == 'loss': 
                        ret[k] = float(word) # %
                    elif k == 'bandwidth': 
                        ret[k] = int(word) # Kbits/sec
                    elif k == 'jitter': 
                        ret[k] = float(word) # ms
                    elif k == 'loss': 
                        ret[k] = int(word) # %
                    word_index += 1
    return ret

# local  : datetime, local ip, local port, remote ip, remote port, test index, interval (0-10) sec, transfer (bytes), bandwidth (Mbit/sec)
# remote  : datetime, local ip, local port, remote ip, remote port, test index, interval (0-10) sec, transfer (bytes), bandwidth (Mbit/sec), 
#            jitter (ms), datagrams lost, total datagrams, % loss, datagrams out of order.
#
# WARNING: did not receive ack of last datagram after 10 tries.
# if client tells this, unable to reach server.

#alex@plank:~/src/miville/inhouse/py/network$ iperf -c brrr  -u -b 1M
#------------------------------------------------------------
#Client connecting to brrr, UDP port 5001
#Sending 1470 byte datagrams
#UDP buffer size:   109 KByte (default)
#    ------------------------------------------------------------
#    [  3] local 10.10.10.145 port 37468 connected with 10.10.10.65 port 5001
#    [ ID] Interval       Transfer     Bandwidth
#    [  3]  0.0-10.0 sec  1.19 MBytes  1000 Kbits/sec
#    [  3] Sent 852 datagrams
#    [  3] Server Report:
#    [  3]  0.0-10.0 sec  1.19 MBytes  1000 Kbits/sec  0.006 ms    0/  852 (0%)
def start_iperf_client(caller, server_addr, bandwidth_megabits=1, duration=10):
    """
    Starts  iperf -c localhost -u -t 10 -b 1M
    """
    global executable
    global state
    
    if state == STOPPED:
        command = ["iperf", "-c", server_addr, "-t ", str(duration), "-y", "c", "-u", "-b", "%dM" % (bandwidth_megabits)] 
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
            start_iperf_client(None, "10.10.10.65") # "localhost")
        else:
            print "usage: <file name> client"
    else:
        start_iperf_server()
    reactor.callLater(1.0, debug_meanwhile)
    reactor.run()
#    txt = """
#    20090126182857,10.10.10.145,35875,10.10.10.65,5001,3,0.0-10.0,1252440,999997
#    20090126182857,10.10.10.65,5001,10.10.10.145,35875,3,0.0-10.0,1252440,999970,0.008,0,852,0.000,0
#    """
#    
#    lines = txt.strip().splitlines()
#    pprint.pprint(_parse_iperf_output(lines))


