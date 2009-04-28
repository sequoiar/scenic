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
This module is an html page for settings information


"""

try:
    from nevow import rend
except ImportError:
    raise ImportError, 'If you want to use the Web interface, you need to install Nevow.'

#App imports
from miville.utils import Observer, log
from miville.utils.i18n import to_utf
from miville.utils.common import find_callbacks

from miville.engines.gstchannel import get_all_gst_channels

from miville.errors import *


log = log.start('debug', 1, 0, 'web')


def print_engine(engine):
    txt = ""
    txt += "<h3>" + str(engine) + "</h3>"
    txt += "<p>mode  = " + engine.mode
    txt += "<p>group = " + engine.group_name
    txt += "<p>stream= " + engine.stream_name
    for cmd in engine.commands:
        command = cmd[0]
        txt += "<p><b>" + command + ": </b>"
        params = cmd[1]
        for p in params:
            v = str(p[1])
            if isinstance(p[1], str) or isinstance(p[1], unicode):
                v = '"' + v + '"'
            txt += str(p[0]) + "= <i>" + v + "</i> "
        txt += "</p>"      
    return txt

def print_proc_params(params, proc_name, engine):
    txt = "<h2>" + proc_name + "</h2>"
    txt += print_engine(engine)
    return txt

def print_channel(channel):
    txt = "<h3>Remote address: " + channel.remote_addr + "</h3>"
    
    counter = 0
    for sync_group, params in channel.receiver_procs_params.iteritems():
        engine = channel.receiver_engines[counter]
        counter += 1
        txt += print_proc_params(params, "Rx process " + str(counter), engine)
        
    counter = 0
    for sync_group, params in channel.sender_procs_params.iteritems():
        engine = channel.sender_engines[counter]
        counter += 1
        txt += print_proc_params(params, "Tx process " + str(counter), engine) 
    
    channel.sender_engines
    return txt

def print_gst():
    txt = "<h1>Propulseart Gst engines</h1>"
    channels_dict = get_all_gst_channels()
    for contact, channel in channels_dict.iteritems():
        txt = "<h2>Contact: %s</2>" % contact
        txt += print_channel(channel)
    return txt

class GstPage(rend.Page):
    def renderHTTP(self, ctx):
        beau_body =  print_gst()      
        html = u"""
        <html>
        <head></head>
        <body>
        
            """
        html += beau_body     
        
        html += u"""
        
        </body>
        </html>
        """ 
        return str(html)


    
    
