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
Log file monitoring tool.

Result is similar to the tail command line utility.::
 tail -F
"""
import os
from twisted.internet import task

class Tail(object):
    """
    File monitoring tool.::
    
     tail -F 

    Uses Twisted or not. If not, simply call the poll_file method 
    every several seconds. 
    
    Excerpt from the tail man page::
     The tail utility displays the contents of file or, by default, its 
     standard input, to the standard output.
     
     The -F option implies the -f option, but tail will also check to
     see if the file being followed has been renamed or rotated.  The
     file is closed and reopened when tail detects that the filename
     being read from has a new inode number.  The -F option is ignored
     if reading from standard input rather than a file.
    """
    def __init__(self, file_name=None, callback=None, interval=0.1, autostart=False):
        self.file_name = file_name
        if self.file_name is None:
            self.file_name = os.devnull
        self.callback = callback
        self.interval = interval
        self.last_line_read = 0
        self.looping_call = task.LoopingCall(self.poll_file)
        if autostart:
            self.start()
        
    def start(self):
        """
        Starts the auto polling using twisted.
        """
        self.looping_call.start(self.interval, False)
    
    def stop(self):
        """
        Stops the auto polling using twisted.
        """
        self.looping_call.stop()
        
    def poll_file(self):
        """
        Reads the file to see if there are new lines of text. 
        Calls the callback with list of lines if there are.
        """
        try:
            f = open(FILENAME, 'rU') # converting to UNIX line ending
            f.seek(0, 2) # end
            end = f.tell() # get position
            diff = end - self.last_line_read # leftover to read
            if diff > 0:
                f.seek(-diff, 2) # rewind
                self.callback(f.readlines()) # call callback with lines
            f.seek(0, 2) # end
            self.last_line_read = f.tell() # get position
            f.close()
        except IOError, e:
            pass

if __name__ == "__main__":
    import sys
    from twisted.internet import reactor
    
    def print_lines(lines):
        """
        Simple test callback.
        """
        for line in lines:
            sys.stdout.write(line)
            sys.stdout.flush()

    FILENAME = "/tmp/ouch"
    tail = Tail(FILENAME, print_lines)
    tail.start()
    try:
        reactor.run()
    except KeyboardInterrupt:
        print("Exiting.")
        
