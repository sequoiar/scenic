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

from core import *
from optparse import OptionParser
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
        help="""Listens only to those interfaces IP. Use this flag many times if needed. Example: -i 127.0.0.1 -i 10.10.10.55""")
    parser.add_option("-v", "--verbose", dest="verbose", action="store_true", \
        help="Sets the output to be verbose.")
    (options, args) = parser.parse_args()
    
    # configure miville
    config = MivilleConfiguration()
    if options.interfaces is not None:
        # print "INTERFACES: %s" % (options.interfaces)
        config.listen_to_interfaces.append(options.interfaces) # a list

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

