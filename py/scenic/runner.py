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
Main of the application.
Some imports are in run().
"""
import sys
import os
from twisted.python import logfile
from optparse import OptionParser
from scenic import logger

log = None

def start_logging_to_stdout(level="warning"):
    global log
    log = logger.start(level=level)

def start_file_logging(level="warning", full_path="/var/tmp/scenic/scenic.log"):
    """
    Starts logging the Master infos to a file.
    @rtype: str path to the log file
    """
    global log
    directory = os.path.dirname(full_path)
    if directory == '':
        directory = os.getcwd()
    if not os.path.exists(directory):
        os.makedirs(directory)
    f = open(full_path, 'w') # erases previous file
    f.close()

    log = logger.start(level=level, to_file=True, log_file_name=full_path)
    return full_path

LOG_FILE_NAME = "~/.scenic/scenic.log"

def moo():
    """
    This image is:
    Copyright (C) 2003, Vijay Kumar
    Permission is granted to copy, distribute and/or modify this image under the terms either:
    
    * the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version, or
    * the GNU Free Documentation License, Version 1.1 or any later version published by the Free Software Foundation; with the no Invariant Sections, with no Front-Cover Texts and with no Back-Cover Texts.
    """
    print("""
    _-`````-,           ,- '- .    
  .'   .- - |          | - .   `.  
 /.'  /                     `.   \ 
:/   :      _...   ..._      ``   :
::   :     /._ .`:'_.._\.    ||   :
::    `._ ./  ,`  :    \ . _.''   .
`:.      |   |  -.  \-. \|_       / 
  \:._ _/  ..'  .@)  \@) ` `\ ,.'  
     _/,--'       .- .\,-.`--`.    
       ,'/''     (( \ `  )     
        /'/'  \    `-'  (      
         '/''  `._,-----'      
          ''/'    .,---'       
           ''/'      ;:            
             ''/''  ''/        
               ''/''/''        
                 '/'/'         
                  `;               
    """)

def run():
    """
    Main function of the application
    """
    from scenic import configure
    
    # command line parsing
    parser = OptionParser(usage="%prog", version=str(configure.VERSION), description=configure.DESCRIPTION)
    parser.add_option("-k", "--kiosk", action="store_true", help="Run in kiosk mode")
    parser.add_option("-l", "--enable-logging", action="store_true", help="Enables logging to file.")
    parser.add_option("-L", "--log-file-name", type="string", help="Specifies the path to the log file. Default is %s. Logging must be enabled for this option value to be useful." % (LOG_FILE_NAME), default=LOG_FILE_NAME)
    parser.add_option("-f", "--fullscreen", action="store_true", help="Run in fullscreen mode")
    parser.add_option("-n", "--disable-v4l2-settings-restoration", action="store_true", help="Disables the state restoring for the V4L2 input number and video standard at startup")
    parser.add_option("-v", "--verbose", action="store_true", help="Enables a verbose logging output with info level messages.")
    parser.add_option("-d", "--debug", action="store_true", help="Enables a very verbose logging output with debug level messages. Also add a debug tab in the user interface.")
    parser.add_option("-M", "--moo", action="store_true", help="There is no easter egg in this program")
    (options, args) = parser.parse_args()
    
    if not os.environ.has_key('DISPLAY'):
        print "You need an X11 display to run Scenic."
        sys.exit(1)
    level = "warning"
    if options.verbose:
        level = "info"
    if options.debug:
        level = "debug"
    if options.enable_logging:
        start_file_logging(level, os.path.expanduser(options.log_file_name))
        log_file_name = options.log_file_name
    else:
        start_logging_to_stdout(level)
    log_file_name = None
    
    from scenic import process
    process.save_environment_variables(os.environ)
    
    if not os.environ.has_key('GTK2_RC_FILES'): # FIXME: is this check needed and desired?
        name = "Darklooks"
        file_name = os.path.join(os.path.join(configure.THEMES_DIR, name, "gtkrc"))
        os.environ["GTK2_RC_FILES"] = file_name # has to be done before gtk2reactor.install()
    if "/sbin" not in os.environ["PATH"]: # for ifconfig
        os.environ["PATH"] += ":/sbin"

    from twisted.internet import gtk2reactor
    gtk2reactor.install() # has to be done before importing reactor
    from twisted.internet import reactor
    from twisted.internet import error
    from scenic import application
    import gtk
    
    try:
        gtk.gdk.Display(os.environ["DISPLAY"])
    except RuntimeError, e:
        msg = "Invalid X11 display: %s. \nYou need an X11 display to run Scenic." % (os.environ["DISPLAY"])
        log.error(msg)
        print msg
        sys.exit(1)
        
    enable_v4l2_state_saving_restoration = True
    if options.moo:
        moo()
        sys.exit(0)
    
    if options.disable_v4l2_settings_restoration:
        enable_v4l2_state_saving_restoration = False
    
    try:
        app = application.Application(kiosk_mode=options.kiosk, fullscreen=options.fullscreen, enable_debug=options.debug, force_previous_device_settings=enable_v4l2_state_saving_restoration, log_file_name=log_file_name)
        app.start()
    except error.CannotListenError, e:
        msg = "There must be an other Scenic running."
        log.error(msg)
        log.error(str(e))
        print(msg)
        print(str(e))
        sys.exit(1)
    else:
        try:
            # starting the application
            reactor.run()
        except KeyboardInterrupt:
            pass
        log.info("Goodbye.")
        sys.exit(0)

