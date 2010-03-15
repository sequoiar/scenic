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
from twisted.python import log
from twisted.python import logfile

def start_logging_to_stdout():
    log.startLogging(sys.stdout)

def start_file_logging(full_path="/var/tmp/scenic/scenic.log"):
    """
    Starts logging the Master infos to a file.
    @rettype: str path to the log file
    """
    file_name = os.path.basename(full_path)
    directory = os.path.dirname(full_path)
    if not os.path.exists(directory):
        os.makedirs(directory)
    f = open(full_path, 'w') # erases previous file
    f.close()
    _log_file = logfile.DailyLogFile(file_name, directory)
    log.startLogging(_log_file)
    return _log_file.path

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
    
    if not os.environ.has_key('GTK2_RC_FILES'):
        name = "Darklooks"
        file_name = os.path.join(os.path.join(configure.THEMES_DIR, name, "gtkrc"))
        os.environ["GTK2_RC_FILES"] = file_name # has to be done before gtk2reactor.install()
        configure.custom_environment_variables["GTK2_RC_FILES"] = file_name
    if "/sbin" not in os.environ["PATH"]: # for ifconfig
        os.environ["PATH"] += ":/sbin"
    
    from optparse import OptionParser
    from twisted.internet import gtk2reactor
    gtk2reactor.install() # has to be done before importing reactor
    from twisted.internet import reactor
    from twisted.internet import error
    from scenic import application
    
    # command line parsing
    parser = OptionParser(usage="%prog", version=str(configure.VERSION))
    parser.add_option("-k", "--kiosk", action="store_true", help="Run in kiosk mode")
    parser.add_option("-l", "--enable-logging", action="store_true", help="Enables logging to file.")
    parser.add_option("-L", "--log-file-name", type="string", help="Specifies the path to the log file. Default is %s" % (LOG_FILE_NAME), default=LOG_FILE_NAME)
    parser.add_option("-f", "--fullscreen", action="store_true", help="Run in fullscreen mode")
    parser.add_option("-M", "--moo", action="store_true", \
        help="There is no easter egg in this program.")
    (options, args) = parser.parse_args()
    kwargs = {}
    if options.moo:
        moo()
        sys.exit(0)
    if options.enable_logging:
        start_file_logging(os.path.expanduser(options.log_file_name))
        kwargs["log_file_name"] = options.log_file_name
    else:
        start_logging_to_stdout()
    try:
        app = application.Application(kiosk_mode=options.kiosk, fullscreen=options.fullscreen, **kwargs)
    except error.CannotListenError, e:
        print("There must be an other Scenic running.")
        print(str(e))
    else:
        try:
            reactor.run()
        except KeyboardInterrupt:
            pass
            sys.exit(0)
