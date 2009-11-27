#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# ToonLoop for Python
#
# Copyright 2008 Alexandre Quessy & Tristan Matthews
# http://toonloop.com
# 
# Original idea by Alexandre Quessy
# http://alexandre.quessy.net
#
# ToonLoop is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# ToonLoop is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the gnu general public license
# along with ToonLoop.  If not, see <http://www.gnu.org/licenses/>.
#
"""
State saving utilies.

Easily readable and editable.
Support objects as well as simpler data types.

The original file is : toonloop/trunk/py/rats/serialize.py
"""
from twisted.spread import jelly
import pprint
import os
import stat

import warnings
try:
    import json # python 2.6
except ImportError:
    import simplejson as json # python 2.4 to 2.5
try:
    _tmp = json.loads
except AttributeError:
    warnings.warn("Use simplejson, not the old json module.")
    sys.modules.pop('json') # get rid of the bad json module
    import simplejson as json

PERMISSIONS = stat.S_IRUSR | stat.S_IWUSR # chmod 600

class SerializeError(Exception):
    """
    Error occuring while trying to save serialized data.
    """
    pass

class UnserializeError(Exception):
    """
    Error occuring while trying to load serialized data.
    """
    pass

class Serializable(jelly.Jellyable):
    """
    Any class that is serializable using these tools
    should extend this one.
    """
    pass

def save(filename, obj):
    """
    Saves any python data type to a file.

    Might throw an SerializeError
    """
    global _verbose
    li = jelly.jelly(obj)
    try:
        f = open(filename, 'w')
        f.write(pprint.pformat(li))
        f.close()
        os.chmod(filename, PERMISSIONS)
    except IOError, e:
        raise SerializeError(e.message)
    except OSError, e:
        raise SerializeError(e.message)

def load(filename):
    """
    Loads any python data type from a file.

    Might throw an UnserializeError
    """
    try:
        f = open(filename, 'r')
        li = eval(f.read()) # do this only on trusted data !
        f.close()
    except IOError, e:
        raise UnserializeError(e.message)
    except OSError, e:
        raise UnserializeError(e.message)
    try:
        obj = jelly.unjelly(li)
    except jelly.InsecureJelly, e:
        raise UnserializeError(e.message)
    except AttributeError, e:
        raise UnserializeError(e.message)
    else:
        return obj

# TODO: get rid of jelly, replace with json

def json_encode(data):
     """
     Python dict/list/int/str/float to json.
     """
     try:
        encoded_data = json.dumps(data)
     except TypeError, e:
         raise SerializeError("Error trying to encode to JSON:%s" % (e.message))
     return encoded_data


def json_decode(encoded_data):
     """
     JSON to Python dict/list/int/str/float.
     """
     try:
        data = json.loads(encoded_data)
     except ValueError, e:
         raise UnserializeError("Error trying to decode from JSON:%s" % (e.message))
     return data

def serialize_json(filename, data):
    """
    Python dict/list/int/str/float to file.
    """
    try:
        f = open(filename, "w")
        json.dump(data, f, indent=4)
    except IOError, e:
        raise SerializeError("Error trying to write file %s:%s" % (filename, e.message))
    except TypeError, e:
        raise SerializeError("Error trying to encode to a JSON serialized object:%s" % (e.message))
    finally:
        f.close()

def unserialize_json(filename):
    """
    File to Python dict/list/int/str/float.
    """
    try:
        f = open(filename, 'r')
        data = json.load(f)
    except IOError, e:
        raise UnserializeError("Error trying to read file %s:%s" % (filename, e.message))
    except ValueError, e:
        raise UnserializeError("Error trying to decode JSON serialized object.")
    finally:
        f.close()
    
    return data

