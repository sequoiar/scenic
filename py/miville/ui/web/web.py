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

"""
This modules is the base of the web client (web user interface)
of the application. Its support internationalisation (i18n).

More about the i18n process
---------------------------

Here we describe all the steps involved in producing translations for this modules.
Maybe, at the moment you read this, there will be script that simplify the process.
But anyway it's good to know how it works.

So here the steps to produce translation from scratch:

    We will use the context of creating a new Widget as an example.
    
    1. from nevow.i18n import _ and use this function everywhere you need
       something translated in your python code::
        
          from nevow.i18n import _
          
          def example(feature):
              return _('Sorry, the feature %s is not implemented.' % feature)

    2. In your XML template put the attribute n:render="i18" inside the
       element you need to translate::
       
           <h3 n:render="i18n">Nerds</h3>
           
    3. Extract the strings to translate from your python script with GNU xgettext.
       Put the name of your Widget as the name of the .poy file::
       
           $ xgettext -o pathToLocaleDirectory/your_widget.poy your_widget.py
    
    4. Extract the strings to translate from your XML template with
       nevow-xmlgettext. Put the name of your Widget as the name of the .poh file::
       
           $ nevow-xmlgettext your_widget.xml > pathToLocaleDirctory/your_widget.poh
           
    5. Merge the two newly created files (.poy and .poh) with msgcat.
       Put the .poy file first::
       
           $ cd pathToLocaleDirectory
           $ msgcat -o your_widget.pot your_widget.poy your_widget.poh
           
    6. Create one .po file for each language you want to support with msginit::
    
           $ msginit -l fr_CA -i your_widget.pot
           
       The resulting file your_widget.po should be created in the directory
       localeDirectory/LL_CC/LC_MESSAGES/. You can add the -o arguments to
       msginit to specified the right place.
        
    7. Translate all the strings in you your_widget.po file for each languages.
       You can do this with a tool like Poedit.
       
    8. Convert .po file to the binary format .mo with msgfmt::
    
           $ msgfmt localeDirectory/fr_CA/LC_MESSAGES/your_widget.po
           
    That's it! Now we need to add how to update the translations with
    the evolution of the code and template.
           
"""


#System imports
import os.path as path
import os
from xml.etree import ElementTree

#Twisted imports
from twisted.internet import reactor
from twisted.python.modules import getModule

try:
    from nevow import loaders, appserver, static, tags, inevow
    from nevow.athena import LivePage, LiveFragment, expose as nevow_expose
    from nevow.i18n import render as i18nrender
    from nevow.i18n import _, I18NConfig
except ImportError:
    raise ImportError, 'If you want to use the Web interface, you need to install Nevow.'

#App imports
from miville.utils import Observer, log
from miville.utils.i18n import to_utf
from miville.utils.common import find_callbacks

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
                log.debug('Loaded the module %s' % widget.name)
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
    mod_name = mod.__name__.split('.')[ - 1]
    # convert the module to the class name by removing the underscore and camelcasing it
    klass_name = camel_name(mod_name)
    # get the class from the widget module
    klass = getattr(mod, klass_name)
    
    def render_widget(self, ctx, data):
        """
        Generic method use to render all widget from the Index class.
        """
        widget = klass(self.api, self.template)
        widget.setFragmentParent(self)
        return widget
    
    return render_widget

def camel_name(name):
    """
    Transform a file name (small case with underscore) to a class name style
    (camel case without underscore). Put first character and each character
    after an underscore in uppper case and remove the underscores.
    
    Example::
    
        class_name -> ClassName
    """
    return ''.join([part.title() for part in name.split('_')])

def small_name(name):
    """
    Transform a class name to a file name (all lower case with underscore)
    
    Example::
    
        ClassName -> class_name
    """
    chars = list(name)
    for i, char in enumerate(chars):
        if i > 0 and char.isupper():
            chars.insert(i, '_')
    return ''.join(chars).lower()

def create_css(href):
    """
    Create a css link Stan tag
    """ 
    return tags.link(rel="stylesheet", type="text/css", href=href)

def create_js(src):
    """
    Create a js script Stan tag
    """ 
    return tags.script(src=src, type="text/javascript")



class Index(LivePage, Observer):
    """
    Class representing the base of the root page (/index.xml).
    """
    
    addSlash = True
    
    base_path = os.getcwdu()
    
    # serve some static files
    child_jslib = static.File('ui/web/js/')
    child_css = static.File('ui/web/templates/default/css/')
    child_js = static.File('ui/web/templates/default/js/')
    child_img = static.File('ui/web/templates/default/img/')

#    render_i18n = i18nrender()

    def __init__(self, subject, template=None):
        Observer.__init__(self, subject)
        self.subject = subject
        self.api = subject.api
        self.template = template
        self.lang = 'en'

        # load base XML file
        self.docFactory = loaders.xmlfile(path.join(path.dirname(__file__), 'index.xml'))
        
# not use anymore ?
#        if self.template:
#            self.child_template = static.File(path.join('ui/web/templates', self.template))
  
        LivePage.__init__(self)

    def renderHTTP(self, ctx):
        """
        Overwrite renderHTTP to get GET arguments, cookies values
        and client IP address.
        """
        
        # We're only overriding renderHTTP to look for a 'lang' query parameter
        # without cluttering up the messages renderer, below.
        
        # If 'lang' is present then we "force" the translation language. This
        # simulates how user preferences from the session might be used to
        # override the browser's language settings.
        lang = ctx.arg('lang')
        if lang is not None:
            ctx.remember([lang], inevow.ILanguages)
#        request = inevow.IRequest(ctx)
#        request.addCookie('temp', 'allo', expires=None, domain=None, path=None, max_age=None, comment=None, secure=None)
#        cookie = request.getCookie('temp')
#        print cookie

        # add the locale file for the index
        ctx.remember(I18NConfig(domain='index', localeDir='locale'), inevow.II18NConfig)                

        # get the client IP address to look if he is in the LAN
#        client_ip = request.getClientIP())

        # Let the base class handle it, really.
        return LivePage.renderHTTP(self, ctx)

    def list_widgets(self):
        """
        This method return a list of all the Widgets that are present
        in the index template.
        """
        base_path = path.dirname(__file__)
        template_path = path.join(base_path, 'templates/default/xml/index.xml')
        # if we don'T use default template check if there's a valid index.xml
        # in the choose template directory and set the path accordinally
        if self.template:
            tmp_path = path.join(base_path, 'templates', self.template, 'xml/index.xml')
            if path.isfile(tmp_path):
                template_path = tmp_path
        
        # parse the XML, find the widget element and return the list
        # of widget's name
        tree = ElementTree.parse(template_path)
        divs = tree.getiterator('div')
        attribute = '{http://nevow.com/ns/nevow/0.1}render'
        widgets = [div.attrib[attribute]
                   for div in divs
                   if (div.attrib.has_key(attribute)
                       # remove i18n because it's not a widget
                       and div.attrib[attribute] != 'i18n')]
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
            links.append(create_css(style_file))
        if self.template and \
        path.isfile(path.join(templates_path, self.template, style_file)):
            links.append(create_css(path.join('template', style_file)))
                
        kinds = {'css':create_css, 'js':create_js}
        
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

        # prettier xml output
        for i in range(1, len(links) * 2, 2):
            links.insert(i, ['\n\t\t'])
            
        return links

    def render_body(self, ctx, data):
        """
        Load and render the body of the page from the selected template.
        """
        body_file = 'xml/index.xml'
        templates_path = path.join(path.dirname(__file__), 'templates')
        if self.template and path.isfile(path.join(templates_path, self.template, body_file)):
            template = self.template
        else:
            template = 'default'

        body = loaders.xmlfile(path.join(templates_path, template, body_file))
        return body

    def child_ (self, ctx):
        """
        This create a new Index instance on browser refresh and multiple connections.
        """
        return Index(self.subject)

    def update(self, origin, key, data):
        """
        Observer update method.
        Call proper callbacks in all the Widgets.
        """
        for children in self.liveFragmentChildren:
            if hasattr(children, 'callbacks') and key in children.callbacks:
                children.callbacks[key](origin, data)


# Remove the rewrite of the XML element id
# (now we need to avoid clash of id ourself)
for key, preprocessors in enumerate(LiveFragment.preprocessors):
    if preprocessors.__name__ == "rewriteAthenaIds":
        del LiveFragment.preprocessors[key]
 
        
class Widget(LiveFragment):
    """
    Base class for all the widget classes.
    
    *Note: we use the deprecated LiveFragment instead of LiveElement because
    i18n need the context, so it doesn't work with LiveElement.* 
    """

    # add the translation renderer
    render_i18n = i18nrender()

    def __init__(self, api, template):
        """
        Get the name of the widget and find the template file (in template and,
        if not, in 'default'.)
        """

        self.docFactory = ['']  # not sure if it's a good idea ?
        class_name = self.__class__.__name__
        tmpl_name = 'xml/%s.xml' % small_name(class_name)
        tmpl_path = path.join(path.dirname(__file__), 'templates')
        if template and path.isfile(path.join(tmpl_path, template, tmpl_name)):
            self.docFactory = loaders.xmlfile(path.join('ui/web/templates/', template, tmpl_name))
        elif path.isfile(path.join(tmpl_path, 'default', tmpl_name)):
            self.docFactory = loaders.xmlfile(path.join('ui/web/templates/default', tmpl_name))
        else:
            log.error("Didn't found any valid %s.xml template." % small_name(class_name))
        # add the JS class for this widget
        # (should be the same name has the python class) 
        self.jsClass = to_utf(class_name)
        LiveFragment.__init__(self)
        self.callbacks = find_callbacks(self, 'cb_')
        self.api = api

    def rend(self, ctx, data):
        """
        We overwrite LiveFragment.rend to add the good i18n domain
        to the context of each Widget base on the module name of the widget. 
        """
        # TODO: get the good localeDir for each platform
        domain = self.__module__.split('.')[ - 1]
        ctx.remember(I18NConfig(domain=domain, localeDir='locale'),
                     inevow.II18NConfig)                

        # Let the base class handle it.
        return LiveFragment.rend(self, ctx, data)

    def callRemote(self, methodName, *args):
        """
        Overwrite callRemote to tranform all object of type string to utf.
        """
        args = tree_to_utf(*args)
        
        # call the base class callRemote
        LiveFragment.callRemote(self, methodName, *args)

        
def tree_to_utf(*args):
    """
    Convert every string to utf in mixed structure of int, float, string,
    unicode, list, tuple and dict to be ready for JSON.
    """
    clean_args = []
    for arg in args:
        if isinstance(arg, str):
            clean_args.append(to_utf(arg))
        elif isinstance(arg, (tuple, list)):
            clean_args.append(tree_to_utf(*arg))
        elif isinstance(arg, dict):
            clean_dict = {}
            for key, value in arg.items():
                clean_dict[to_utf(key)] = tree_to_utf(value)[0]
            clean_args.append(clean_dict)
        else:
            clean_args.append(arg)
            
    return clean_args
        

def expose(loc):
    """
    Function that automatically expose (that mean become accessible from javascript)
    all methods beginning by "rc\_" (for "remote call") in the local scope
    of a Class.
    
    :param loc: Should be a dictionnary where values are method object
                (usually you pass locals())
     
    Usage::
        
        class Widget(LiveElement):
            def rc_method(self):
                pass
            expose(locals())
    
    """
    for key, value in loc.iteritems():
        if key.startswith('rc_'):
            nevow_expose(value)
    


def start(subject, port=8080, interfaces=''):
    """
    This function is call when the core find and load all the UI packages.
    """
    site = appserver.NevowSite(Index(subject))
    # reactor.listenTCP(port, site, 5, '127.0.0.1')
    # subject is the api...
    subject.api.listen_tcp(port, site, interfaces) #  subject.config.ui_network_interfaces)
    import_widgets()
    
    
    
