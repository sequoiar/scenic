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
Starts jackd with the arguments passed in the command line
"""

import sys
import os
import subprocess

JACKD_BIN = "/usr/bin/jackd"
USAGE = "-dalsa -dhw:0 -r44100 -p1024 -n2"

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print "Error: Not enough arguments to start jackd"
        print "Usage:", sys.argv[0], USAGE
        sys.exit(1)

    args = sys.argv[1:]
    args.insert(0, JACKD_BIN)
    # start the process
    try:
        pid = subprocess.Popen(args).pid
    except OSError, e:
        print "Error starting JACK", e
        sys.exit(1)
    print pid
    sys.exit(0)

