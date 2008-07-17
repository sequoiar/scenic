# -*- coding: utf-8 -*-

# Sropulpof
# Copyright (C) 2008 Société des arts technoligiques (SAT)
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


import weakref, sys

def get_def_name(level=2):
    return sys._getframe(level).f_code.co_name


class Observer(object):
    """Instances of Observer are notified of changes happening in the
    Subject instances they are attached to.
    It's possible for an Observer to watch many Subject.
    """
    
    def __init__(self, subjects):
        self.subjects = []
        if isinstance(subjects, tuple):
            for subject in subjects:
                self.append(subject)
        else:
            self.append(subjects)
        
    def append(self, subject):
        if isinstance(subject, Subject):
            self.subjects.append(subject)        
            subject._attach(self)   # should we make an excepption/error message
                                    # if subject isn't a Subject instance ?

    def update(self, origin, key, value):
        """Called when an attribute of the observed object is changed.
        Should be overridden.

        @param origin:    observed object for which the attribute is changed
        @type origin:     any
        @param key:       attribute changed
        @type key:        string
        @param value:     value of the attribute after being set
        @type value:      any
        """
        pass
       
       
class Subject(object):
    """
    """
    
    def __init__(self):
        self.observers = weakref.WeakValueDictionary()

    def _attach(self, observer):
        ob_id = id(observer)
        if ob_id not in self.observers:
            self.observers[ob_id] = observer
                    
    def notify(self, caller, value, key=None):
        if not key:
            key = get_def_name()
        for observer in self.observers.itervalues():
            observer.update(caller, key, value)
            
            