# -*- coding: utf-8 -*-

# Miville
# Copyright (C) 2008 Société des arts technoligiques (SAT)
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


# this is the base class mediator, implemented
# as a template pattern. The main controller should inherit from this.
class Mediator:
    def __init__(self):
        pass

    def colleague_changed(self, colleague, event):
        """Template pattern interface method, intended to be called directly"""
        self._colleague_changed(colleague, event)

    def _colleague_changed(self, colleague, event):
        """Template pattern polymorphic method, intended to be inherited"""
    

# this is the base class colleague, implemented
# as a template pattern. The input classes should inherit from this.
class Colleague:
    def __init__(self, mediator):
        self.mediator = mediator

    def changed(self, colleague, event):
        """Template pattern inteface method, intended to be called directly"""
        self._changed(colleague, event)

    def _changed(self, colleague, event):
        """Template pattern interface method, intended to be inherited"""
        self.mediator.colleague_changed(colleague, event)


class ColleagueExample(Colleague):
    """This takes part in the Mediator/Colleague pattern"""

    def __init__(self, mediator, *_args, **_kwargs):
        # somewhat wacky initialization of parent control
        Colleague.__init__(self, mediator)

    def on_event(self, event):
        self.changed(self, event)
       

# main controller of the application
class MediatorExample(Mediator):
    """The MainFrame of the application"""
    
    def __init__(self):
        Mediator.__init__(self)

    def _colleague_changed(self, colleague, event, *args, **kargs):
        """This method gets called when when of the colleagues has changed.
        This is the central 'clearing' house for all changes and orchestrates
        these changes among the cooperating controls"""

        if event[0] != '_':
            try:
                callback = getattr(self, event)
            except:
                print "The callback %s doesn't exist." % (event)
            if callback:          
                 callback(colleague, *args, **kargs)

    def connect(self, colleague, addr):
        print "Connecting to %s" % (addr)
