#!/usr/bin/env python
# -*- coding: utf-8 -*-
# 
# Scenic
# Copyright (C) 2008 Société des arts technologiques (SAT)
# http://www.sat.qc.ca
# All rights reserved.
#
# This file is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# Scenic is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Scenic. If not, see <http://www.gnu.org/licenses/>.
"""
Tools to help with choosing listening port numbers.
"""

# for port_is_avaiable
import socket 
from scenic import logger

log = logger.start(name="ports")

class PortsAllocatorError(Exception):
    """
    Any error raised by the PortsAllocator
    """
    pass

class PortsAllocator(object):
    """
    Allocates ports from a pool
    """
    def __init__(self, minimum=10000, increment=10, maximum=65535):
        self.minimum = minimum
        self.increment = increment
        self.maximum = maximum
        self.allocated = set()

    def check_port(self, port):
        """
        Verify that port is available by trying to bind to it. Socket
        does not persist. Note that some other process could still
        bind to this port in between when this check happens and when
        we bind to the port.
        
        :param port: int
        Raises a PortsAllocatorError if port is not available.
        """
        #TODO: return bool
        # Set the socket parameters
        host = 'localhost'
        addr = (host, port)
        busy = False

        # Create socket and bind to address
        udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        try:
            udp_socket.bind(addr)
        except socket.error, e:
            busy = True
        finally:
            # Close socket
            udp_socket.close()
        if busy:
            raise PortsAllocatorError("Port %d is not available. Reason:%s" % (port, e))
        
    def allocate(self):
        """
        Allocates a port number and returns it.
        """
        value = self.minimum
        chosen = False
        while not chosen:
            while value in self.allocated: # loop over allocated ports
                value += self.increment
                if value > self.maximum:
                    raise PortsAllocatorError("Maximum value reached. No more ports available.")
            try:
                self.check_port(value)
            except PortsAllocatorError, e:
                log.error('error: %s' % (e.message)) 
                value += self.increment
            else:
                chosen = True
        self.allocated.add(value)
        return value
    
    def free(self, value):
        """
        Frees an allocated port number.
        Raises a PortsAllocatorError if not allocated.
        """
        if value not in self.allocated:
            raise PortsAllocatorError("Value %d not in allocated ports set." % (value))
        self.allocated.remove(value)

        
    def allocate_many(self, num=1):
        """
        Allocates many ports at once.
        Returns a list of allocated ports.
        """
        ret = []
        for i in range(num):
            ret.append(self.allocate())
        return ret
    
    def free_many(self, values):
        """
        Frees many allocated ports at a time.
        :param values: list of integers.
        """
        for value in values:
            self.free(value)

