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

# System imports
import codecs

def to_utf(obj, encoding='utf-8'):
    """
    Converts a str to the UTF-8 character encoding.
    If already UTF-8, leaves it as it is.
    """
    if isinstance(obj, basestring):
        if not isinstance(obj, unicode):
            obj = unicode(obj, encoding)
    return obj

def open(filename, mode):
    """
    Opens an encoded file using the given mode (read/write) and returns
    a wrapped version providing transparent encoding/decoding in UTF-8.
    """
    return codecs.open(filename, mode, encoding='utf-8')

