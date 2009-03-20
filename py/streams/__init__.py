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


# App imports
from streams.video.gst import AudioVideoGst

from errors import *

def create_engine(engine_name):
    engine_name = str(engine_name)
    if engine_name.upper() == 'GST':
        return AudioVideoGst()
    raise StreamsError, 'Engine "%s" is not supported' %  engine_name

        
def create__video_engine(engine_name):
    engine_name = str(engine_name)
    if engine_name.upper() == 'GST':
        return VideoGst()
    raise StreamsError, 'Engine "%s" is not supported' %  engine_name 