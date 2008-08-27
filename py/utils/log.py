#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Sropulpof
# Copyright (C) 2008 Société des arts technoligiques (SAT)
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


# System imports
import logging
import sys

# Twisted imports
import twisted.python.log as tw_log
import twisted
version = int(twisted.__version__.split('.')[0])
if version < 8:
    from utils.twisted_old import PythonLoggingObserver
    tw_log.PythonLoggingObserver = PythonLoggingObserver


def start(level='info', to_stdout=1, to_file=0, log_name='twisted'):
    log_file = 'miville.log'
    logger = logging.getLogger(log_name)
    formatter = logging.Formatter('%(asctime)s %(name)-8s %(levelname)-8s %(message)s')
    set_level(level, log_name)
    if to_stdout:
        so_handler = logging.StreamHandler(sys.stdout)
        so_handler.setFormatter(formatter)
        logger.addHandler(so_handler)
    if to_file:
#        file_handler = logging.FileHandler(log_file, mode='a', encoding='utf-8')
        file_handler = logging.FileHandler(log_file)
        file_handler.setFormatter(formatter)
        logger.addHandler(file_handler)
    if log_name == 'twisted':
        observer = tw_log.PythonLoggingObserver(log_name)
        observer.start()
    else:
        return logging.getLogger(log_name)

def stop():
    logging.shutdown()

def set_level(level, logger='twisted'):
    levels = {'critical':logging.CRITICAL,  # 50
              'error':logging.ERROR,        # 40
              'warning':logging.WARNING,    # 30
              'info':logging.INFO,          # 20
              'debug':logging.DEBUG}        #10
    logger = logging.getLogger(logger)
    if level in levels:
        logger.setLevel(levels[level])
    else:
        print "%s it's not a valid log level." % (level) #ERR ?

def critical(msg):
    tw_log.msg(msg, logLevel=logging.CRITICAL)

def error(msg):
    tw_log.msg(msg, logLevel=logging.ERROR)

def warning(msg):
    tw_log.msg(msg, logLevel=logging.WARNING)

def info(msg):
    tw_log.msg(msg)
#    logging.info(msg)

def debug(msg):
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
    set_level('asd')
    stop()
    
