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
from protocols import osc_protocols
from streams import stream


class VideoGst(stream.VideoStream):
    """Class streams->video->gst.VideoGst
    """
    
    def get_attr(self, name):
        """
        name: 
        """
        return None # should raise NotImplementedError()
    
    def get_attrs(self):
        """
        """
        return None # should raise NotImplementedError()
    
    def set_attr(self, name):
        """
        name: 
        """
        return None # should raise NotImplementedError()
    
    def set_attrs(self):
        """
        """
        return None # should raise NotImplementedError()
    
    def start_sending(self, address):
        """
        address: string
        """
        return None # should raise NotImplementedError()
    
    def stop_sending(self):
        """
        """
        return None # should raise NotImplementedError()
    
    def start_receving(self):
        """
        """
        return None # should raise NotImplementedError()
    
    def stop_receving(self):
        """
        """
        return None # should raise NotImplementedError()
    

