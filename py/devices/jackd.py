#!/bin/bash
# -*- coding: utf-8 -*-
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
JACK driver

Attributes of its (jackd) devices: 
* jackd name
* number of system sinks and sources
* sample rate
* process id
* command line string that started it.
* buffer size
"""
#TODO: 
# ----------------------------------------------------------
# System imports
import os
import sys
import glob
import pprint

# Twisted imports
from twisted.internet import reactor, protocol
from twisted.python import procutils
from twisted.python import failure

# App imports
#from utils import log
#from utils.commands import *
from devices import *

def _parse_jack_lsp(lines):
    """
    Parses the output of the `jack_lsp` command.
    Returns a dict.
    """
    ret = {}

    if len(lines) == 1 and lines[0].strip() == "JACK server not running":
        ret['nb_sys_sinks'] = 0
        ret['nb_sys_sources'] = 0
        ret['running'] = False
    else:
        ret['running'] = True
        system_sinks = []
        system_sources = []
        for line in lines:
            line = line.strip()
            name = line.split(":")[1]
            if line.startswith("system:capture"):
                system_sinks.append(name)
            elif line.startswith("system:playback"):
                system_sources.append(name)
        ret['nb_sys_sinks'] = len(system_sinks)
        ret['nb_sys_sources'] = len(system_sources)
        #ret['sample_rate'] = 48000 # TODO
        #ret['server_name'] = "SPAM" # TODO
    return ret

#TODO: add to general errors.
class DeviceError(Exception):
    """
    Can be raised at Device.prepare() or when polling.
    """
    pass

def jackd_get_infos():
    """
    Looks in the file system for informations on running jackd servers. 

    Returns a list of dict
    """
    # /dev/shm/jack-$UID for jackd name 
    # g = glob.glob('/dev/shm/jack-1002/*/*-0')
    # file("/proc/28923/cmdline").read()
    ret = []
    i = 0
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
            ret.append({}) # populate a dict
            ret[i]['name'] = name # probably 'default' or $JACK_DEFAULT_SERVER
            ret[i]['pid'] = pid
            f = file("/proc/%d/cmdline" % (pid), "r")
            s = f.read()
            f.close()
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
                
                elif backend == 'freebob': # freebob arguments
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
        i += 1
    return ret

class JackDriver(AudioDriver):
    """
    Video4linux 2 Driver.
    
    self.selected_device = "/dev/video0"
    """
    name = 'JACK'
    _jack_server_name = 'default'
        
    def prepare(self):
        """
        Returns a Deferred instance? no
        """
        try:
            tmp = find_command('jack_lsp')
        except:
            raise CommandNotFoundError("jacklsp command not found. Please sudo apt-get install jack")
        name = os.getenv("JACK_DEFAULT_SERVER")
        if isinstance(name, str):
            self._jack_server_name = name

        return Driver.prepare(self)
    
    def _on_devices_polling(self, caller=None, event_key=None):
        """
        Get all infos for all devices.
        Starts the commands.

        Must return a Deferred instance.
        """
    def on_attribute_change(self, attr, caller=None, event_key=None):
        name = attr.name
        val = attr.get_value() # new value
        dev_name = attr.device.name
        ret = None
        #ret = single_command_start(command, self.on_commands_results, 'attr_change', caller) 
        
    def on_commands_results(self, results, commands, extra_arg=None, caller=None): 
        """
        Parses commands results
        
        extra_arg can be device name, such as /dev/video0
        """
        for i in range(len(results)):
            result = results[i]
            success, results_infos = result
            if isinstance(results_infos, failure.Failure):
                print "failure ::: ", results_infos.getErrorMessage()  # if there is an error, the programmer should fix it.
            else:
                command = commands[i]
                stdout, stderr, signal_or_code = results_infos
                if success:
                    arg = None
                    if isinstance(extra_arg, list):
                        arg = extra_arg[i]
                    else:
                        arg = extra_arg
                    self._handle_shell_infos_results(command, stdout, arg, caller) # this is where all the poutine happens
                else:
                    print "failure for command %s" % (command[0])
                    print "stderr: %s" % (stderr)
                    print "signal is ", signal_or_code
        if extra_arg == 'attr_change':
            event_name = 'attr'
        else:
            self._on_done_devices_polling(caller) # attr_change, 
    
    def _handle_shell_infos_results(self, command, results, extra_arg=None, caller=None):
    	"""
        Handles results for only one command received by on_commands_results
        
        See on_command_results() 
        """
        pass

def start(api):
    """
    Starts this driver
    """
    driver = JackDriver()
    driver.api = api
    managers['audio'].add_driver(driver)
    driver.prepare()



"""
#!/bin/bash
cd ~/bin

function var_declaration {
    for $(seq 1 8); do
        export JACK_SYSTEM_OUT$i="alsa_pcm:playback_$i"
        export JACK_SYSTEM_IN$i="alsa_pcm:capture_$i"
    done
}

function list_jack_ports {
    if [ "x$1" = "x" ]; then
        :
    else
        jack_lsp | grep $1 | sort
    fi
}

# MIDI ports
function find_midi_port {
    # arg1 = [o|i] (input/ouput)
    # arg2 = searched_string
    # TODO: Verify the args
    echo $(aconnect -l$1 | awk "/client.*$2/ { print \$2 "0" }" )
}

pd_midi_in=$(find_midi_port o Pure)
oxygen_midi_out=$(find_midi_port i Oxygen)

aconnect $oxygen_midi_out $pd_midi_in


# Audio ports
# TODO: check if pof is running

# set the prefixes
pof_out=$(jack_lsp  | egrep pof.*:out | sed 's/:.*$//' | uniq | sed 's/$/:out_jackaudiosink0_/')
pof_in=$(jack_lsp  | egrep pof.*:in | sed 's/:.*$//' | uniq | sed 's/$/:in_jackaudiosrc/')

pd_out=$(jack_lsp  | egrep pure_data.*:out | sed 's/:.*$//' | uniq | sed 's/$/:output/')
pd_in=$(jack_lsp  | egrep pure_data.*:in | sed 's/:.*$//' | uniq | sed 's/$/:input/')

system_out="system:capture_"
system_in="system:playback_"

# doo da sheet
set -x

jack_connect ${pof_out}5 ${system_in}1
sleep 2
jack_connect ${pof_out}5 ${system_in}4
sleep 2

for i in $(seq 1 4)
do
    j=$((i-1))
    jack_connect ${pd_out}$i ${system_in}$i 2>/dev/null
    sleep 2
    jack_connect ${pd_out}$i ${pof_in}${j}_1 2>/dev/null
    sleep 2
done

for i in $(seq 1 3)
do
    jack_connect ${system_out}$i ${pd_in}${i}
    sleep 2
done
jack_connect ${system_out}1 ${pof_in}4_1
"""


if __name__ == '__name__':
    print "JACK infos:"
    pprint.pprint(jackd_get_infos())

