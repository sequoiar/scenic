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
States for streams.
"""
# states common to all streams
# states for streamers (and streams?)
# states for session, streams, streamers.
STATE_IDLE = "idle"
STATE_STARTING = "starting"     # map to 1
STATE_STREAMING = "streaming"   # map to 2
STATE_STOPPING = "stopping"     # map to 3
STATE_STOPPED = "stopped"       # map to 0
STATE_FAILED = "failed"
STATE_ERROR = "error"

# streamers modes
STREAMER_SENDER = "send"
STREAMER_RECEIVER = "recv"

# agent roles
ROLE_OFFERER = "offerer" # alice
ROLE_ANSWERER = "answerer" # bob
# directions
DIRECTION_TO_OFFERER = "TO_OFFERER"
DIRECTION_TO_ANSWERER = "TO_ANSWERER"
# locations
LOCATION_REMOTE = "remote"
LOCATION_LOCAL = "local"
