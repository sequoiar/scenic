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


# App imports
from miville.engines import base_gst
from miville.engines import audiovideogst 
import gstchannel
from miville.utils import log
from miville.errors import *

log = log.start('debug', 1, 0, 'engines')

def create_channel(engine_name):
    log.debug('settings.create_channel: ' + str(engine_name) )
    engine_name = str(engine_name)
    if engine_name.upper() == 'GST':
        chan = gstchannel.GstChannel()
        return chan
    raise StreamsError, 'Engine "%s" has no communication channel' %  engine_name

