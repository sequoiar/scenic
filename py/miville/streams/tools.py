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
Tools for deferreds, delayed calls and process management.
"""
#TODO: Most of this should be deprecated.
import os
# for port_is_avaiable
import socket 

from twisted.internet import defer
from twisted.internet import error
from twisted.python import failure
from twisted.internet import protocol
from twisted.internet import reactor
from twisted.python import procutils

from miville.utils import sig
from miville.utils import log

log = log.start("info", True, True, "streams.tools")

class StreamerProblem(object):
    """
    Any problem reported by a streamer
    """
    def __init__(self, message, details=u''):
        self.message = message
        self.details = details

    def __str__(self):
        return '%s %s' % (self.message, self.details)

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
                log.error(e.message) 
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
        
class AsynchronousError(Exception):
    """
    Raised by DeferredWrapper or DelayedWrapper
    """
    pass

#class ManagedProcessError(Exception):
#    """
#    Raised by ProcessManager
#    """
#    pass

class DeferredWrapper(object):
    """
    Wraps a Deferred.
    We need to know if it has been called or not.
    """
    def __init__(self):
        self.deferred = None
        self.is_called = False
        self.is_success = None
        self._created = False
    
    def make_deferred(self):
        """
        Returns a Deferred
        """
        if self._created:
            raise AsynchronousError("This Deferred has already been set up.")
        else:
            self.deferred = defer.Deferred()
            self._created = True
            return self.deferred
    
    def callback(self, result):
        """
        Wraps Deferred.callback(result)
        """
        if not self._created:
            raise AsynchronousError("This Deferred has not been set up yet.")
        else:
            if not self.is_called:
                self.is_success = True
                self.is_called = True
                log.debug("Calling deferred callback %s" % (result))
                self.deferred.callback(result)
            else:
                pass # TODO: add a warning
    
    def errback(self, error):
        """
        Wraps Deferred.errback(error)
        """
        if not self._created:
            raise AsynchronousError("This Deferred has not been set up yet.")
        else:
            if not self.is_called:
                self.is_success = False
                self.is_called = True
                log.debug("Calling deferred errback %s" % (error))
                self.deferred.errback(error)
            else:
                pass # TODO: add a warning

class DelayedWrapper(object):
    """
    Wraps reactor.callLater and twisted.internet.base.DelayedCall
    Adds a Deferred to it.

    You must call the call_later method once an instance is created.
    """
    def __init__(self): #, delay, callback, *args, **kwargs):
        """
        Wraps reactor.callLater(...)
        """
        self.deferred_wrapper = None
        self._to_call = None
        self.delayed_call = None
        self._args = []
        self._kwargs = {}
        self.is_called = False
        self.is_cancelled = False
        self.is_scheduled = False
    
    def call_later(self, delay, function, *args, **kwargs):
        """
        Returns a Deferred
        
        :param delay: Duration in seconds.
        :param function: callable
        :param args: list or args to pass to function to call.
        """
        self._to_call = function
        self.deferred_wrapper = DeferredWrapper()
        self._args = args
        self._kwargs = kwargs
        self.delayed_call = reactor.callLater(delay, self._call_it)
        self._args = args
        self._kwargs = kwargs
        self.is_scheduled = True
        log.debug("Creating delayed (%f s) %s" % (delay, function))
        return self.deferred_wrapper.make_deferred()
    
    def _call_it(self):
        log.debug("Calling delayed %s" % (self._to_call))
        if self.is_scheduled:
            result = self._to_call(*self._args, **self._kwargs)
            if isinstance(result, failure.Failure):
                self.deferred_wrapper.errback(result)
            else:
                self.deferred_wrapper.callback(result)
            self.is_called = True

    def cancel(self, result):
        """
        Cancels the delayed call.
        
        If result is a Failure, calls the deferred errback, otherwise, calls it callback
        with the result.
        """
        # TODO: verify this, test it, and think about it.
        if self.is_scheduled and not self.is_cancelled and not self.is_called:
            try:
                self.delayed_call.cancel()
            except error.AlreadyCancelled, e:
                log.error("Cancelling already cancelled function %s" % (self._to_call.__name__))
                pass #self.is_cancelled = True
            except error.AlreadyCalled, e:
                log.error("Calling already called function %s" % (self._to_call.__name__))
                pass #self.is_called = True
            else:
                log.debug("Cancelling delayed %s giving it result %s" % (self._to_call.__name__, result))
                self.is_cancelled = True
                if isinstance(result, failure.Failure):
                    self.deferred_wrapper.errback(result)
                else:
                    self.deferred_wrapper.callback(result)
                self.is_cancelled = True
            #return defer.fail(failure.Failure(StreamError("Cannot stop process. Process is not running.")))

def deferred_list_wrapper(deferreds):
    """
    Wraps a DeferredList. All results are strings and are concatenated !!
    
    You should add individual callbacks to each deferred prior to pass it as an argument to this function. In this case, do not forget to return the result in the callbacks.
    
    :param deferreds: list of Deferred instances.
    """
    def _cb(result, d):
        overall_success = True
        overall_msg = ""
        overall_err_msg = ""
        #exc_type = AsynchronousError # type of exception
        #_failure = None
        for (success, value) in result:
            if success:
                overall_msg += str(value)
                #log.debug("defereredlist wrapper : success: %s" % (value))
            else:
                overall_success = False
                msg = value.getErrorMessage()
                #_failure = success # takes the last failure and throw it !
                overall_err_msg += msg
                #log.debug("deferrelist wrapper : failure: %s" % (msg))
        if overall_success:
            log.debug("deferredlist wrapper : overall success")
            d.callback(overall_msg)
        else:
            log.debug("deferredlist wrapper : overall error")
            # TODO:
            # we use to concatenate the error messages and fail with 
            # an AsynchronousError :
            # d.errback(failure.Failure(AsynchronousError(overall_err_msg)))
            # Now, we errback with the last failure in the deferred list 
            # in order to keep the original exception type
            # d.errback(_failure)
            d.errback(failure.Failure(AsynchronousError(overall_err_msg)))
    d = defer.Deferred()
    dl = defer.DeferredList(deferreds, consumeErrors=True)
    dl.addCallback(_cb, d) # this deferred list needs no errback
    return d

