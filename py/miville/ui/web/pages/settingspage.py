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


##System imports
#import os.path as path
#import os
#from xml.etree import ElementTree
#
##Twisted imports
#from twisted.internet import reactor
#from twisted.python.modules import getModule

try:
#    from nevow import loaders, appserver, static, tags, inevow
#    from nevow.athena import LivePage, LiveFragment, expose as nevow_expose
#    from nevow.i18n import render as i18nrender
#    from nevow.i18n import _, I18NConfig
    from nevow import rend
except ImportError:
    raise ImportError, 'If you want to use the Web interface, you need to install Nevow.'

#App imports
from miville.utils import Observer, log
from miville.utils.i18n import to_utf
from miville.utils.common import find_callbacks

from miville.settings import global_settings
from miville.settings import media_settings
from miville.settings import Settings
from miville.errors import *


log = log.start('debug', 1, 0, 'web')

def print_all_media_settings(media_settings):
    txt = "<h1>All MEDIA SETTINGS...</h1>"
    txt += "<ul>"
    for k, v in media_settings.iteritems():
        txt += "<li> [" + str(k) + "] " + v.name + "</li>"
        txt += "<ul>"
        for key, value in v.settings.iteritems():
            txt += "<li>  %s : %s</li>" % (key, str(value))
        txt += "</ul>"
    txt += "</ul>"
    return txt    

def print_media_setting(media_setting):
    txt = "<li>[" + str(media_setting.id) + "] media setting \"" + media_setting.name + "\"</li>"
    txt += "<ul>"
    for key, value in media_setting.settings.iteritems():
        txt += "<li>%s : %s</li>" % (key, str(value))
    txt += "</ul>"
    return txt

def print_stream(stream):
    txt = " <li>[" + stream.name + "] stream</li>"
    txt += "<ul>"
    txt += "    <li>enabled: " + str(stream.enabled) + "</li>"
    txt += "    <li>sync: " + stream.sync_group      + "</li>"
    txt += "    <li>port: " + str(stream.port)       + "</li>"
    txt += "</ul>"
    #txt += "<p>    media setting: %s\n" % str(stream.setting)
    return txt

def print_stream_sub_group(group_id, group):
    txt = " <li>[" + str(group_id) + "] stream sub group " + group.name + "</li>"
    txt += "<ul>"
    txt += "<li>enabled: " + str(group.enabled) + "</li>"
    txt += "<li>mode: " + str(group.mode) + "</li>"
    txt += "</ul>"
    return txt

def print_global_setting(global_setting_id, global_setting):
    txt = "<li>[" + str(global_setting_id) + "] global setting  " + global_setting.name + "</li>"
    return txt
    
def print_settings():

    txt = "<h1>GLOBAL SETTINGS:</h1>"
    txt += '<ul id="adoptme">'
    for k, global_setting in global_settings.iteritems():
        
        txt += print_global_setting(k, global_setting)
        for group_id,group in global_setting.stream_subgroups.iteritems():
            txt += "<ul>"
            txt += print_stream_sub_group(group_id, group)
            for stream in group.media_streams:
                txt += print_stream(stream)                
                try:
                    id = stream.setting
                    media_setting = Settings.get_media_setting_from_id(id)
                    txt += print_media_setting(media_setting)
                except:
                    txt += "<p>     media setting %s (not available)</p>" % stream.setting
            txt += "</ul>"
    txt += "</ul>"                
    txt += print_all_media_settings(media_settings)
    return txt


def get_doc():
    
    head = """
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" version="-//W3C//DTD XHTML 1.1//EN" xml:lang="en">

<head>

<title>mooTree 2</title>

<meta http-equiv="content-type" content="text/html; charset=iso-8859-1" />
<link rel="stylesheet" href="mootree.css" type="text/css" media="screen" />

<script type="text/javascript" src="mootools/mootools-1.2-core.js"></script>
<script type="text/javascript" src="mootree.js"></script>

<script type="text/javascript">

var tree;

window.onload = function() {
    
    tree = new MooTreeControl({
        div: 'mytree',
        mode: 'folders',
        grid: true,
        onSelect: function(node, state) {
            if (state) window.alert('url: ' + node.data.url);
        }
    },{
        text: 'Root Node',
        open: true
    });
    
}


</script>

</head>

<body>

<h2>mooTree 2</h2>
<h4>example 2: adopting a tree from a structure of ul/li elements</h4>

<p>
    This demonstrates adoption of a tree from a structure of ul/li elements
    in your document.
</p>

<p>

    Note that the properties of the a-tag in each li element are stored in the
    node's data-object with the same name - for example, data.href contains
    the href attribute.
</p>
<p>
    The onSelect event of the tree in this example simply displays the url in an
    alert, but you would probably want to, for example, set the location.href of
    an iframe, or do some AJAX operation, or something else.
</p>

<p id="hideme">
    <a href="#" onclick="$('hideme').style.display='none'; tree.adopt('adoptme'); return false">click here</a> to adopt the structure below...
</p>

<div id="mytree">
</div>

<p>
    <input type="button" value=" expand all " onclick="tree.expand()" />
    <input type="button" value=" collapse all " onclick="tree.collapse()" />
</p>
"""

    toes = """


</body>

</html>

"""
    return head, toes

class SettingsPage(rend.Page):
    def renderHTTP(self, ctx):
        
        beau_body =  print_settings()
        
#        head, footer = get_doc()
#        html = head
        
        html = u"""
        <html>
        <head></head>
        <body>
        
            """
        
        html += beau_body     
#        html += footer
        html += u"""
        
        </body>
        </html>
        """ 
        return str(html)


    
    
