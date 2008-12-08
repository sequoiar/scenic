#!/usr/bin/env python

import gobject
import os
import time
import sys
import atexit
import gtk


def fill_env():
    return [key + '=' + value for key, value in os.environ.items()]

def proc_watch(*args):
    print "proc_watch"
    return False

def atexit_handler():
    print "\nhandler\n"
    gobject.source_remove(my_watch)

atexit.register(atexit_handler)



host = '10.10.10.65'
cmd = [ '/usr/local/bin/pof',  '--receiver',  '--address', host, '--videocodec', 'h264', '--audiocodec', 'raw', '--videoport', '8000', '--audioport', '8010' ]

pid, stdin, stdout, stderr = gobject.spawn_async(cmd,
                                                    envp = fill_env(),
                                                    working_directory = os.environ['PWD'],
                                                    flags = gobject.SPAWN_SEARCH_PATH ,
                                                    standard_input = False,         # Could be to true, we just don't need it
                                                    standard_output = True,
                                                    standard_error = True)

my_watch = gobject.io_add_watch(    stdout,
                                    gobject.IO_HUP,
                                    proc_watch )


gtk.main()
