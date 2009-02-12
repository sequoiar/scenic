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

#System imports
import os.path as path
import os
from xml.etree import ElementTree
import glob
from pprint import pprint
import new

#Twisted imports
from twisted.internet import reactor
from twisted.python.filepath import FilePath
from twisted.python.modules import getModule
from nevow import rend, loaders, appserver, static, tags, stan
from nevow.athena import LivePage, LiveElement, expose

#App imports
from utils import Observer, log
from utils.i18n import to_utf
import ui
from utils.common import find_callbacks
from streams import audio, video, data
from streams.stream import AudioStream, VideoStream, DataStream

log = log.start('debug', 1, 0, 'web')

widgets_mod = []

def import_widgets():
    package = 'ui.web.widgets'
    for widget in getModule(package).iterModules():
        if widget.isPackage():
            medias = getModule(widget.name).iterModules()
            for media in medias:
                try:
                    mod = media.load()
                except:
                    log.error('Unable to load the module %s' % media.name)
                else:
                    if hasattr(mod, 'render'):
                        widgets_mod.append(mod)
        else:
            try:
                mod = widget.load()
            except:
                log.error('Unable to load the module %s' % widget.name)
            else:
                if hasattr(mod, 'render'):
                    widgets_mod.append(mod)
                
    for widget in widgets_mod:
        name = widget.__name__.replace(package, '')[1:].replace('.', '_')
        setattr(Index, 'render_' + name, widget.render)
        

class Index(LivePage, Observer):
    addSlash = True
    
    base_path = os.getcwdu()
    
    # serve some static files
    child_jslib = static.File('ui/web/js/')
    child_css = static.File('ui/web/templates/default/css/')
    child_js = static.File('ui/web/templates/default/js/')
    child_img = static.File('ui/web/templates/default/img/')

    def __init__(self, subject, template=None):
        Observer.__init__(self, subject)
        self.subject = subject
        self.api = subject.api
        self.template = template
                
        # load base HTML file
        self.docFactory = loaders.xmlfile(path.join(path.dirname(__file__), 'index.html'))
        
        if self.template:
            self.child_template = static.File(path.join('ui/web/templates', self.template))
  
        LivePage.__init__(self)

    
    def list_widgets(self):
        base_path = path.dirname(__file__)
        template_path = path.join(base_path, 'templates/default/html/index.html')
        if self.template:
            tmp_path = path.join(base_path, 'templates', self.template, 'html/index.html')
            if path.isfile(tmp_path):
                template_path = tmp_path
        
        tree = ElementTree.parse(template_path)
        divs = tree.getiterator('div')
        attribute = '{http://nevow.com/ns/nevow/0.1}render'
        widgets = [div.attrib[attribute]
                   for div in divs
                   if div.attrib.has_key(attribute)]
        return widgets
    
    def render_header(self, ctx, data):
        """
        Add javascript and css import to the page header.
        """
        base_path = path.dirname(__file__).replace(self.base_path + '/', '')
        templates_path = path.join(base_path, 'templates')

        widgets = self.list_widgets()
        widgets.insert(0, 'index')
        log.debug(widgets)
        
        links = []
        
        # add style.css files
        style_file = 'css/style.css'
        if path.isfile(path.join(templates_path, 'default', style_file)):
            links.append(self.create_css(style_file))
        if self.template and \
        path.isfile(path.join(templates_path, self.template, style_file)):
            links.append(self.create_css(path.join('template', style_file)))
                
        kinds = {'css':self.create_css, 'js':self.create_js}
        
        # add nevow js, display js and css for every widget
        for widget in widgets:
            widget_file = '%s.js' % widget
            # add the nevow js code for this widget
            if path.isfile(path.join(base_path, 'js', widget_file)):
                links.append(self.create_js(path.join('jslib', widget_file)))
            for kind in kinds:
                # kind directory plus name of the file. Ex.: css/index.css
                end_path = path.join(kind, '%s.%s' % (widget, kind))
                if self.template and \
                path.isfile(path.join(templates_path, self.template, end_path)):
                    links.append(kinds[kind](path.join('template', end_path)))
                elif path.isfile(path.join(templates_path, 'default', end_path)):
                    links.append(kinds[kind](path.join(end_path)))

        # Update the mapping of known JavaScript modules so that the
        # client-side code for this example can be found and served to the
        # browser.
#        self.jsModules.mapping[u'AddressBook'] = '/Users/etienne/Documents/propulesart/miville/trunk/py/ui/web/js/addressbook.js'

        # pretty html output
        for i in range(1, len(links)*2, 2):
            links.insert(i, ['\n\t\t'])
            
        return links

    def create_css(self, href):
        """
        Create a css link tag
        """ 
        return tags.link(rel="stylesheet", type="text/css", href=href)

    def create_js(self, src):
        """
        Create a js script tag
        """ 
        return tags.script(src=src, type="text/javascript")

    def render_body(self, ctx, data):
        """
        Load and render the body of the page from the selected template.
        """
        body_file = 'html/index.html'
        templates_path = path.join(path.dirname(__file__), 'templates')
        if self.template and path.isfile(path.join(templates_path, self.template, body_file)):
            template = self.template
        else:
            template = 'default'

        body = loaders.xmlfile(path.join(templates_path, template, body_file))
        return body

    def child_ (self, ctx):
        """
        This send a new Index instance on browser refresh and multiple connections.
        """
        return Index(self.subject)

    def update(self, origin, key, data):
        for children in self.liveFragmentChildren:
            if hasattr(children, 'callbacks') and key in children.callbacks:
                children.callbacks[key](origin, data)
        



def start(subject, port=8080):
    """
    This function is call when the core find and load all the UI packages.
    """
    site = appserver.NevowSite(Index(subject))
    reactor.listenTCP(port, site, 5, '127.0.0.1')
    import_widgets()
    
    
    
