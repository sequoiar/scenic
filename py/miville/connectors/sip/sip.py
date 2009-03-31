# -*- coding: utf-8 -*-

# Miville
# Copyright (C) 2008 Société des arts technoligiques (SAT)
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


class ConnectionFactory(object):
    """Class that create an instance of a connection with the specified protocol.
    """
    def __init__(self, protocol):
        self.protocol = protocol
    
    # Operations
    def get_connection(self):
        connection = self.protocol()
        return connection


class Sip(object):
    """Sip connection protocol
    """
    def connect(self, address, port):
        """function connect
        
        address: string
        port: int
        
        returns 
        """
        return None # should raise NotImplementedError()
    
    def disconnect(self):
        """function disconnect
        
        returns 
        """
        return None # should raise NotImplementedError()
    
    def accept(self):
        """function accept
        
        returns 
        """
        return None # should raise NotImplementedError()
    
    def refuse(self, reason):
        """function refuse
        
        reason: 
        
        returns 
        """
        return None # should raise NotImplementedError()
    
    def build_sdp(self):
        """function build_sdp
        
        returns 
        """
        return None # should raise NotImplementedError()
    

