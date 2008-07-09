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



       
class Stream:
    """That represent all the settings of a stream to send """
    pass


class LocalStream(Stream):
    """That represent all the settings of a stream to send """

    def __init__(self):
        self.a_codec = ""
        self.v_codec = ""
        self.address = ""
    
    def connect(self, address=""):
        print "Connecting to %s" % (address)
    
    def status(self):
        return (self.a_codec, self.v_codec, self.address)
