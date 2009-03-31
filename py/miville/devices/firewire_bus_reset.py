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
Firewire bus reset utility.

Make sure to install firecontrol in /usr/local/bin
See https://svn.sat.qc.ca/trac/miville/wiki/AudioVideo

To install firecontrol : 

sudo apt-get install libraw1394-dev

cd src/
svn checkout https://firecontrol.svn.sourceforge.net/svnroot/firecontrol/trunk firecontrol
cd firecontrol
./configure
make 
sudo make install 

sudo modprobe raw1394
sudo gpasswd -a `whoami` disk
"""

from ctypes import cdll, c_int, c_void_p
# constants from libraw1394.h
RAW1394_LONG_RESET = 0
RAW1394_SHORT_RESET = 1

# exporting the functions
libraw = cdll.LoadLibrary("libraw1394.so")

new_handle = libraw.raw1394_new_handle
new_handle.res_type = c_void_p

reset_bus_new = libraw.raw1394_reset_bus_new

loop_iterate = libraw.raw1394_loop_iterate

def reset_firewire_bus():
    """
    Resets the raw1394 bus.
    """
    global libraw
    print "libraw1394.so : ", libraw
    print "function", new_handle
    handle = new_handle()
    if handle == 0:
        print 'Failed to get raw1394 handle.'
        print '  Are you member of the '
    else:
        print "handle: (pointer to a struct) ", handle, type(handle)
        result = reset_bus_new(handle, c_int(RAW1394_LONG_RESET))
        if result == -1:
            print "Failure when trying to reset firewire bus."
        else:
            print "Seems ok so far"
            print "results: ", result
            loop_iterate(handle)

# -----------------------------------------------
# struct raw1394_handle {
#         int fd;
#         int protocol_version;
#         unsigned int generation;
# 
#         nodeid_t local_id;
#         int num_of_nodes;
#         nodeid_t irm_id;
# 
#         raw1394_errcode_t err;
#         void *userdata;
# 
#         bus_reset_handler_t bus_reset_handler;
#         tag_handler_t     tag_handler;
#         arm_tag_handler_t arm_tag_handler;
#         fcp_handler_t     fcp_handler;
#         iso_handler_t     iso_handler[64];
# 
#     /* new ISO API */
# 
#     /* memory mapping of the DMA buffer */
#     unsigned char *iso_buffer;
#     enum { ISO_INACTIVE, ISO_XMIT, ISO_RECV } iso_mode;
#     enum { ISO_STOP, ISO_GO } iso_state;
# 
#     /* iso XMIT only: */
#     unsigned int iso_buf_stride; /* offset between successive packets */
#     unsigned int next_packet; /* index of next packet to be transmitted */
# 
#     /* status buffer, updated from _raw1394_iso_iterate() */
#     struct raw1394_iso_status iso_status;
#     unsigned int iso_packets_dropped;
# 
#     /* user-supplied handlers */
#     raw1394_iso_xmit_handler_t iso_xmit_handler;
#     raw1394_iso_recv_handler_t iso_recv_handler;
# 
#     quadlet_t buffer[HBUF_SIZE/4]; /* 2048 */
#     void *iso_packet_infos; /* actually a struct raw1394_iso_packet_info* */
# };
# -----------------------------------------------
# typedef struct raw1394_handle *raw1394handle_t;
# -----------------------------------------------
# /**
#  * raw1394_new_handle - create new handle
#  *
#  * Creates and returns a new handle which can (after being set up) control one
#  * port.  It is not allowed to use the same handle in multiple threads or forked
#  * processes.  It is allowed to create and use multiple handles, however.  Use
#  * one handle per thread which needs it in the multithreaded case.
#  *
#  * The default device node is /dev/raw1394, but one can override the default
#  * by setting environment variable RAW1394DEV. However, if RAW1394DEV points to
#  * a non-existant or invalid device node, then it also attempts to open the
#  * default device node.
#  *
#  * Returns: the created handle or %NULL when initialization fails. In the latter
#  * case errno either contains some OS specific error code or EPROTO if
#  * libraw1394 and raw1394 don't support each other's protocol versions.
#  **/
# raw1394handle_t raw1394_new_handle(void);
# -----------------------------------------------
# /**
#  * raw1394_reset_bus_new - Reset the connected bus (with certain type). 
#  * @handle: libraw1394 handle
#  * @type: RAW1394_SHORT_RESET or RAW1394_LONG_RESET
#  *
#  * Returns: %0 for success or -1 for failure 
#  **/
# int raw1394_reset_bus_new(raw1394handle_t handle, int type);
# -----------------------------------------------

if __name__ == '__main__':
    print "Will reset the firewire bus."
    reset_firewire_bus()


