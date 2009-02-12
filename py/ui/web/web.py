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
    """
    Import all the widget modules found in the widgets directory
    and add render_<widget> method to the Index class for each.
    """
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
                if hasattr(mod, camel_name(mod.__name__.replace(package, '')[1:])):
                    widgets_mod.append(mod)
                
    # for each widget add a render_<widget> method to the Index class
    for widget in widgets_mod:
        name = widget.__name__.replace(package, '')[1:].replace('.', '_')
        setattr(Index, 'render_' + name, widget_factory(widget))

def widget_factory(mod):
    """
    Factory that produce a render_widget method to be add to the Index class
    for each widget.
    """
    # get the module name without the path
    mod_name = mod.__name__.split('.')[-1]
    # convert the module to the class name by removing the underscore and camelcasing it
    klass_name = camel_name(mod_name)
    # get the class from the widget module
    klass = getattr(mod, klass_name)
    
    def render_widget(self, ctx, data):
        widget = klass(self.api, self.template)
        widget.setFragmentParent(self)
        return widget
    
    return render_widget

def camel_name(name):
    """
    Transform a file name (small case with underscore) to a class name style
    (camel case without underscore). Put first character and each character
    after an underscore in uppper case and remove the underscores. 
    """
    return ''.join([part.title() for part in name.split('_')])

def small_name(name):
    """
    Transform a class name to a file name (all lower case with underscore)
    """
    chars = list(name)
    for i, char in enumerate(chars):
        if i > 0 and char.isupper():
            chars.insert(i, '_')
    return ''.join(chars).lower()


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
            widget_path = path.join(self.base_path, base_path, 'js', widget_file)
            if path.isfile(widget_path):
#                links.append(self.create_js(path.join('jslib', widget_file)))
                self.jsModules.mapping[camel_name(widget)] = widget_path
            else:
                log.debug("'%s' is not a valid file." % widget_path)
            for kind in kinds:
                # kind directory plus name of the file. Ex.: css/index.css
                end_path = path.join(kind, '%s.%s' % (widget, kind))
                if self.template and \
                path.isfile(path.join(templates_path, self.template, end_path)):
                    links.append(kinds[kind](path.join('template', end_path)))
                elif path.isfile(path.join(templates_path, 'default', end_path)):
                    links.append(kinds[kind](path.join(end_path)))

        # prettier html output
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
 
        
class Widget(LiveElement):
    """
    Base class for all the widget classes.
    """
    def __init__(self, api, template):
        class_name = self.__class__.__name__
        tmpl_name = 'html/%s.html' % small_name(class_name)
        tmpl_path = path.join(path.dirname(__file__), 'templates')
        if template and path.isfile(path.join(tmpl_path, template, tmpl_name)):
            self.docFactory = loaders.xmlfile(path.join('ui/web/templates/', template, tmpl_name))
        elif path.isfile(path.join(tmpl_path, 'default', tmpl_name)):
            self.docFactory = loaders.xmlfile(path.join('ui/web/templates/default', tmpl_name))
        else:
            log.error("Didn't found any valid %s.html template." % small_name(class_name))
        self.jsClass = to_utf(class_name)
        LiveElement.__init__(self)
        self.callbacks = find_callbacks(self)
        self.api = api
    


def start(subject, port=8080):
    """
    This function is call when the core find and load all the UI packages.
    """
    site = appserver.NevowSite(Index(subject))
    reactor.listenTCP(port, site, 5, '127.0.0.1')
    import_widgets()
    
    
    
