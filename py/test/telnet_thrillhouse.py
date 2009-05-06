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

""" Run two milhouse servers before running this:
milhouse -r --serverport xxxxx
milhouse -s --serverport xxxx
"""

import telnetlib
import time


receiverServerport = 9000
senderServerport = 9001
TIMEOUT = 10

receiverTn = telnetlib.Telnet('localhost', receiverServerport)
senderTn = telnetlib.Telnet('localhost', senderServerport)

receiverTn.write('video_init: codec="h264" port=10000 address="127.0.0.1"\n')
receiverTn.read_until('video_init: ack="ok"')

senderTn.write('video_init: codec="h264" bitrate=3000000 port=10000 address="127.0.0.1" source="videotestsrc"\n')
senderTn.read_until('video_init: ack="ok"')

receiverTn.write('start:\n')
receiverTn.read_until('start: ack="ok"')
senderTn.write('start:\n')
senderTn.read_until('start: ack="ok"')


# wait a while
time.sleep(TIMEOUT)


receiverTn.write('stop:\n')
receiverTn.read_until('stop: ack="ok"')
senderTn.write('stop:\n')
senderTn.read_until('stop: ack="ok"')

receiverTn.write('quit:\n')
senderTn.write('quit:\n')


