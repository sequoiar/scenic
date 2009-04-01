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

#import sys
#print sys.path
from optparse import OptionParser
from miville.core import *

__version__ = "0.1 alpha"

if __name__ == '__main__':
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
    parser.add_option("-I", "--ui-interfaces", type="string", action="append", \
        help="""User interfaces listen only to those network interfaces IP. Use this flag many times if needed. Default is localhost only. Example: -i 127.0.0.1 -i 10.10.10.55""")
    parser.add_option("-v", "--verbose", dest="verbose", action="store_true", \
        help="Sets the output to be verbose.")
    (options, args) = parser.parse_args()
    
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
    if options.miville_home:
        config.miville_home = options.miville_home
    # set the port offset        
    config.port_numbers_offset = options.offset

    log.start()
    log.info('Starting Miville...')
    # changes terminal title
    for terminal in ['xterm', 'rxvt']:
        if terminal.find:
            hostname = socket.gethostname()
            sys.stdout.write(']2;miville on ' + hostname + '')
    try:
        main(config)
        reactor.run()
    except CannotListenError, e:
        log.error(str(e))
        exit(1)
    exit(0)

