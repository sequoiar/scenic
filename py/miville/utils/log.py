#!/usr/bin/env python
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
"""
Python logging utility.

Wraps the logging module and Twisted's python.log module.
Now with NON-BLOCKING OUTPUT
"""
# System imports
import logging
import sys

# Twisted imports
import twisted.python.log as tw_log
import twisted
from twisted.internet import fdesc
# deal with older version of twisted (in Ubuntu and other)
version = int(twisted.__version__.split('.')[0])
if version < 8:
    from miville.utils.twisted_old import PythonLoggingObserver
    tw_log.PythonLoggingObserver = PythonLoggingObserver

# App import
import miville.utils.common
from miville.errors import InstallFileError

#TODO: Specified the level by output

ENABLE_NON_BLOCKING_OUTPUT = False

LoggerClass = logging.getLoggerClass()

class CoreLogger(LoggerClass):
    """
    Overrides the logger class in tyhe logging module.
    """
    def debug(self, msg=None, *args):
        # if there is no msg get the name of method/function
        if not msg:
            msg = "def: %s" % sys._getframe(2).f_code.co_name
        LoggerClass.debug(self, msg, *args)

logging.setLoggerClass(CoreLogger)

def start(level='info', to_stdout=True, to_file=False, log_name='twisted'):
    """
    Starts the logging for a single module.

    The programmer can choose the level from which to log.
    Example : is level is INFO, the DEBUG messages (lower level) will not be displayed
    but the CRITICAL ones will.
    """
    logger = logging.getLogger(log_name)
    formatter = logging.Formatter('%(asctime)s %(name)-8s %(levelname)-8s %(message)s')
    set_level(level, log_name)
    if to_stdout:
        so_handler = logging.StreamHandler(sys.stdout)
        if ENABLE_NON_BLOCKING_OUTPUT: 
            fdesc.setNonBlocking(so_handler.stream) # NON-BLOCKING OUTPUT
        so_handler.setFormatter(formatter)
        logger.addHandler(so_handler)
    if to_file:
        try:
            log_file = miville.utils.common.install_dir('miville.log')
        except InstallFileError, err:
            print "Logging module ERROR\t%s" % err
        else:
    #        file_handler = logging.FileHandler(log_file, mode='a', encoding='utf-8')
            file_handler = logging.FileHandler(log_file)
            if ENABLE_NON_BLOCKING_OUTPUT: 
                fdesc.setNonBlocking(file_handler.stream) # NON-BLOCKING OUTPUT
            file_handler.setFormatter(formatter)
            logger.addHandler(file_handler)
    if log_name == 'twisted':
        observer = tw_log.PythonLoggingObserver(log_name)
        observer.start()
    else:
        return logging.getLogger(log_name)

def stop():
    """
    Stops logging for a single module.
    """
    logging.shutdown()

class LogError(Exception):
    """
    Any error that log can throw
    """
    pass

def set_level(level, logger='twisted'):
    """
    Sets the logging level for a single file. 
    """
    levels = {'critical':logging.CRITICAL,  # 50
              'error':logging.ERROR,        # 40
              'warning':logging.WARNING,    # 30
              'info':logging.INFO,          # 20
              'debug':logging.DEBUG}        #10
    logger = logging.getLogger(logger)
    if level in levels:
        logger.setLevel(levels[level])
    else:
        raise LogError("%s is not a valid log level." % (level)) #ERR ?

def critical(msg):
    """
    Logs a message with CRITICAL level. (highest)
    """
    tw_log.msg(msg, logLevel=logging.CRITICAL)

def error(msg):
    """
    Logs a message with ERROR level. (2nd) 
    """
    tw_log.msg(msg, logLevel=logging.ERROR)

def warning(msg):
    """
    Logs a message with WARNING level. (3rd)
    """
    tw_log.msg(msg, logLevel=logging.WARNING)

def info(msg):
    """
    Logs a message with INFO level. (4th)
    """
    tw_log.msg(msg)
#    logging.info(msg)

def debug(msg): 
    """
    Logs a message with DEBUG level. (5th and last level)
    """
    tw_log.msg(msg, logLevel=logging.DEBUG)
       
if __name__ == "__main__":
    start('warning', 1, 1)
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
    except LogError, e:
        print e.message
    stop()
    
