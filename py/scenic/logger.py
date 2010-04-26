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
Python logging utility.

Wraps the logging module and Twisted's python.log module.
Now with non-blocking output
"""
import logging
import sys
import twisted # for its version 
import twisted.python.log as twisted_log
from twisted.internet import fdesc

# Deals with older version of twisted (in Ubuntu 8.04 and other)
# We need to add the class PythonLoggingObserver to the twisted.python.log module.
version = int(twisted.__version__.split('.')[0])
if version < 8:
    from scenic.twistedbackports import PythonLoggingObserver
    twisted_log.PythonLoggingObserver = PythonLoggingObserver

#TODO: Specify the level by output
ENABLE_NON_BLOCKING_OUTPUT = False
#LoggerClass = logging.getLoggerClass()

# XXX: totally useless.
#class CoreLogger(LoggerClass):
#    """
#    Overrides the logger class in tyhe logging module.
#    """
#    def debug(self, msg=None, *args):
#        # if there is no msg get the name of method/function
#        if not msg:
#            msg = "def: %s" % sys._getframe(2).f_code.co_name
#        LoggerClass.debug(self, msg, *args)
#
#logging.setLoggerClass(CoreLogger)

#logging.setLoggerClass(LoggerClass)

def start(level="info", to_stdout=True, to_file=True, name="twisted", log_file_name="/tmp/default-log-file.log"):
    """
    Starts the logging for a single module.
    
    Each module should import this logging module and decide its level.
    
    The first time this is called, don't give any argument. It will log everything with the name "twisted".
    
    The programmer can choose the level from which to log.
    Example : is level is INFO, the DEBUG messages (lower level) will not be displayed
    but the CRITICAL ones will.
    
    @param level: debug, info, error, warning or critical
    @type level: str
    @param to_stdout: Whether it should be printed to stdout.
    @param to_file: Whether it should be printed to file.
    @param name: What string to prefix with.
    """
    logger = logging.getLogger(name)
    formatter = logging.Formatter('%(asctime)s %(name)-8s %(levelname)-8s %(message)s')
    set_level(level, name)
    if to_stdout:
        so_handler = logging.StreamHandler(sys.stdout)
        if ENABLE_NON_BLOCKING_OUTPUT: 
            fdesc.setNonBlocking(so_handler.stream) # NON-BLOCKING OUTPUT
        so_handler.setFormatter(formatter)
        logger.addHandler(so_handler)
    if to_file:
        # file_handler = logging.FileHandler(log_file_name, mode='a', encoding='utf-8')
        file_handler = logging.FileHandler(log_file_name) # not catching IOError that could occur.
        if ENABLE_NON_BLOCKING_OUTPUT: 
            fdesc.setNonBlocking(file_handler.stream) # NON-BLOCKING OUTPUT
        file_handler.setFormatter(formatter)
        logger.addHandler(file_handler)
    if name == 'twisted':
        observer = twisted_log.PythonLoggingObserver(name)
        observer.start()
    else:
        return logging.getLogger(name)

def stop():
    """
    Stops logging for a single module.
    """
    logging.shutdown()

def set_level(level, logger='twisted'):
    """
    Sets the logging level for a single file. 
    """
    levels = {
        'critical':logging.CRITICAL, # 50
        'error':logging.ERROR, # 40
        'warning':logging.WARNING, # 30
        'info':logging.INFO, # 20
        'debug':logging.DEBUG, # 10
        }        
    logger = logging.getLogger(logger)
    if level in levels:
        logger.setLevel(levels[level])
    else:
        raise RuntimeError("%s is not a valid log level." % (level)) #ERR ?

def critical(msg):
    """
    Logs a message with CRITICAL level. (highest)
    """
    twisted_log.msg(msg, logLevel=logging.CRITICAL)

def error(msg):
    """
    Logs a message with ERROR level. (2nd) 
    """
    twisted_log.msg(msg, logLevel=logging.ERROR)

def warning(msg):
    """
    Logs a message with WARNING level. (3rd)
    """
    twisted_log.msg(msg, logLevel=logging.WARNING)

def info(msg):
    """
    Logs a message with INFO level. (4th)
    """
    twisted_log.msg(msg)

def debug(msg): 
    """
    Logs a message with DEBUG level. (5th and last level)
    """
    twisted_log.msg(msg, logLevel=logging.DEBUG)
       
if __name__ == "__main__":
    # Here is a simple test:
    start('warning', True, True)
    critical('critical1é')
    error('error1')
    warning('warning1')
    info('info1')
    debug('debug1')
    set_level('debug')
    critical('critical2')
    error('error2')
    warning('warning2')
    info('info2')
    debug('debug2')
    try:
        set_level('asd')
    except RuntimeError, e:
        print e.message
    stop()
    
