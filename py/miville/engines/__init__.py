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


# App imports
from miville.engines import base_gst
from miville.engines import audiovideogst 
from miville.engines import gstchannel

import gstchannel
from miville.utils import log
# from miville.errors import *
from miville import connectors

log = log.start('debug', 1, 0, 'engines')

def init_connection_listeners(api):
    """
    Registers the callbacks to the com_chan. 

    This function must be called from the API.
    """
    log.debug("engines.init_connection_listeners")
    connectors.register_callback("gst_on_connect", gstchannel.on_com_chan_connected, event="connect")
    connectors.register_callback("gst_on_disconnect", gstchannel.on_com_chan_disconnected, event="disconnect")
    gstchannel.set_api(api)

def get_channel_for_contact(engine_name, contact):
    """
    Returns a com_chan for the starting of milhouse process.
    :param engine_name: str "GST". Totally useless argument. 
    :param contact: miville.addressbook.Contact object.
    """
    # TODO: get rid of engine_name argument
    log.debug("engines.get_channel_for_contact engine='%s' contact='%s'" %  (engine_name, contact) )    
    engine_name = str(engine_name)
    if engine_name.upper() == 'GST':
        return gstchannel.get_gst_channel_for_contact(contact)
    
