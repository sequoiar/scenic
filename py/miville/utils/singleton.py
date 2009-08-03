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
# along with Miville. If not, see <http://www.gnu.org/licenses/>.

# TODO: remove this file. This is useless in python (module variables are the way to go)

class Singleton (object):
    def __new__(klass, *args, **kwargs): 
        if not hasattr(klass, 'self'):
            klass.self = object.__new__(klass)
        return klass.self

if __name__ == '__main__':
    print "starting example:"
    class Some(Singleton):
        def __init__(self):
            pass
            print 'New Some object.'
    a = Some()
    b = Some()
    print a
    print b
    if a is b:
        print "SUCCESS"
    else:
        raise Exception,"a and b are not the same instance."
