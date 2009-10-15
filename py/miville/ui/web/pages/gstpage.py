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

from miville.services.servicescomchan import get_all_gst_channels

from miville.errors import *


log = log.start('debug', 1, 0, 'gstpage')

def print_service_process_log(service):
    txt = ""
    try:
        txt += "<h3>Process output log</h3>"
        for index,msg in enumerate(service.output_logger.get()):
            txt += "<p><b>%d</b>  %s</p>" % (index+1, msg)
    except Exception, e:
        txt += "<h2>ERROR in print_service_process_log: %s </h2>" % e
    return txt

def print_service_telnet_log(service):
    txt = ""
    try:
        txt += "<h3>Telnet log</h3>"
        for index,msg in enumerate(service.logger.get()):
            txt += "<p><b>%d</b>  %s</p>" % (index+1, msg)
    except Exception, e:
        txt += "<h2>ERROR in print_service_telnet_log: %s </h2>" % e
    return txt

def print_service_acks(service):
    
    txt = ""
    try:
        txt += "<h3>Acknowledgments:</h3>"
        for item in service.acknowledgments:
            timestamp = item[0]
            msg = item[1]
            timestamp_str = timestamp.strftime("%Y-%m-%d %H:%M:%S")
            txt += "<p><b>%s</b> %s</p>" % (timestamp_str, msg)
    except Exception, e:
        txt += "<h2>ERROR in print_service_acks: %s </h2>" % e    
    return txt

def print_service_rtp_stats(service):
    txt = ""
    try:
        txt += "<h3>RTP stats</h3>"
        for stream, stat in service.rtp_stats.iteritems():
            txt += "<h4>RTP stats for %s</h4>" % stream
            for name, ring in stat.iteritems():
                stats = ring.get()
                for index,val in enumerate(stats):
                    if val == None:
                        txt += "<p><b>%d</b> None</p>" % (index+1)
                    else:
                        timestamp = val[0]
                        timestamp_str = timestamp.strftime("%Y-%m-%d %H:%M:%S")
                        value = val[1]
                        txt += "<p><b>%d</b>  %s %s = %s</p>" % (index+1, timestamp_str, name, value)
    except Exception, e:
        txt += "<h2>ERROR in print_service_rtp_stats: %s </h2>" % e
    return txt


def print_service_summary(service):
    txt = ""
    try:
        txt += "<h3>" + str(service) + "</h3>"
        txt += "<p>mode   = " + service.mode + "</p>"
        txt += "<p>group  = " + service.group_name + "</p>"
        txt += "<p>streams= " + str(service.stream_names) + "</p>"
        txt += "<p>process= " + str(service.proc_path) + "</p>"
        txt += "<p>args   = " + str(service.args)  + "</p>"
        txt += "<p>pid    = " + str(service.pid) + "</p>"
        txt += "<p>state  = " + str(service.get_status()) + "</p>"
        txt += "<p>port   = " + str(service.gst_port) + "</p>"
        txt += "<p>ip     = " + str(service.gst_address) + "</p>"
        txt += "<p>version= " + str(service.get_version_str()) + "</p>"
    except Exception, e:
        txt += "<h2>ERROR in print_service_summary: %s </h2>" % e
    return txt

def print_service(service):
    txt = ""
    try:
        txt += print_service_summary(service)
        txt += "<h3>Telnet commands</h3>"
        for cmd in service.commands:
            command = cmd[0]
            txt += "<p><b>" + command + ": </b>"
            params = cmd[1]
            if params == None:
                txt += str("<i>No parameter</i> ")
            else:
                # check if its iterable
                if not getattr(params, '__iter__', False):
                    txt += str("<i>" + str(params) + "</i> ")
                else:
                    for p in params:
                        v = str(p[1])
                        if isinstance(p[1], str) or isinstance(p[1], unicode):
                            v = '"' + v + '"'
                        txt += str(p[0]) + "= <i>" + v + "</i> "
            txt += "</p>"  
        
        txt += print_service_acks(service)    
        txt += print_service_rtp_stats(service)
        txt += print_service_telnet_log(service)
        txt += print_service_process_log(service)       
    except Exception, e:
        txt += "<h2>ERROR in print_service: %s </h2>" % e
    return txt

def print_proc_params(params, proc_name, service):
    txt = ""
    txt += "<h2>" + proc_name + "</h2>"
    txt += print_service(service)
    return txt

def print_channel(channel):
    txt = ""
    try:
        txt += "<p>Remote address: " + channel.remote_addr + "</p>"
        txt += "<p>contact %s</p>" % channel.contact.name
        txt += "<p>setting %s</p>" % channel.contact.setting
        
        rx_counter = 0
        tx_counter = 0
        if channel.receiver_procs_params:
            rx_counter = len(channel.receiver_procs_params)
        if channel.sender_procs_params:
            tx_counter = len(channel.sender_procs_params)
        txt += "<p><b><i>%d Rx process(es), %d Tx process(es)</i></b></p>" % (rx_counter, tx_counter)
        counter = 0
        if not channel.receiver_procs_params:
            txt += "<p>No active rx process</p>"
        else:
            for sync_group, params in channel.receiver_procs_params.iteritems():
                service = channel.receiver_services[counter]
                counter += 1
                txt += print_proc_params(params, "Rx process " + str(counter), service)
            
        counter = 0
        if not channel.sender_procs_params:
            txt += "<p>No active tx process</p>"
        else:
            for sync_group, params in channel.sender_procs_params.iteritems():
                service = channel.sender_services[counter]
                counter += 1
                txt += print_proc_params(params, "Tx process " + str(counter), service) 
        
        channel.sender_services
    except Exception, e:
        txt += "<h2>ERROR in print_channel: %s </h2>" % e
    return txt

def print_gst():
    txt = "<h1>Propulseart Gst services</h1>"
    channels_dict = get_all_gst_channels()
    for contact, channel in channels_dict.iteritems():
        txt += "<h2>Contact: %s</2>" % contact
        txt += print_channel(channel)
    return txt

def print_head():
    txt=u"""
<head>
<style type="text/css">
h1 {
    color: #00ff00
    font-family: courier
    font-size: 100%
}
p {font-family: courier}
h2 {
    color: #dda0dd
    font-family: courier
    text-indent: 1cm
    font-size: 100%
}
p {
    color: rgb(0,0,255)
    font-family: courier
    text-indent: 3cm
    font-size: 50%
}
h3 {
    font-family: courier
    text-indent: 2cm
    font-size: 100%
}
</style>
</head>
"""
    return txt

class GstPage(rend.Page):
    def renderHTTP(self, ctx):
        square_head = print_head()
        beau_body =  print_gst()
           
        html = u"""
        <html>
        """
        html += square_head 
        html += u"""<body>
        
            """
        html += beau_body     
        
        html += u"""
        
        </body>
        </html>
        """ 
        return str(html)


    
    
