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
This module is an html page for settings information


"""

import commands

try:
    from nevow import rend
except ImportError:
    raise ImportError, 'If you want to use the Web interface, you need to install Nevow.'

#App imports
from miville.utils import Observer, log
from miville.utils.i18n import to_utf
from miville.utils.common import find_callbacks
from miville.errors import *


log = log.start('debug', 1, 0, 'procspage')


def print_head():
    txt=u"""
<head>
<style type="text/css">
h1 {
    color: #00ff00
    font-family: courier
    font-size: 100%
}
p {font-family: courier}
h2 {
    color: #dda0dd
    font-family: courier
    text-indent: 1cm
    font-size: 100%
}
p {
    color: rgb(0,0,255)
    font-family: courier
    text-indent: 3cm
    font-size: 50%
}
h3 {
    font-family: courier
    text-indent: 2cm
    font-size: 100%
}
</style>
</head>
"""
    return txt

def doit(cmd):
    print cmd
    status = commands.getstatusoutput(cmd)
    output = status[1]
    return output.split('\n')


def ps(cmd):
    
    lines = []
    lines.append('<b>')
    lines.append(cmd)
    lines.append('</b>')
    pslines = doit(cmd)
    
    for line in pslines:
        if line.find('sh -c { ps ax |') == -1:
            lines.append(line)
    
    lines.append('<p></p>')
    lines.append('<p>==========</p>')
    lines.append('<p></p>')
    
    return lines

def print_procs():

    s = "<p>"
    lines = []
    
    lines += ps('ps ax | grep miville')
    lines += ps('ps ax | grep milhouse')
    lines += ps('ps ax | grep iperf')
    lines += ps('ps ax | grep jack')
    s += "</p><p>".join(lines) + "</p>"    
    return s

class ProcsPage(rend.Page):
    def renderHTTP(self, ctx):
        square_head = print_head()
        beau_body =  print_procs()
           
        html = u"""
        <html>
        """
        html += square_head 
        html += u"""<body>
        
            """
        html += beau_body     
        
        html += u"""
        
        </body>
        </html>
        """ 
        return str(html)


    
    
