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

from twisted.internet import reactor
from nevow import rend, loaders, appserver

#App imports
from utils import Observer, log
from utils.i18n import to_utf
import ui
from ui.common import find_callbacks
from streams import audio, video, data
from streams.stream import AudioStream, VideoStream, DataStream


log = log.start('info', 1, 0, 'web')

class HelloWorld(rend.Page):
    addSlash = True
    docFactory = loaders.xmlstr("""
    <html><head><title>Nested Maps Sequence Rendering</title></head>
        <body>Allo
        </body>
    </html>
    """)


def start(subject, port=8080):
    site = appserver.NevowSite(HelloWorld())
    reactor.listenTCP(port, site, 5, '127.0.0.1')