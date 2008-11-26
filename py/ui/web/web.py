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
from twisted.python.filepath import FilePath
from nevow import rend, loaders, appserver, static, tags
from nevow.athena import LivePage, LiveElement, expose

#App imports
from utils import Observer, log
from utils.i18n import to_utf
import ui
from ui.common import find_callbacks
from streams import audio, video, data
from streams.stream import AudioStream, VideoStream, DataStream

sibling = FilePath(__file__).sibling


log = log.start('info', 1, 0, 'web')

class Base(LivePage, Observer):
    addSlash = True
    docFactory = loaders.xmlfile('ui/web/base.html')

    child_css = static.File('ui/web/css/')
    child_js = static.File('ui/web/js/')
    child_img = static.File('ui/web/img/')

    def __init__(self, subject, controller=None, *arg, **karg):
        Observer.__init__(self, subject)
        self.subject = subject
        self.api = subject.api
        self.controller = controller
        LivePage.__init__(self)
        # Update the mapping of known JavaScript modules so that the
        # client-side code for this example can be found and served to the
        # browser.
        self.jsModules.mapping[u'AddressBook'] = '/Users/etienne/Documents/propulesart/miville/trunk/py/ui/web/js/addressbook.js'
        
    def render_addressbook(self, ctx, data):
        """
        Replace the tag with a new AddressBook element.
        """
        adb = AddressBook()
        adb.setFragmentParent(self)
        return adb

    # add this to send a new Base instance on browser refresh and multiple connections
    def child_ (self, ctx):
        return Base(self.subject)

    def update(self, origin, key, data):
        for children in self.liveFragmentChildren:
            if hasattr(children, 'callbacks') and key in children.callbacks:
                children.callbacks[key](origin, data)
        

class AddressBook(LiveElement):
    """
    """
    docFactory = loaders.xmlfile('ui/web/addressbook.html')
    jsClass = u"AddressBook.List"
    
    def __init__(self):
        LiveElement.__init__(self)
        self.callbacks = find_callbacks(self)

    def _get_contacts(self, origin, data):
        adb = []
        contacts = data.items()
        contacts.sort()
        for name, contact in contacts:
            if name != '_selected':
                adb.append((contact.name, to_utf(contact.address), contact.port))
        log.info('receive update: %r' % self)
        print adb
        self.callRemote('updateList', adb)




def start(subject, port=8080):
    site = appserver.NevowSite(Base(subject))
    reactor.listenTCP(port, site, 5, '127.0.0.1')
    
    
    
    