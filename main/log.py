#!/usr/bin/env python


# System imports
import logging
import sys

# Twisted imports
import twisted.python.log as tw_log


def start(level='info', to_stdout=1, to_file=0):
    log_file = 'miville.log'
    logger = logging.getLogger('twisted')
    formatter = logging.Formatter('%(asctime)s %(levelname)s %(message)s')
    set_level(level)
    if to_stdout:
        so_handler = logging.StreamHandler(sys.stdout)
        so_handler.setFormatter(formatter)
        logger.addHandler(so_handler)
    if to_file:
        file_handler = logging.FileHandler(log_file)
        file_handler.setFormatter(formatter)
        logger.addHandler(file_handler)
    observer = tw_log.PythonLoggingObserver()
    observer.start()   

def stop():
    logger = logging.getLogger('twisted')
    logger.stop()    

def set_level(level):
    levels = {'critical':logging.CRITICAL,
              'error':logging.ERROR,
              'warning':logging.WARNING,
              'info':logging.INFO,
              'debug':logging.DEBUG}
    logger = logging.getLogger('twisted')
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

def debug(msg):
    tw_log.msg(msg, logLevel=logging.DEBUG)


       
if __name__ == "__main__":
    start('warning', 1, 1)
    critical('critical1')
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
    
