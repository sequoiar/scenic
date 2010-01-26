#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
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
Starting point of miville from the command line.

Parses the arguments given at startup and changes the 
corresponding miville configuration variables. 

See the MivilleConfiguration class in miville/core.py
"""
from twisted.internet.error import CannotListenError
from twisted.internet import reactor
import sys
import os
import socket
from optparse import OptionParser
from miville.options import MivilleConfiguration
from miville.utils import log
from miville.utils import common

log_file_name = log.LOG_FILE_NAME

# FIXME: this should come from configure.ac
__version__ = "0.3.5"
VERSION = __version__

def moo():
    """
    This image is:
    Copyright (C) 2003, Vijay Kumar
    Permission is granted to copy, distribute and/or modify this image under the terms either:
    
    * the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version, or
    * the GNU Free Documentation License, Version 1.1 or any later version published by the Free Software Foundation; with the no Invariant Sections, with no Front-Cover Texts and with no Back-Cover Texts.
    """
    print("""
    _-`````-,           ,- '- .    
  .'   .- - |          | - .   `.  
 /.'  /                     `.   \ 
:/   :      _...   ..._      ``   :
::   :     /._ .`:'_.._\.    ||   :
::    `._ ./  ,`  :    \ . _.''   .
`:.      |   |  -.  \-. \|_       / 
  \:._ _/  ..'  .@)  \@) ` `\ ,.'  
     _/,--'       .- .\,-.`--`.    
       ,'/''     (( \ `  )     
        /'/'  \    `-'  (      
         '/''  `._,-----'      
          ''/'    .,---'       
           ''/'      ;:            
             ''/''  ''/        
               ''/''/''        
                 '/'/'         
                  `;               
    """)

def run():
    """
    Everything that is related to using Miville from 
    a shell such as command line arguments parsing, and environment
    variables checking must be here.
    """
    # command line parsing
    parser = OptionParser(usage="%prog", version=str(__version__))
    parser.add_option("-o", "--offset", dest="offset", default=0, type="int", \
        help="Specifies an offset for port numbers to be changed..")
    parser.add_option("-i", "--interfaces", type="string", action="append", \
        help="""Communication channel listen only to those network interfaces IP. Use this flag many times if needed. Default is all. Example: -i 127.0.0.1 -i 10.10.10.55""")
    parser.add_option("-m", "--miville-home", type="string", \
        help="Path to miville configuration files. (dot files)")
    parser.add_option("-w", "--web-template", type="string", default="default", \
        help="Name of the template for the Web interface.")
    parser.add_option("-M", "--moo", action="store_true", \
        help="There is no easter egg in this program.")
    parser.add_option("-C", "--disable-escape-sequences", action="store_true", \
        help="Disables ANSI escape sequences in shell and telnet clients.")
    parser.add_option("-j", "--restart-jackd", action="store_true", \
        help="Enables the restarting of jackd when it hangs or crashes.")
    parser.add_option("-I", "--ui-interfaces", type="string", action="append", \
        help="""User interfaces listen only to those network interfaces IP. Use this flag many times if needed. Default is localhost only. Example: -i 127.0.0.1 -i 10.10.10.55""")
    parser.add_option("-a", "--all-interfaces", action="store_true", \
        help="""Makes user interfaces listen to all network interfaces.""")
    parser.add_option("-v", "--verbose", dest="verbose", action="store_true", \
        help="Sets the output to be verbose.")
    (options, args) = parser.parse_args()
    if options.moo:
        moo()
        sys.exit(0)
    
    # configure miville
    config = MivilleConfiguration()
    # network interfaces for the communication channel
    if isinstance(options.interfaces, list):
        if isinstance(config.listen_to_interfaces, list):
            config.listen_to_interfaces.append(options.interfaces)
        else:
            config.listen_to_interfaces = options.interfaces
    # network interfaces for the user interfaces
    if isinstance(options.ui_interfaces, list):
        if isinstance(config.ui_network_interfaces, list):
            config.ui_network_interfaces.append(options.ui_interfaces)
        else:
            config.ui_network_interfaces = options.ui_interfaces
    if options.all_interfaces:
        config.ui_network_interfaces = ''
    if options.web_template:
        config.web_template = options.web_template
    if options.miville_home:
        config.miville_home = options.miville_home
    if options.verbose:
        config.verbose = options.verbose
    if options.restart_jackd:
        config.restart_jackd = True
    if options.disable_escape_sequences: # in both telnet CLI and this shell
        config.enable_escape_sequences = False
    # set the port offset        
    config.port_numbers_offset = options.offset
    # checks if files we need to write to are writable
    for f in [common.install_dir(log_file_name), common.install_dir(config.addressbook_filename)]:
        try:
            fp = open(f, 'a')
        except IOError, e:
            if os.path.exists(f):
                print("File %s is not writable by this user. Check its permission and try again.", f)
            else:
                d = os.path.dirname(f)
                if os.path.exists(d):
                    print("Directory %s is not writable by this user. Check its permissions and try again." % (d))
                else:
                    print("Directory %s does not exist. The application should be able to write in this directory." % (d))
            exit(1)
        else:
            fp.close()
    log.start('warning') # THIS IS THE LOG LEVEL FOR TWISTED AND PYTHON MESSAGES
    log.info('Starting Miville...')
    from miville.core import main
    # changes terminal title
    for terminal in ['xterm', 'rxvt']:
        if terminal.find and config.enable_escape_sequences:
            hostname = socket.gethostname()
            title = "mivilled on %s #%d" % (hostname, options.offset)
            sys.stdout.write(']2;%s' % (title))
    try:
        main(config)
        reactor.run()
    except CannotListenError, e:
        log.error(str(e))
        exit(1)
    print "Miville has quit."
    exit(0)

