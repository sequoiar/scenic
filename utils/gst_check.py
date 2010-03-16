#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (C) 2008 Société des arts technologiques (SAT)
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
#

""" Checks this host to ensure all gstreamer plugins needed by milhouse
are installed """

import glob
import os
import sys
import re

try:
    import pygst
    pygst.require('0.10')
    import gst
except ImportError:
    print("import failed, please install gst-python")
    sys.exit(1)

# Get the full path to cpp files relative to the script location
cwd = os.path.dirname(os.path.realpath(__file__))
cpp_files = glob.glob(os.path.realpath(cwd + "/../src/gst") + "/*.cpp")
if ( len(cpp_files) == 0):
    sys.stderr.write("No cpp files found. Make sure the script is located within source directory \"utils\".")
    sys.exit(2)

""" List of matches
codec_ = Pipeline::Instance()->makeElement("theoraenc", NULL);
if (source_ == "videotestsrc")
if (sink_ == "xvimagesink")
*coder(pipeline, "mad")
"""
matches = [
    re.compile(r"^.*makeElement\(\""), 
    re.compile(r"^.*source_ == \""),
    re.compile(r"^.*sink_ == \""),
    re.compile(r"^.*coder\(pipeline, \""),
]

end = re.compile(r"\".*$")

gst_plugins = []
missing_plugins = []

# Scan for files and retrieve used gst elements
for source_file in cpp_files:
    try:
        f = open(source_file)
        for line in f:
            for m in matches:
                if (m.search(line) is not None):
                    """ 
                    We want to push the element name in the gst_plugins list:
                    1) We strip the line
                    2) We substitute the match with the empty string
                    3) We strip all characters after the double quote    
                    """
                    gst_plugins.append((end.sub("", m.sub("", line.strip()))))

    except IOError, e:
        sys.stderr.write(e)
    finally:
        f.close()

gst_plugins = list(set(gst_plugins))
gst_plugins.sort()

try:
    gst_plugins.remove('sharedvideosink')
except:
    pass

optional_plugins = ["dc1394src", "dv1394src", "alsasrc", "alsasink", "pulsesrc", "pulsesink", "glimagesink", "theoraenc", "theoradec"]

for plugin in gst_plugins:
    if gst.element_factory_find(plugin) is None: 
        print("Error: plugin " + plugin + " is NOT installed")
        missing_plugins.append(plugin)
    else:
        print(plugin + " installed")
print("-------------------------------")
if len(missing_plugins) == 0:
    print("All " + str(len(gst_plugins)) + " necessary plugins installed")
    sys.exit(0)
else:
    print("The following gstreamer plugins need to be installed: ")
    missing_critical = False
    for plugin in missing_plugins:
        print(plugin)
        if plugin not in optional_plugins:
            missing_critical = True
    print("You may have to install the corresponding development headers \
    (i.e. lib<MODULE>-dev)")
    print("before building the missing gstreamer plugins")
    if missing_critical:
        sys.exit(1)
    else:
        sys.exit(0)
