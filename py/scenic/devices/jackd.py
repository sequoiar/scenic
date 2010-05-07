#!/usr/bin/env python 
# -*- coding: utf-8 -*-
# Miville
# Copyright (C) 2008 Société des arts technologiques (SAT)
# http://www.sat.qc.ca
# All rights reserved.
#
# This file is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# Miville is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Miville.  If not, see <http://www.gnu.org/licenses/>.

"""
JACK driver

Attributes of its (jackd) devices: 
* jackd name
* number of system sinks and sources
* sample rate
* process id
* command line string that started it.
* buffer size

WARNING: For now, only supports short options to jackd. (such as "-d alsa" and not "--driver alsa")
"""
import os
import sys
import glob
import pprint
from twisted.internet import reactor
from twisted.python import failure

def double_fork(args):
    """
    Starts a process and double fork so init (PID 1) becomes
    the parent pid.
    """
    # TODO: should return/raise in case of error/success
    # or at least process ID... ?
    # print("The magic of the double fork.")
    if os.fork() != 0: # die if not in the child process 
        return 
        # sys.exit(0)
    if reactor.running:
        reactor.stop()
    os.umask(0) # set current process' file creation mode to octal 000
    os.setsid() # setsid runs a program in a new session. 
    if os.fork() != 0: # die if not the child process 
        sys.exit(0)
    # execute the command line to start jackd using the same stdout, stderr and stdin as the parent process.
    # last chance to print something on the current stdin
    print "Executing the %s command in a double fork." % (str(args))
    #print "JACK should be running."
    #print "ps aux | grep jackd | grep -v grep"
    sys.stdout.flush()
    sys.stderr.flush()
    stdin = file('/dev/null', 'r')
    stdout = file('/dev/null', 'a+')
    stderr = file('/dev/null', 'a+', 0)
    # dup2(int oldfd, int newfd)
    # create a copy of the file descriptors.
    os.dup2(stdin.fileno(), sys.stdin.fileno())
    os.dup2(stdout.fileno(), sys.stdout.fileno())
    os.dup2(stderr.fileno(), sys.stderr.fileno())
    os.execv(args[0], args) # TODO: use execve to send os.environ

class JackFrozenError(Exception):
    pass

def jackd_get_infos():
    """
    Looks in the file system for informations on running jackd servers. 
    
    Returns a list of dict : 
    [{'backend': 'alsa',
    'device': 'hw:0',
    'name': 'default',
    'nperiods': 2,
    'period': 1024,
    'pid': 7471,
    'rate': 44100}]
    """
    global _state_printed_jackd_is_frozen
    # TODO: each short arg has a long version which should be implemented too.
    # /dev/shm/jack-$UID for jackd name 
    # g = glob.glob('/dev/shm/jack-1002/*/*-0')
    # file("/proc/28923/cmdline").read()
    ret = []
    i = 0 # jackd number
    uid = os.getuid() # or os.geteuid() ?
    all = glob.glob("/dev/shm/jack-%d/*/*-0" % (uid))
    for running in all:
        sp = running.split('/')
        try:
            name = sp[4] # probably 'default'
            pid = sp[5].split('-')[3]
            pid = int(pid)
        except IndexError:
            pass
        except ValueError:
            pass
        else:
            ret.append({})
            ret[i]['name'] = name # probably 'default' or $JACK_DEFAULT_SERVER
            ret[i]['pid'] = pid
            # adding some default values!! XXX
            ret[i]["period"] = 1024
            ret[i]["nperiods"] = 0 
            ret[i]["rate"] = 48000
            filename = "/proc/%d/cmdline" % (pid)
            try:
                f = file(filename, "r")
                s = f.read()
                f.close()
            except IOError, e:
                try:
                    os.kill(pid, 0) # test if process is running
                except OSError, e:
                    pass # it is running
                    ret.pop()
                else:
                    msg = "Jackd seems frozen. IOError : (trying to read %s) PID %s is still alive. %s" % (filename, pid, e)
                    raise JackFrozenError(msg)
            else:
                _state_printed_jackd_is_frozen = False 
                # '/usr/bin/jackd\x00-dalsa\x00-dhw:0\x00-r44100\x00-p1024\x00-n2\x002\x00'
                cmdline = s.split('\x00') # ['/usr/bin/jackd', '-dalsa', '-dhw:0', '-r44100', '-p1024', '-n2', '2', '']
                backends = ['alsa', 'freebob'] # supported backends so far. #TODO: add more
                backend = None
                for arg in cmdline:
                    arg_name = None
                    cast = int
                    default = None
                    offset = 2
                    if backend == None: # jackd general arguments
                        if arg.startswith('-d'): # backend (actual audio driver)
                            arg_name = 'backend'
                            cast = str
                    
                    elif backend == 'freebob' or backend == "firewire": # freebob arguments
                        #Default values for freebob:
                        #-p, --period    Frames per period (default: 1024)
                        #-n, --nperiods  Number of periods of playback latency (default: 3)
                        #-r, --rate      Sample rate (default: 48000)
                        # TODO: For USB audio devices it is recommended to use -n 3.  Firewire  devices  sup‐
                        # ported by FFADO (formerly Freebob) are configured with -n 3 by default.
                        if arg.startswith('-r'):
                            arg_name = 'rate'
                            default = 48000
                        elif arg.startswith('-n'):
                            arg_name = 'nperiods'
                            default = 3
                        elif arg.startswith('-p'):
                            arg_name = 'period'
                            default = 1024
                    
                    elif backend == 'alsa': # ALSA arguments
                        if arg.startswith('-r'): # common for all channels.
                            arg_name = 'rate'
                            default = 48000
                        elif arg.startswith('-n') and backend == 'alsa': # -n is for JACK name, or -npriods if ALSA arg
                            arg_name = 'nperiods' # default is 2. See man jackd
                            default = 2
                        elif arg.startswith('-p'):
                            arg_name = 'period'
                            default = 1024
                        elif arg.startswith('-d'):
                            arg_name = 'device'
                            cast = str
                            default = "hw:0"# The ALSA pcm device name to use.  If none is specified, JACK will use "hw:0",
                                            # the first hardware card defined in /etc/modules.conf.
                    # now actually assign it.
                    if arg_name is not None:
                        try:
                            val = cast(arg[offset:].strip()) # TODO: assumes that jackd has been started with short args form. (-r and not --rate)
                            ret[i][arg_name] = val 
                        except ValueError:
                            ret[i][arg_name] = default
                        else:
                            # All the arguments after having specified a backend (-d alsa) are arguments specific for this backend.
                            if arg_name == 'backend':
                                for b in backends:
                                    if val == b:
                                        backend = b
                # now, the command line args used to start jackd (as a string)
                ret[i]["cmdline"] = ""
                for arg in cmdline[1:]:
                    ret[i]["cmdline"] += " " + arg
            # if set to 0, it was not set in the CLI, so we set it to the default according to backend
            if len(ret) > 0:
                #print "i = ", i
                if ret[i]["nperiods"] == 0:
                    if ret[i]["backend"] == "freebob":
                        ret[i]["nperiods"] = 3
                    else:
                        ret[i]["nperiods"] = 2
            i += 1 # very important...
    return ret

if __name__ == "__main__":
    def _poll_jackd():
        """
        @rtype: Deferred
        """
        try:
            jack_servers = jackd_get_infos() # returns a list a dict such as :
        except JackFrozenError, e:
            print e
        else:
            print jack_servers
    _poll_jackd()
