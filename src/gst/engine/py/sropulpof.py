#!/usr/bin/env python
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
#

from optparse import OptionParser

import libmilhouse

versionNum=str(libmilhouse.PACKAGE_VERSION) + '\b' + str(libmilhouse.RELEASE_CANDIDATE)

parser = OptionParser(version="%prog " + versionNum)
parser.add_option("-i", "--address", 
        dest="ip", default="127.0.0.1", help="provide ip address")
parser.add_option("-v", "--videocodec", 
        dest="videocodec", default="h264", help="video codec: h264, h263 or mpeg4")
parser.add_option("-a", "--audiocodec", 
        dest="audiocodec", default="raw", help="audio codec: vorbis, raw or mp3")
parser.add_option("-k", "--videosink", 
        dest="videosink", default="xvimagesink", help="videosink: xvimagesink or glimagesink")
parser.add_option("-t", "--audioport", 
        type="int", dest="audioport", help="audioport: 1024-60000")
parser.add_option("-p", "--videoport", 
        type="int", dest="videoport", help="videoport: 1024-60000")
parser.add_option("-s", "--sender", 
        action="store_true", dest="isSender", help="designate this process as sender")
parser.add_option("-r", "--receiver", 
        action="store_false", dest="isSender", help="designate this process as receiver")
parser.add_option("-f", "--fullscreen", 
        action="store_true", dest="fullscreen", help="set videowindow to fullsceen (receiver side only)")
parser.add_option("-d", "--videodevice", 
        dest="videoDevice", help="video4linux device to use: /dev/video0, /dev/video1")
parser.add_option("-n", "--screen", 
        type="int", dest="screenNum", help="xinerama screen number")
parser.add_option("-c", "--numChannels", 
        type="int", dest="numChannels", default=2, help="number of channels (sender side only)")

(options, args) = parser.parse_args()


