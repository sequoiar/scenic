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

def _parse_v4l2_ctl_all(lines):
    """
    Parses the output of the `v4l2-ctl --all -d /dev/video0` command
    @rettype: dict
    """
    results = dict()
    category = None
    sub_category = None
    pixel_format_is_done = False
    #print lines
    for line in lines:
        is_value = False
        if line.startswith('\t\t'):
            pass # subvalue
        elif line.startswith('\t'):
            # value ------------------------------
            splitted = line.strip('\t').split(':')
            try:
                key = splitted[0].strip()
                value = splitted[1].strip()
                is_value = True
                #print "key is "+key+" and value is "+ value
            except IndexError:
                pass
            if is_value:
                if key == 'Driver name':
                    results['driver'] = value
                elif key == 'card type':
                    results['card'] = value
                elif key == 'Pixel Format' and not pixel_format_is_done:
                    results['pixel format'] = value
                    pixel_format_is_done= True
                elif key == 'Width/Height' and category.startswith('Format Video Capture'):
                    #log.debug("parsing width/height in category %s/%s" % (category, sub_category))
                    dimen = value.split('/')
                    results['width'] = dimen[0]
                    results['height'] = dimen[1]
                #elif key == 'Video input':
                #    results['input'] = value.split('(')[1].split(')')[0] # 0 (Composite0)
                    # log.debug('V4L2 input: ' + results['input'])
                    # TODO : possibilities are Composite0, Composite1, 
                # norm
            elif category == "Video Standard":
                norm = line.strip('\t').strip('\n')
                if norm.startswith('NTSC-'):
                    norm = 'ntsc'
                elif norm.startswith('PAL-'):
                    norm = 'pal'
                elif norm.startswith('SECAM-'):
                    norm = 'secam'
                results['norm'] = norm
        elif line.find("Video input") == 0:
            try:
                results['input'] = line.split('(')[1].split(')')[0]
            except IndexError, e:
                # Sometimes you don't have parentheses for Video input: x
                log.error("_parse_v4l2_ctl_all: " + e.message)
                results['input'] = line.split(':')[1].strip()
            # log.debug('V4L2 input: ' + results['input'])
            # TODO : possibilities are Composite0, Composite1, 
        elif line.find(":") > 0:
            try:
                category = line.split(':')[0]
            except IndexError:
                pass
        elif line.find("Video Standard") == 0:
            category  = "Video Standard"
    #pprint.pprint(results)
    return results

def _parse_v4l2_ctl_list_inputs(lines):
    """
    Parses the output of `v4l2-ctl --list-inputs -d /dev/video0`
    
    Returns a list with all input names.
    Their index is their number.
    """
    inputs = []
    for line in lines:
        if line.startswith('\t'):
            splitted = line.strip('\t').split(':')
            try:
                key = splitted[0].strip()
                value = splitted[1].strip()
            except IndexError:
                pass
            else:
                if key == 'Name':
                    inputs.append(value.strip())
    # log.debug('v4l2 inputs: %s' % (inputs))
    return inputs

def list_v4l2_cameras():
    """
    @rettype Deferred
    """
    commands.append(['v4l2-ctl', '--all', '-d', name])
    commands.append(['v4l2-ctl', '--list-inputs', '-d', name])
    
def set_v4l2_video_standard(device_name="/dev/video0", norm="ntsc"):
    """
    Norm
    """
    command = ['v4l2-ctl', '--set-standard=%s' % (norm), '-d', device_name]
    raise NotImplementedError('to do')

def set_v4l2_input_number(device_name="/dev/video0", input_number=0):
    command = ['v4l2-ctl', '--set-input=' + str(input_number), '-d', device_name]
    raise NotImplementedError('to do')
